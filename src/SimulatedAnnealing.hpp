#ifndef _SIMULATED_ANNEALING_H_
#define _SIMULATED_ANNEALING_H_
#include "Random.hpp"
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
template<typename Timer, typename Temp, typename State>
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
            if (!rng.Judge(prob)) v.rollback();
            else score_cur = score_next;
        }
    }
};
#endif