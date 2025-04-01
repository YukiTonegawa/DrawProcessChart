#include "CheckLib.hpp"
#include "Lib.hpp"
#include "Random.hpp"
#include "BeamSearch.hpp"
#include "SimulatedAnnealing.hpp"

std::vector<std::pair<int, int>> Ord;
std::vector<std::pair<int, int>> E2;
std::vector<int> Xcnt;

struct MyCmp {
    bool operator () (std::pair<int, double> a, std::pair<int, double> b) {
        return (a.first == b.first ? a.second < b.second : a.first > b.first);
    }
};

struct MyState {
    using self_t = MyState;
    using Score = std::pair<int, double>;
    using Update = std::tuple<int, int, int>;

    Score score;
    std::vector<std::pair<int, int>> pos;
    int eid;

    MyState() : score({0, 0}), eid(0) {}
    
    self_t update(Update u) {
        auto [id, x, y] = u;
        MyState s = *this;
        s.pos.push_back({x, y});
        while (s.eid < E2.size() && id >= E2[s.eid].second) {
            s.eid++;
        }
        auto Etmp = std::vector<std::pair<int, int>>(E2.begin(), E2.begin() + s.eid);
        s.score = {score.first + 1, calc_score(s.pos, Etmp)};
        return s;
    }

    std::vector<Update> get_neighbors() {
        int id = pos.size();
        if (id == Ord.size()) return {};
        int x = Ord[id].second;
        int W = Xcnt[x] * 2;
        std::vector<Update> res;
        while (res.size() <= std::min(Xcnt[x], 50)) {
            int y = rng.random_number() % W;
            bool ng = false;
            for (auto [_x, _y] : pos) {
                if (_x == x && _y == y) {
                    ng = true;
                    break;
                }
            }
            if (!ng) res.push_back({id, x, y});
        }
        return res;
    }
};

int main() {
    std::string path_in = "../testcase/random_small.csv";
    std::string path_out = "../testcase/random_small_be.csv";
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
    Xcnt.resize(N, 0);

    for (int i = 0; i < N; i++) {
        Ord.push_back({i, X[i]});
        Xcnt[X[i]]++;
    }

    std::sort(Ord.begin(), Ord.end(), [&](auto a, auto b) { return a.second < b.second; });

    std::vector<int> rev(N);
    for (int i = 0; i < N; i++) {
        auto [id, x] = Ord[i];
        rev[id] = i;
    }

    for (auto [s, t] : E) {
        E2.push_back({rev[s], rev[t]});
    }

    std::sort(E2.begin(), E2.end(), [&](auto a, auto b) { return a.second < b.second; });

    MyState s;
    auto U = beam_search<timer<0>, MyState, MyCmp>()(s, 50, 2000);
    for (auto u : U) {
        s = s.update(u);
    }

    // 縦方向の座標を雑に決める
    std::vector<int> Y(N);
    for (int i = 0; i < N; i++) {
        auto [id, x] = Ord[i];
        int y = s.pos[i].second;
        Y[id] = y;
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