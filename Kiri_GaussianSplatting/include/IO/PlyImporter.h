#pragma once 
#include "vector"
#include "string"
#include "Render/GaussianTypes.h"

class PlyImporter {
public:
    PlyImporter();
    ~PlyImporter();

    static bool Import(const std::string& path, std::vector<StandardGaussian>& stdGaussianDataVec);
};