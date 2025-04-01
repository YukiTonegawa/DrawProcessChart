#ifndef _BEAM_SEARCH_H_
#define _BEAM_SEARCH_H_
#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>

template<typename Timer, typename State, typename Cmp>
struct beam_search {
    using Score = typename State::Score;
    using Update = typename State::Update;

    struct node {
        std::shared_ptr<node> p;
        Update u;
        node(std::shared_ptr<node> _p, Update _u): p(_p), u(_u){}
    };
    using ptr = std::shared_ptr<node>;
    using psp = std::pair<State, ptr>;
    struct _Cmp{ bool operator ()(const psp &A, const psp &B){return Cmp()(A.first.score, B.first.score);} };

    std::vector<Update> operator ()(State s, int Width, int TimeEnd) {
        Timer::set();
        std::vector<psp> Snow;
        Snow.push_back({s, ptr(nullptr)});
        psp best{s, ptr(nullptr)};

        while (true) {
            long long te = Timer::elapse();
            if (te * 1.1 > TimeEnd) Width = 1; // 時間がない場合幅を1にする
            if (te > TimeEnd) break;
            std::vector<psp> Snext;
            for (int i = 0; i < std::min(Width, (int)Snow.size()); i++) {
                auto [s_now, v] = Snow[i];
                for (Update u : s_now.get_neighbors()) {
                    State s_next = s_now.update(u);
                    Snext.push_back({s_next, ptr(new node(v, u))});
                }
            }
            std::swap(Snow, Snext);
            if (Snow.empty()) break;
            std::sort(Snow.begin(), Snow.end(), _Cmp());
            // bestの更新
            if (Cmp()(Snow[0].first.score, best.first.score)) best = Snow[0];
            // 強制終了
            //if(Snow[0].first.is_done()) break;
        }
        // bestの操作列の復元
        std::vector<Update> res;
        ptr v = best.second;
        while (v) {
            res.push_back(v->u);
            v = v->p;
        }
        std::reverse(res.begin(), res.end());
        return res;
    }
};
#endif