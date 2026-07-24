
#include <gtest/gtest.h>
#include <oneapi/tbb/parallel_sort.h>
#include <vector>
#include <algorithm>
#include <random>
#include <execution>

const int N = 10000000;

TEST(MathTest, StdSort)
{
	std::vector<float> testBuffer(N);
	std::mt19937 rng(12345);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	for (int i = 0; i < N; ++i)
	{
		testBuffer[i] = dist(rng);
	}
	std::sort(testBuffer.begin(), testBuffer.end());
	EXPECT_TRUE(std::is_sorted(testBuffer.begin(), testBuffer.end()));
}


TEST(MathTest, StdSort_par_unseq)
{
	std::vector<float> testBuffer(N);
	std::mt19937 rng(12345);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	for (int i = 0; i < N; ++i)
	{
		testBuffer[i] = dist(rng);
	}
	std::sort(std::execution::par_unseq , testBuffer.begin(), testBuffer.end());
	EXPECT_TRUE(std::is_sorted(testBuffer.begin(), testBuffer.end()));
}
 

TEST(MathTest, TbbParallelSort)
{
	std::vector<float> testBuffer(N);
	std::mt19937 rng(12345);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	for (int i = 0; i < N; ++i)
	{
		testBuffer[i] = dist(rng);
	}
	oneapi::tbb::parallel_sort(testBuffer.begin(), testBuffer.end());
	EXPECT_TRUE(std::is_sorted(testBuffer.begin(), testBuffer.end()));
}
