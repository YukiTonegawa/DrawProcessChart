#include "lib.hpp"

// 貫通を解消
std::vector<std::pair<int, int>> solve_penetration(std::vector<std::pair<int, int>> P, std::vector<std::pair<int, int>> E) {
    const int N = P.size();
    std::vector<std::vector<int>> Col(N);
    for (int i = 0; i < N; i++) {
        auto [x, y] = P[i];
        if ((int)Col[x].size() < y + 1) {
            Col[x].resize(y + 1, -1);
        }
        Col[x][y] = i;
    }

    // a, b, cが右向きの一直線にあるか
    auto check_straight_line = [&](int a, int b, int c) -> bool {
        auto [ax, ay] = P[a];
        auto [bx, by] = P[b];
        auto [cx, cy] = P[c];
        if (bx <= ax || cx <= bx) return false;
        int dx_ab = bx - ax;
        int dy_ab = by - ay;
        int dx_ac = cx - ax;
        int dy_ac = cy - ay;
        return dy_ab * dx_ac == dy_ac * dx_ab;
    };

    std::vector<std::vector<bool>> M(N, std::vector<bool>(N, false));
    for (auto [a, b] : E) M[a][b] = true;

    for (int y = 0;; y++) {
        bool ok = false;
        for (int x = 0; x < N; x++) {
            if (y < (int)Col[x].size()) {
                ok = true;
            } else {
                continue;
            }
            int id = Col[x][y];
            if (id == -1) continue;
            bool ok2 = true;
            
            for (int a = 0; a < N && ok2; a++) {
                for (int b = a + 1; b < N && ok2; b++) {
                    int A = a, B = b, C = id;
                    if (P[A].first > P[B].first) std::swap(A, B);
                    if (P[B].first > P[C].first) std::swap(B, C);
                    if (P[A].first > P[B].first) std::swap(A, B);
                    if (M[A][C] && check_straight_line(A, B, C)) {
                        ok2 = false;
                    }
                }
            }

            if (!ok2) {
                Col[x].insert(Col[x].begin() + y, -1);
                for (int t = y + 1; t < (int)Col[x].size(); t++) {
                    id = Col[x][t];
                    if (id == -1) continue;
                    P[id].second = t;
                }
            }
        }
        if (!ok) break;
    }
    return P;
}