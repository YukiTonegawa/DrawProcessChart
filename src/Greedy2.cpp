#include "CheckLib.hpp"
#include "Lib.hpp"
#include "SimulatedAnnealing.hpp"
#include <numeric>

int main() {
    std::string path_in = "../testcase/random_med.csv";
    std::string path_out = "../testcase/random_med_perm.csv";
    int time_end = 2000;

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

    if (K <= 6) {
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
        double min_score = std::numeric_limits<double>::max();
        std::vector<int> min_perm(K);
        std::iota(min_perm.begin(), min_perm.end(), 0);
        timer<0>::set();
        while (timer<0>::elapse() <= time_end) {
            auto perm = rng.random_permutation(K);
            pos = compress_y(P, perm, X);
            double score = calc_score(pos, E);
            if (score < min_score) {
                min_score = score;
                min_perm = perm;
            }
        }
        pos = compress_y(P, min_perm, X);
    }
    double score = calc_score(pos, E);
    std::cout << "score is " << score << '\n';
    std::cout << "lensum is " << sum_edge_length(pos, E) << '\n';
    std::cout << "cross is " << count_edge_cross(pos, E) << '\n';
    std::cout << "penetration is " << count_bad_penetration(pos, E) << '\n';

    for (int i = 0; i < N; i++) {
        ans[i] = {mp.get_process(i), pos[i].first, pos[i].second};
    }
    CheckLib::write_csv(path_out, ans);
}