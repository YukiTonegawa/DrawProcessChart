#ifndef _RANDOM_H_
#define _RANDOM_H_
#include <random>
#include <vector>
#include <algorithm>

struct RandomGenerator {
  private:
    std::mt19937 mt;
  
  public:
    RandomGenerator(const int seed = 1234) : mt(seed) {}

    // [0, 2^32)
    uint32_t RandomNumber() {
        return mt();
    }

    // [0, N)の順列
    std::vector<int> RandomPermutation(int N) {
        std::vector<int> res(N);
        std::iota(res.begin(), res.end(), 0);
        std::shuffle(res.begin(), res.end(), mt);
        return res;
    }

    // 確率pで1, (1-p)で0
    bool Judge(double p) {
        static constexpr double inf = (double)std::numeric_limits<uint32_t>::max();
        return RandomNumber() < inf * p;
    }
} rng;
#endif