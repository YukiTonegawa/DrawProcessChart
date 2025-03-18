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
    // diff_scoreが0以上     1 
    //    0未満             e ^ {diff_score / Tcur}
    template<typename T>
    static double p_move(T score_before, T score_after, double Tcur) {
        T diff_score = score_after - score_before;
        return diff_score >= 0 ? 1 : std::exp((double)diff_score / Tcur);
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


// 各工程の位置
struct node {
    int x, y;
};

/*
更新の種類
1工程の横方向を1増加/減少可能ならそうする
1工程の縦方向
*/
struct StateSA {
    using UpdateType = std::tuple<int, int, int>;
    using ScoreType = int;
    ScoreType score;
    //std::vector<std::vector<int>> 

    StateSA() {}
    void random_update() {
        //
    }
    void rollback() {
        //
    }
    std::pair<UpdateType, ScoreType> get_next() {
        return {UpdateType{}, score};
    }
   
    ScoreType get_score() {
        return score;
    }
};

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