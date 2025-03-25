#include "CheckLib.hpp"
#include "Lib.hpp"
#include "SimulatedAnnealing.hpp"
#include "solve_penetration.hpp"
#include <queue>
#include <numeric>

std::vector<std::pair<int, int>> compress_y(int N, std::vector<std::vector<int>> P, std::vector<int> perm, std::vector<int> X) {
    int R = P.size();
    int l = 0, y = 0;
    std::vector<std::pair<int, int>> ans(N);
    while (l < R) {
        std::vector<bool> used(N, false);
        int r = l;
        while (r < R) {
            int lx = X[P[perm[r]][0]];
            int rx = X[P[perm[r]].back()];
            bool ok = true;
            for (int j = lx; j <= rx; j++) {
                if (used[j]) {
                    ok = false;
                    break;
                }
                used[j] = true;
            }
            if (!ok) break;
            r++;
        }
        for (int i = l; i < r; i++) {
            for (int v : P[perm[i]]) {
                ans[v].first = X[v];
                ans[v].second = y;
            }
        }
        y++;
        l = r;
    }
    return ans;
}

struct StateSA {
    using UpdateType = std::tuple<int, int, int>;
    using ScoreType = double;
    ScoreType score;
    std::vector<int> perm;
    int N;
    std::pair<int, int> last_swapped;
    double last_score;
    std::vector<std::vector<int>> P;
    std::vector<int> X;
    std::vector<std::pair<int, int>> E;

    StateSA(int _N, std::vector<std::vector<int>> _P, std::vector<int> _X, std::vector<std::pair<int, int>> _E) : score(std::numeric_limits<double>::max()), perm(_P.size()), N(_N), P(_P), X(_X), E(_E) {
        std::iota(perm.begin(), perm.end(), 0);
        auto pos = compress_y(N, P, perm, X);
        score = sum_edge_length(pos, E);
    }

    void random_update() {
        int M = P.size();
        int a = rng.random_number() % M;
        int b = rng.random_number() % M;
        std::swap(perm[a], perm[b]);
        last_swapped = {a, b};
        last_score = score;
        auto pos = compress_y(N, P, perm, X);
        score = sum_edge_length(pos, E);
    }

    void rollback() {
        auto [a, b] = last_swapped;
        std::swap(perm[a], perm[b]);
        score = last_score;
    }

    ScoreType get_score() {
        return score;
    }
};

int main() {
    std::string path_in = "../testcase/case1.csv";
    std::string path_out = "../testcase/case1_ans.csv";

    std::vector<std::pair<int, int>> E;
    ProcessMap mp;
    for (auto [s, t] : CheckLib::read_csv(path_in)) {
        int sid = mp.register_process(s);
        int tid = mp.register_process(t);
        E.push_back({sid, tid});
    }

    const int N = mp.size();
    
    auto X = calc_min_x(N, E);
    auto P = decompose_long_path(N, E);

    int K = P.size();
    if (K <= 8) {
        std::vector<int> perm(K);
        std::iota(perm.begin(), perm.end(), 0);
        double min_score = std::numeric_limits<double>::max();
        auto min_perm = perm;
        do {
            auto pos = compress_y(N, P, perm, X);
            double score = sum_edge_length(pos, E);
            if (score < min_score) {
                min_score = score;
                min_perm = perm;
            }
        } while (std::next_permutation(perm.begin(), perm.end()));
        
        auto pos = compress_y(N, P, min_perm, X);
        double score = sum_edge_length(pos, E);
        std::cout << "score is " << score << '\n';
        std::vector<std::tuple<std::string, int, int>> ans(N);
        for (int i = 0; i < N; i++) {
            ans[i] = {mp.get_process(i), pos[i].first, pos[i].second};
        }
        CheckLib::write_csv(path_out, ans);
    } else {
        StateSA sa(N, P, X, E);
        simulated_annealing<timer<0>, temperature_scheduler_exp<0>, StateSA>()(sa, 1000, 0.1, 2000, 1);
        auto pos = compress_y(N, P, sa.perm, X);
        double score = sum_edge_length(pos, E);
        std::cout << "score is " << score << '\n';
        std::vector<std::tuple<std::string, int, int>> ans(N);
        for (int i = 0; i < N; i++) {
            ans[i] = {mp.get_process(i), pos[i].first, pos[i].second};
        }
        CheckLib::write_csv(path_out, ans);
    }
}