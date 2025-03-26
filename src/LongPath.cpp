#include "CheckLib.hpp"
#include "Lib.hpp"
#include "SimulatedAnnealing.hpp"
#include <numeric>

/*
貫通の処理
y座標の圧縮


1. 無視できる貫通
A -> B -> C -> D
 ------------->

2. 許容できる貫通
中心部を通らない

3. 許容できない貫通
中心部を通る = 直線がある格子点を通り、そこに工程が置かれている

LongPath分解の場合横方向の貫通は全て1になる


斜め方向の貫通を減らしたい
1. 何もしない. 辺の長さを最小化 = 斜めの辺の長さを最小化 = 3の貫通が減る
2. 


*/

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
    std::string path_out = "../testcase/case1_ans_lp.csv";
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
        
        for (int i = 0; i < N; i++) {
            ans[i] = {mp.get_process(i), pos[i].first, pos[i].second};
        }
    } else {
        StateSA sa(N, P, X, E);
        simulated_annealing<timer<0>, temperature_scheduler_exp<0>, StateSA>()(sa, 1000, 0.1, 2000, 1);
        auto pos = compress_y(N, P, sa.perm, X);
        double score = sum_edge_length(pos, E);
        std::cout << "score is " << score << '\n';
        for (int i = 0; i < N; i++) {
            ans[i] = {mp.get_process(i), pos[i].first, pos[i].second};
        }
    }
    CheckLib::write_csv(path_out, ans);
}