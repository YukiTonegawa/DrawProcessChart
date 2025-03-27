#ifndef _LIB_H_
#define _LIB_H_
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <tuple>
#include <map>
#include <algorithm>
#include <cmath>
#include <queue>
#include <cassert>

// 工程名と番号を1対1対応させるmap
struct ProcessMap {
  private:
    std::map<std::string, int> _mp;
    std::vector<std::string> _S;
  
  public:
    // 登録されている工程の数
    int size() const {
        return _S.size();
    }

    // 工程名sを登録してその番号(登録順の非負整数)を返す. すでに登録されている場合はその番号を返す
    int register_process(const std::string &s) {
        auto itr = _mp.find(s);
        if (itr == _mp.end()) {
            int next_id = _mp.size();
            _mp.emplace(s, next_id);
            _S.push_back(s);
            return next_id;
        } else {
            return itr->second;
        }
    }

    // 工程名sが登録されている場合その番号, そうでない場合-1を返す
    int get_id(const std::string &s) const {
        auto itr = _mp.find(s);
        if (itr == _mp.end()) {
            return -1;
        } else {
            return itr->second;
        }
    }

    // 番号idの工程名を取得, そのような工程が存在しない場合空文字列を返す
    std::string get_process(int id) const {
        if (0 <= id && id < _S.size()) {
            return _S[id];
        } else {
            return "";
        }
    }
};

// 辺集合 -> 隣接リスト
std::vector<std::vector<int>> adjacency_list(int N, const std::vector<std::pair<int, int>> &E) {
    std::vector<std::vector<int>> ans(N);
    for (auto [s, t] : E) {
        assert(0 <= s && s < N);
        assert(0 <= t && t < N);
        ans[s].push_back(t);
    }
    return ans;
}

// 多重辺を省く
std::vector<std::pair<int, int>> remove_multiple_edge(std::vector<std::pair<int, int>> E) {
    std::sort(E.begin(), E.end());
    E.erase(std::unique(E.begin(), E.end()), E.end());
    return E;
}

/*
負のx座標を使わないことにすると、各工程が存在できる最小のx座標が決まる。
この値を計算する
O(N)
*/
std::vector<int> calc_min_x(const std::vector<std::vector<int>> &G) {
    int N = G.size();
    std::vector<int> in(N, 0), X(N);
    for (int i = 0; i < N; i++) {
        for (int t : G[i]) {
            in[t]++;
        }
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
    return X;
}

/*
DAGなので最長パスが計算できる
最長パスを取り去ることを繰り返して(全頂点使うまで)いくつかのパスに分解
O(N^2)
*/
std::vector<std::vector<int>> decompose_long_path(const std::vector<std::vector<int>> &G) {
    int N = G.size();
    std::vector<int> dep(N, -1), next(N, -1), used(N, 0);
    std::vector<std::vector<int>> ans;

    auto dfs = [&](auto &&dfs, int v) -> int {
        if (used[v]) return -1;
        if (dep[v] != -1) return dep[v];
        dep[v] = 0;
        for (int t : G[v]) {
            int d = dfs(dfs, t) + 1;
            if (dep[v] < d) {
                dep[v] = d;
                next[v] = t;
            }
        }
        return dep[v];
    };
    
    while (true) {
        std::pair<int, int> M = {-1, -1};
        for (int i = 0; i < N; i++) {
            dfs(dfs, i);
            if (!used[i]) M = std::max(M, {dep[i], i});
        }
        if (M.first == -1) break;
        int v = M.second;
        ans.push_back({});
        while (v != -1) {
            ans.back().push_back(v);
            used[v] = true;
            v = next[v];
        }
        for (int i = 0; i < N; i++) {
            if (!used[i]) {
                dep[i] = next[i] = -1;
            }
        }
    }
    return ans;
}

// 辺の長さの総和を返す
double sum_edge_length(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    double ans = 0;
    for (auto [s, t] : E) {
        auto [sx, sy] = pos[s];
        auto [tx, ty] = pos[t];
        int dx = tx - sx;
        int dy = ty - sy;
        ans += std::sqrt(dx * dx + dy * dy);
    }
    return ans;
}

int count_bad_penetration(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    /*
    int ans = 0;
    for (auto [s, t] : E) {
        //
    }
    return ans;
    */
    return 0;
}

int count_all_penetration(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    int N = pos.size();
    int ans = 0;
    for (auto [s, t] : E) {
        auto [sx, sy] = pos[s];
        auto [tx, ty] = pos[t];
        for (int i = 0; i < N; i++) {
            auto [ix, iy] = pos[i];
            if (sx < ix && ix < tx && (iy - ty) * (tx - sx) == (ty - sy) * (ix - sx)) {
                ans++;
            }
        }
    }
    return ans;
}

// 辺の長さの総和 * (1 + 貫通 / 辺の数)
double calc_score(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    double len = sum_edge_length(pos, E);
    int num_pen = count_all_penetration(pos, E);
    int num_e = E.size();
    return len * (1.0 + ((double)num_pen / num_e));
}

/*
1. 無視できる貫通
A -> B -> C -> D
 ------------->

A->Dがビジュアライザ上でB, Cを貫通することによってA->B && B->C && C->Dと読み間違える可能性がある
読み間違えの有無に関わらず情報量が変わらない場合無視できる

2. 許容できる貫通
中心部を通らない
ビジュアライザ上で角を掠める程度の貫通の場合許容できる

3. 許容できない貫通
中心部を通る = 直線がある格子点を通り、そこに工程が置かれている
*/
#endif