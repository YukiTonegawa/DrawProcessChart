#include "CheckLib.hpp"
#include "Lib.hpp"
#include "SimulatedAnnealing.hpp"
#include <numeric>

struct StateSA {
    using UpdateType = std::tuple<int, int, int>;
    using ScoreType = double;
    ScoreType score;
    std::vector<int> perm;
    int N;
    std::tuple<int, int, int> last_query;
    double last_score;
    std::vector<std::vector<int>> P;
    std::vector<int> minX, curX, ord;
    std::vector<std::pair<int, int>> E;
    std::vector<std::vector<int>> G;

    StateSA(std::vector<std::vector<int>> _P, std::vector<int> _X, std::vector<std::pair<int, int>> _E) : score(std::numeric_limits<double>::max()), perm(_P.size()), N(_X.size()), P(_P), minX(_X), curX(_X), E(_E), G(adjacency_list(N, E)) {
        std::iota(perm.begin(), perm.end(), 0);
        auto tmpX = curX;
        auto pos = compress_y(P, perm, tmpX);
        score = calc_score(pos, E);

        std::vector<int> in(N, 0), X(N);
        for (int i = 0; i < N; i++) {
            for (int t : G[i]) {
                in[t]++;
            }
        }
        std::queue<int> que;
        for (int i = 0; i < N; i++) {
            if (in[i] == 0) {
                que.push(i);
            }
        }
        while (!que.empty()) {
            int s = que.front();
            que.pop();
            ord.push_back(s);
            for (int t : G[s]) {
                in[t]--;
                if (in[t] == 0) {
                    que.push(t);
                }
            }
        }
    }

    std::vector<int> make_tmpX() {
        auto tmpX = curX;
        for (int s : ord) {
            for (int t : G[s]) {
                tmpX[t] = std::max(tmpX[t], tmpX[s] + 1);
            }
        }
        return tmpX;
    }

    void random_update() {
        int M = P.size();
        int type = rng.random_number() % 3;
        //type = 0;
        if (type == 0) {
            int a = rng.random_number() % M;
            int b = rng.random_number() % M;
            std::swap(perm[a], perm[b]);
            last_query = {0, a, b};
        } else if (type == 1) {
            int a = rng.random_number() % N;
            last_query = {1, a, 1};
            curX[a]++;
        } else {
            int a = rng.random_number() % N;
            if (curX[a] == minX[a]) {
                last_query = {2, a, 0};
            } else {
                last_query = {2, a, 1};
                curX[a]--;
            }
        }
        last_score = score;
        auto tmpX = make_tmpX();
        auto pos = compress_y(P, perm, tmpX);
        score = calc_score(pos, E);
    }

    void rollback() {
        auto [type, a, b] = last_query;
        if (type == 0) {
            std::swap(perm[a], perm[b]);
            score = last_score;
        } else if (type == 1) {
            curX[a]--;
            score = last_score;
        } else {
            curX[a] += b;
            score = last_score;
        }
    }

    ScoreType get_score() {
        return score;
    }
};

int main() {
    std::string path_in = "../testcase/case1.csv";
    std::string path_out = "../testcase/case1_lp.csv";
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
    auto P = decompose_long_path(G);
    int K = P.size();
    std::vector<std::tuple<std::string, int, int>> ans(N);
    std::vector<std::pair<int, int>> pos;

    if (false) {
        std::vector<int> perm(K);
        std::iota(perm.begin(), perm.end(), 0);
        double min_score = std::numeric_limits<double>::max();
        auto min_perm = perm;
        do {
            pos = compress_y(P, perm, X);
            double score = calc_score(pos, E);
            if (score < min_score) {
                min_score = score;
                min_perm = perm;
            }
        } while (std::next_permutation(perm.begin(), perm.end()));
        pos = compress_y(P, min_perm, X);
    } else {
        StateSA sa(P, X, E);
        simulated_annealing<timer<0>, temperature_scheduler_exp<0>, StateSA>()(sa, 1000, 0.1, 2000, 1);
        pos = compress_y(P, sa.perm, sa.make_tmpX());
    }
    double score = calc_score(pos, E);
    std::cout << "score is " << score << '\n';
    for (int i = 0; i < N; i++) {
        ans[i] = {mp.get_process(i), pos[i].first, pos[i].second};
    }
    CheckLib::write_csv(path_out, ans);
}