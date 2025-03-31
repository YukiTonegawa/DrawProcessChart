#include "CheckLib.hpp"
#include "Lib.hpp"

int main() {
    std::string path_in = "../testcase/case2.csv";
    std::string path_out = "../testcase/case2_gr.csv";
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
    // 各工程の横軸の座標を決定
    auto X = calc_min_x(G);

    // 縦方向の座標を雑に決める
    std::vector<int> Y(N), xcnt(N, 0);
    for (int i = 0; i < N; i++) {
        int x = X[i];
        Y[i] = xcnt[x];
        xcnt[x]++;
    }

    std::vector<std::pair<int, int>> pos(N);
    for (int i = 0; i < N; i++) {
        pos[i] = {X[i], Y[i]};
    }

    // スコア計算
    double score = calc_score(pos, E);
    std::cout << "score is " << score << '\n';

    // 答えを作成
    std::vector<std::tuple<std::string, int, int>> P(N);
    for (int i = 0; i < N; i++) {
        P[i] = {mp.get_process(i), pos[i].first, pos[i].second};
    }
    CheckLib::write_csv(path_out, P);
}