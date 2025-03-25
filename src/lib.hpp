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
    int register_process(std::string s) {
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
    int get_id(std::string s) const {
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




std::vector<int> calc_min_x(int N, std::vector<std::pair<int, int>> E) {
    std::vector<std::vector<int>> G(N);
    for (auto [s, t] : E) {
        G[s].push_back(t);
    }
    std::vector<int> in(N, 0), X(N);
    for (auto [s, t] : E) {
        in[t]++;
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

std::vector<std::vector<int>> decompose_long_path(int N, std::vector<std::pair<int, int>> E) {
    std::vector<int> dep(N, -1), next(N, -1), used(N, 0);
    std::vector<std::vector<int>> G(N), res;
    for (auto [s, t] : E) {
        G[s].push_back(t);
    }
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
        res.push_back({});
        while (v != -1) {
            res.back().push_back(v);
            used[v] = true;
            v = next[v];
        }
        for (int i = 0; i < N; i++) {
            if (!used[i]) {
                dep[i] = next[i] = -1;
            }
        }
    }
    return res;
}

// 辺の長さの総和を返す
double sum_edge_length(std::vector<std::pair<int, int>> P, std::vector<std::pair<int, int>> E) {
    // 多重辺を省く
    std::sort(E.begin(), E.end());
    E.erase(std::unique(E.begin(), E.end()), E.end());

    double ans = 0;
    for (auto [s, t] : E) {
        auto [sx, sy] = P[s];
        auto [tx, ty] = P[t];
        int dx = tx - sx;
        int dy = ty - sy;
        ans += std::sqrt(dx * dx + dy * dy);
    }
    return ans;
}
#endif