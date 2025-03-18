#ifndef _RANDOM_H_
#define _RANDOM_H_
#include <random>
#include <vector>
#include <algorithm>

struct random_generator {
  private:
    std::mt19937 mt;
  
  public:
    random_generator(const int seed = 1234) : mt(seed) {}

    // [0, 2^32)
    uint32_t random_number() {
        return mt();
    }

    // [0, N)の順列
    std::vector<int> random_permutation(int N) {
        std::vector<int> res(N);
        std::iota(res.begin(), res.end(), 0);
        std::shuffle(res.begin(), res.end(), mt);
        return res;
    }
} rng;
#endif