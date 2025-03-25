#include "CheckLib.hpp"
#include "Lib.hpp"
#include "solve_penetration.hpp"
#include <queue>

int main() {
    std::string path_in = "../testcase/case1.csv";
    std::string path_out = "../testcase/case1_ans.csv";

    std::vector<std::pair<int, int>> E;
    process_map mp;
    for (auto [s, t] : CheckLib::ReadCsv(path_in)) {
        int sid = mp.register_process(s);
        int tid = mp.register_process(t);
        E.push_back({sid, tid});
    }

    // 各工程の横軸の座標を決定
    const int N = mp.size();
    std::vector<int> in(N, 0), X(N);
    std::vector<std::vector<int>> G(N);
    for (auto [s, t] : E) {
        in[t]++;
        G[s].push_back(t);
    }
    std::queue<int> que;
    for (int i = 0; i < N; i++) {
        if (in[i] == 0) {
            X[i] = 0;
            que.push(i);
        }
    }
    while (!que.empty()) {
        int s = que.front();
        que.pop();
        for (int t : G[s]) {
            in[t]--;
            if (in[t] == 0) {
                X[t] = X[s] + 1;
                que.push(t);
            }
        }
    }
    
    // 縦方向の座標を雑に決める
    std::vector<int> Y(N), xcnt(N, 0);
    for (int i = 0; i < N; i++) {
        int x = X[i];
        Y[i] = xcnt[x];
        // Y[i] = (xcnt[x] % 2 == 1 ? (xcnt[x] + 1) / 2 : -xcnt[x] / 2);
        xcnt[x]++;
    }

    std::vector<std::pair<int, int>> pos(N);
    for (int i = 0; i < N; i++) {
        pos[i] = {X[i], Y[i]};
    }
    pos = solve_penetration(pos, E);

    // スコア計算
    double score = sum_edge_length(pos, E);
    std::cout << "score is " << score << '\n';

    // 答えを作成
    std::vector<std::tuple<std::string, int, int>> P(N);
    for (int i = 0; i < N; i++) {
        P[i] = {mp.get_process(i), pos[i].first, pos[i].second};
    }
    CheckLib::WriteCsv(path_out, P);
}