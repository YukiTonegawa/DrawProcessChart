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
#include <numeric>

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

// 斜めの辺の長さの総和
double sum_edge_length_naname(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    double ans = 0;
    for (auto [s, t] : E) {
        auto [sx, sy] = pos[s];
        auto [tx, ty] = pos[t];
        int dx = tx - sx;
        int dy = ty - sy;
        if (dy != 0) ans += std::sqrt(dx * dx + dy * dy);
    }
    return ans;
}

// 無視できない貫通を数える
int count_bad_penetration(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    int N = pos.size();
    std::vector<int> idx(N);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return pos[a].first < pos[b].first; });
    std::vector<std::vector<bool>> mat(N, std::vector<bool>(N, false));
    for (auto [s, t] : E) {
        mat[s][t] = true;
    }
    int ans = 0;

    for (auto [s, t] : E) {
        int dx = pos[t].first - pos[s].first;
        int dy = pos[t].second - pos[s].second;
        int g = std::gcd(dx, dy);
        dx /= g;
        dy /= g;
        int x = pos[s].first + dx, y = pos[s].second + dy;
        int v = s;
        while (x != pos[t].first) {
            int next = -1;
            for (int i = 0; i < N; i++) {
                if (pos[i].first == x && pos[i].second == y) {
                    next = i;
                    break;
                }
            }
            if (next != -1) {
                if (!mat[v][next]) ans++;
                v = next;
            }
            x += dx;
            y += dy;
        }
    }
    return ans;
}

// 無視できない貫通に関与する辺の長さの和
int sum_edge_length_bad_penetration(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    int N = pos.size();
    std::vector<int> idx(N);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return pos[a].first < pos[b].first; });
    std::vector<std::vector<bool>> mat(N, std::vector<bool>(N, false));
    for (auto [s, t] : E) {
        mat[s][t] = true;
    }
    int ans = 0;

    for (auto [s, t] : E) {
        int dx = pos[t].first - pos[s].first;
        int dy = pos[t].second - pos[s].second;
        int g = std::gcd(dx, dy);
        dx /= g;
        dy /= g;
        int x = pos[s].first + dx, y = pos[s].second + dy;
        int v = s;
        int dxsum = dx, dysum = dy;
        while (x != pos[t].first) {
            int next = -1;
            for (int i = 0; i < N; i++) {
                if (pos[i].first == x && pos[i].second == y) {
                    next = i;
                    break;
                }
            }
            if (next != -1) {
                if (!mat[v][next]) {
                    ans += std::sqrt(dxsum * dxsum + dysum * dysum);
                }
                v = next;
            }
            x += dx;
            y += dy;
            dxsum += dx;
            dysum += dy;
        }
    }
    return ans;
}

// 全ての貫通を数える
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

// 辺が交差する回数
int count_edge_cross(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    int ans = 0;
    int M = E.size();
    for (int i = 0; i < M; i++) {
        auto [a, b] = E[i];
        auto [ax, ay] = pos[a];
        auto [bx, by] = pos[b];
        for (int j = i + 1; j < M; j++) {
            auto [c, d] = E[j];
            auto [cx, cy] = pos[c];
            auto [dx, dy] = pos[d];
            assert(ax != bx);
            assert(cx != dx);
            double s1 = double(by - ay) / (bx - ax);
            double s2 = double(dy - cy) / (dx - cx);
            // y = (by - ay) / (bx - ax) * (x - ax) + ay
            // y = (dy - cy) / (dx - cx) * (x - cx) + cy
            // y = s1x - s1ax + ay
            // y = s2x - s2cx + cy
            // (s1 - s2)x = s1ax - ay - s2cx + cy
            if (s1 != s2) {
                double cross_x = (s1 * ax - ay - s2 * cx + cy) / (s1 - s2);
                if (ax < cross_x && cross_x < bx && cx < cross_x && cross_x < dx) {
                    ans++;
                } 
            }
        }
    }
    return ans;
}

// (辺の長さの総和) + a(無視できない貫通に関与する辺の長さの和) + b(斜めの辺の長さの和)
double calc_score(const std::vector<std::pair<int, int>> &pos, const std::vector<std::pair<int, int>> &E) {
    // 定数
    static constexpr double a = 1.0;
    static constexpr double b = 1.0;

    double lensum = sum_edge_length(pos, E);
    double p_lensum = sum_edge_length_bad_penetration(pos, E);
    double naname_lensum = sum_edge_length_naname(pos, E);
    return lensum + a * p_lensum + b * naname_lensum;
}

/*
oooooo
oo
    oo
という3つのパスがある場合
oooooo
oo  oo
とできる
*/
std::vector<std::pair<int, int>> compress_y(const std::vector<std::vector<int>> &P, const std::vector<int> &perm, const std::vector<int> &X) {
    int N = X.size(), R = P.size();
    int l = 0, y = 0;
    std::vector<std::pair<int, int>> ans(N);
    while (l < R) {
        std::vector<bool> used(N, false);
        int r = l;
        while (r < R) {
            int lx = X[P[perm[r]][0]];
            int rx = X[P[perm[r]].back()];
            if (rx >= used.size()) {
                used.resize(rx + 1, false);
            }
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
#endif