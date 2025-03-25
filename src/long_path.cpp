#include "lib.hpp"
#include "solve_penetration.hpp"
#include "random.hpp"
#include <queue>
#include <numeric>
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

template<int id>
struct temperature_scheduler_exp {
    static bool ok;
    static double T0, T1, Tend;
    // 指数スケジューリング
    // t := 時刻を[0, 1]に正規化したもの
    // Tcur = T0 ^ (1 - t) * T1 ^ t
    // Tend : 終了時刻(ms)
    static void set(double T0_, double T1_, double Tend_) {
        ok = true;
        T0 = T0_;
        T1 = T1_;
        Tend = Tend_;
    }
    // 現在時刻 -> 現在の温度
    static double get(double elapse_ms) {
        assert(ok);
        assert(0 <= elapse_ms && elapse_ms <= Tend);
        elapse_ms /= Tend;
        return std::pow(T0, 1 - elapse_ms) * std::pow(T1, elapse_ms);
    }
    // (変更前のスコア, 変更後のスコア, 現在の温度) -> 遷移確率
    // diff_scoreが0以下     1 
    //    0より大きい             e ^ {diff_score / Tcur}
    template<typename T>
    static double p_move(T score_before, T score_after, double Tcur) {
        T diff_score = score_after - score_before;
        return diff_score <= 0 ? 1 : std::exp((double)-diff_score / Tcur);
    }
};
template<int id>
bool temperature_scheduler_exp<id>::ok(false);
template<int id>
double temperature_scheduler_exp<id>::T0(0);
template<int id>
double temperature_scheduler_exp<id>::T1(0);
template<int id>
double temperature_scheduler_exp<id>::Tend(0);

// FreqTempUpdate := この回数ごとに1回時刻と温度を更新
template<typename Timer, typename Temp, typename State, bool use_rollback = true>
struct simulated_annealing {
    using UpdateType = typename State::UpdateType;
    using ScoreType = typename State::ScoreType;
    void operator ()(State &v, double _Temp0, double _Temp1, int _TimeEnd, int _FreqTempUpdate) {
        int TimeEnd = _TimeEnd; // 終了時刻
        int TimeCur; // 現在時刻
        double TempCur; // 現在の温度
        Timer::set();
        Temp::set(_Temp0, _Temp1, _TimeEnd);
        ScoreType score_cur = v.get_score();
        Timer::set();
        int i = _FreqTempUpdate;
        while (true) {
            if (i == _FreqTempUpdate) {
                TimeCur = Timer::elapse();
                if (TimeCur >= TimeEnd) return;
                TempCur = Temp::get(TimeCur);
                i = 0;
            }
            i++;
            v.random_update();
            ScoreType score_next = v.get_score();
            double prob = Temp::p_move(score_cur, score_next, TempCur);
            if (!rng.judge(prob)) v.rollback();
            else score_cur = score_next;
        }
    }
};


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
    std::vector<std::pair<int, int>> E;
    process_map mp;
    for (auto [s, t] : read_csv("../testcase/random_small.csv")) {
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
        write_csv("../testcase/random_small_ans_lp.csv", ans);
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
        write_csv("../testcase/random_small_ans_lp.csv", ans);
    }
}