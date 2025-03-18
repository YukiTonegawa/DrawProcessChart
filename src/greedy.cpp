#include "lib.hpp"
#include <queue>

int main() {
    std::vector<std::pair<int, int>> E;
    process_map mp;
    for (auto [s, t] : read_csv("../testcase/case2.csv")) {
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
        Y[i] = xcnt[x]++;
    }

    // スコア計算
    {
        std::vector<std::pair<int, int>> pos(N);
        for (int i = 0; i < N; i++) {
            pos[i] = {X[i], Y[i]};
        }
        double score = sum_edge_length(pos, E);
        std::cout << "score is " << score << '\n';
    }

    // 答えを作成
    std::vector<std::tuple<std::string, int, int>> P(N);
    for (int i = 0; i < N; i++) {
        P[i] = {mp.get_process(i), X[i], Y[i]};
    }
    write_csv("../testcase/case2_ans.csv", P);
}