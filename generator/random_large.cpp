#include "../src/Random.hpp"
#include <iostream>

int main() {
    const int N = 50; // 工程数
    const int M = 50; // 辺数
    auto P = rng.random_permutation(N);
    for (int i = 0; i < M; i++) {
        int a = rng.random_number() % N;
        int b = rng.random_number() % N;
        while (a == b) {
            b = rng.random_number() % N;
        }
        if (P[a] > P[b]) std::swap(a, b);
        std::cout << 'P' << a << "," << 'P' << b << '\n';
    }
}