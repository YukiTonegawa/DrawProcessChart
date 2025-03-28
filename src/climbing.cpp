#include "CheckLib.hpp"
#include "Lib.hpp"
#include "Random.hpp"
#include "SimulatedAnnealing.hpp"
#include <queue>
#include <chrono>
#include <cassert>

// 山登り法
// 時間の許す限り縦方向の並びを変更
int main() {
    std::string path_in = "../testcase/random_large.csv";
    std::string path_out = "../testcase/random_large_ans_cl.csv";
    assert(CheckLib::is_valid_input(path_in));
    std::vector<std::pair<int, int>> E;
    ProcessMap mp;
    for (auto [s, t] : CheckLib::read_csv(path_in)) {
        int sid = mp.register_process(s);
        int tid = mp.register_process(t);
        E.push_back({sid, tid});
    }
    E = remove_multiple_edge(E);
    int N = mp.size();
    auto G = adjacency_list(N, E);
    auto X = calc_min_x(G);

    // 縦方向の座標を雑に決める
    std::vector<int> Y(N), xcnt(N, 0);
    std::vector<std::vector<int>> Col(N);
    for (int i = 0; i < N; i++) {
        int x = X[i];
        Y[i] = xcnt[x];
        xcnt[x]++;
        Col[x].push_back(i);
    }

    auto _calc_score = [&]() -> double {
        std::vector<std::pair<int, int>> tmp(N);
        for (int i = 0; i < N; i++) {
            tmp[i] = {X[i], Y[i]};
        }
        return calc_score(tmp, E);
    };

    double score = _calc_score();

    const int Tend = 2000;
    timer<0>::set();
    while (true) {
        if (timer<0>::elapse() >= Tend) break;
        int x = rng.random_number() % N;
        int sz = Col[x].size();
        if (sz <= 1) continue;
        int a = Col[x][rng.random_number() % sz];
        int b = Col[x][rng.random_number() % sz];
        std::swap(Y[a], Y[b]);
        double new_score = _calc_score();
        if (new_score < score) {
            score = new_score;
        } else {
            std::swap(Y[a], Y[b]);
        }
    }

    std::vector<std::pair<int, int>> pos(N);
    for (int i = 0; i < N; i++) {
        pos[i] = {X[i], Y[i]};
    }
    std::cout << "score is " << score << '\n';
    std::vector<std::tuple<std::string, int, int>> P(N);
    for (int i = 0; i < N; i++) {
        P[i] = {mp.get_process(i), pos[i].first, pos[i].second};
    }
    CheckLib::write_csv(path_out, P);
}