#include "IO/PlyImporter.h"
#include "chrono"
#include "Common/Utils.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_sort.h>


PlyImporter::PlyImporter() {

}
PlyImporter::~PlyImporter() {

}

bool PlyImporter::Import(const std::string& path, std::vector<StandardGaussian>& stdGaussianDataVec) {

	auto start = std::chrono::high_resolution_clock::now();

	std::ostringstream oss;

	auto data = std::make_unique<PLYData>();
#ifdef _WIN32
	std::wstring  wpath = utf8_to_utf16(path);
	std::ifstream fs(wpath, std::ios::binary);
#else
	std::ifstream fs(path, std::ios::binary);
#endif 
	if (!fs.is_open()) {
		return false;
	}

	std::vector<char> io_buffer(1024 * 1024 * 4); // 4MB buffer
	fs.rdbuf()->pubsetbuf(io_buffer.data(), io_buffer.size());

	std::string line;
	size_t header_size = 0;
	int propertyOffset = 0;

	while (std::getline(fs, line)) {
		header_size += line.length() + 1; // +1 for \n
		if (line.find("element vertex") != std::string::npos) {
			std::stringstream ss(line);
			std::string tmp;
			ss >> tmp >> tmp >> data->vertexCount;
		}
		if (line.find("property float") != std::string::npos) {
			data->propertyCount++;
			std::stringstream ss(line);
			std::string propertyName;
			std::string tmp;
			ss >> tmp >> tmp >> propertyName;
			data->propMap[propertyName] = propertyOffset++;
			oss.str("");
			oss << propertyName << " " << data->propMap[propertyName] << std::endl;
			PLOGI << oss.str();
		}
		if (line == "end_header") break;
	}

	if (data->vertexCount == 0 || data->propertyCount == 0)
	{
		return false;
	}

	stdGaussianDataVec.resize(data->vertexCount);

	size_t total_floats = data->vertexCount * data->propertyCount;
	data->buffer.resize(total_floats);

	fs.read(reinterpret_cast<char*>(data->buffer.data()), total_floats * sizeof(float));

	int idxX = data->propMap["x"];
	int idxY = data->propMap["y"];
	int idxZ = data->propMap["z"];
	int idxOp = data->propMap["opacity"];
	int idxS0 = data->propMap["scale_0"], idxS1 = data->propMap["scale_1"], idxS2 = data->propMap["scale_2"];
	int idxR0 = data->propMap["rot_0"], idxR1 = data->propMap["rot_1"], idxR2 = data->propMap["rot_2"], idxR3 = data->propMap["rot_3"];

	int idxDC[3] = { data->propMap["f_dc_0"], data->propMap["f_dc_1"], data->propMap["f_dc_2"] };

	int idxRest[45];
	for (int j = 0; j < 45; j++) {
		idxRest[j] = data->propMap["f_rest_" + std::to_string(j)];
	}

	auto processBegin = std::chrono::high_resolution_clock::now();

	oneapi::tbb::parallel_for(
		oneapi::tbb::blocked_range<int>(0, static_cast<int>(data->vertexCount)),
		[&](const oneapi::tbb::blocked_range<int>& range) {
			for (int i = range.begin(); i != range.end(); ++i) {
				const float* src = &data->buffer[i * propertyOffset];
				StandardGaussian& dest = stdGaussianDataVec[i];

				memcpy(&dest.pos[0], &src[idxX], 3 * sizeof(float));
				//dest.pos[0] = src[idxX];
				//dest.pos[1] = src[idxY];
				//dest.pos[2] = src[idxZ];

				memcpy(&dest.scale[0], &src[idxS0], 3 * sizeof(float));
				memcpy(&dest.quat[0], &src[idxR0], 4 * sizeof(float));


				dest.opacity = src[idxOp];

				memcpy(&dest.sh[0], &src[idxDC[0]], 3 * sizeof(float));

				for (int j = 0; j < 45; j++) {
					int row = j % 15;
					int col = j / 15;
					dest.sh[row * 3 + col + 3] = src[idxRest[j]];
				}
			}
		});
	auto processEnd = std::chrono::high_resolution_clock::now();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::chrono::duration<double, std::milli> processElapsed = processEnd - processBegin;
	oss << "[FastPly] Loaded " << data->vertexCount << " points in "
		<< elapsed.count() << " ms" << std::endl;
	oss << "process Loaded " << processElapsed.count() << "ms " << (processElapsed.count() / elapsed.count()) * 100 << "%" << std::endl;
	PLOGI << oss.str();

	oss.str("");
	oss << stdGaussianDataVec[0].toString() << std::endl;
	//oss << stdGaussianDataVec[1].toString() << std::endl ;
	//oss << stdGaussianDataVec[2].toString() << std::endl ;
	//oss << stdGaussianDataVec[3].toString() << std::endl ;
	PLOGI << oss.str();

	return true;
}