#include "lib.hpp"
#include "random.hpp"
#include <queue>
#include <chrono>
#include <cassert>

// ms単位で現在の時刻を取得
long long timems() {
    auto p = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(p).count();
}

template<int id>
struct timer {
    static bool ok; // set済みか
    static long long T0;
    // 基準点をセット
    static void set() {
        ok = true;
        T0 = timems();
    }
    // 基準点からの経過時間(ms単位)
    static long long elapse() {
        assert(ok);
        return timems() - T0;
    };
};
template<int id>
bool timer<id>::ok(false);
template<int id>
long long timer<id>::T0(0);


// 山登り法
// 時間の許す限り縦方向の並びを変更
int main() {
    std::vector<std::pair<int, int>> E;
    process_map mp;
    for (auto [s, t] : read_csv("../testcase/random_med.csv")) {
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
    std::vector<std::vector<int>> Col(N);
    for (int i = 0; i < N; i++) {
        int x = X[i];
        Y[i] = (xcnt[x] % 2 == 1 ? (xcnt[x] + 1) / 2 : -xcnt[x] / 2);
        xcnt[x]++;
        Col[x].push_back(i);
    }

    auto calc_score = [&]() -> double {
        std::vector<std::pair<int, int>> pos(N);
        for (int i = 0; i < N; i++) {
            pos[i] = {X[i], Y[i]};
        }
        return sum_edge_length(pos, E);
    };

    double score = calc_score();

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
        double new_score = calc_score();
        if (new_score < score) {
            score = new_score;
        } else {
            std::swap(Y[a], Y[b]);
        }
    }

    std::cout << "score is " << score << '\n';

    // 答えを作成
    std::vector<std::tuple<std::string, int, int>> P(N);
    for (int i = 0; i < N; i++) {
        P[i] = {mp.get_process(i), X[i], Y[i]};
    }
    write_csv("../testcase/random_med_ans_climbing.csv", P);
}