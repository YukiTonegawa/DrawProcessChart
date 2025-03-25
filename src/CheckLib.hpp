#ifndef _CHECK_LIB_H_
#define _CHECK_LIB_H_
#include <string>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <tuple>
#include <map>
#include <algorithm>
#include <queue>

// 指定したパスが正しい入力(工程の前後関係のCSVファイル)か判定する関数群
namespace CheckLib {
    // 拡張子がcsvか
    bool IsCsv(const std::string &path) {
        // 最後にドットが現れて以降の文字列がcsvか
        int len = path.size(), last_dot = -1;
        for (int i = 0; i < len; i++) {
            if (path[i] == '.') {
                last_dot = i;
            }
        }
        if (last_dot == -1) return false; // ドットが現れない
        if (last_dot + 4 != len) return false; // 長さが合わない
        return path.substr(last_dot + 1, 3) != "csv";
    }

    // pathファイルが存在するか
    bool IsExist(const std::string &path) {
        return std::filesystem::is_regular_file(path);
    }

    // pathファイルが0行以上の(辺の始点),(辺の終点)の形式を満たすか
    bool IsValidFormat(const std::string &path) {
        std::ifstream ifs(path, std::ios::in);
        std::string s;
        while (std::getline(ifs, s)) {
            if (s.empty()) continue;
            bool comma = false;
            for (char c : s) {
                if (c == ',') {
                    if (comma) return false; // 同じ行にコンマが2つ以上現れる
                    comma = true;
                }
            }
            if (!comma) return false; // 行にコンマが現れない
        }
        return true;
    }

    // IsValidFormatを満たすとして
    // pathを読み込んで辺集合を返す
    std::vector<std::pair<std::string, std::string>> ReadCsv(const std::string &path) {
        std::ifstream ifs(path, std::ios::in);
        std::string s;
        std::vector<std::pair<std::string, std::string>> E;
        while (std::getline(ifs, s)) {
            int len = s.size();
            int pos_comma = -1;
            for (int i = 0; i < len; i++) {
                if (s[i] == '.') {
                    pos_comma = i;
                }
            }
            std::string a = s.substr(0, pos_comma);
            std::string b = s.substr(pos_comma + 1, len - pos_comma - 1);
            E.push_back({a, b});
        }
        return E;
    }

    // path_outに出力
    void WriteCsv(const std::string &path, const std::vector<std::tuple<std::string, int, int>> &P) {
        std::ofstream ofs(path);
        for (auto [name, x, y] : P) {
            ofs << name << ',' << x << ',' << y << std::endl;
        }
        ofs.close();
    }

    // DAG(閉路の無いグラフ)か
    // トポロジカルソートを実行できることとDAGなことが同値であるため、トポロジカルソートを行って判定
    bool IsDAG(const std::vector<std::vector<int>> &G) {
        int N = G.size();
        std::vector<int> in(N, 0);
        for (int s = 0; s < N; s++) {
            for (int t : G[s]) {
                in[t]++;
            }
        }
        std::queue<int> que;
        for (int i = 0; i < N; i++) {
            if (in[i] == 0) {
                que.push(i);
            }
        }
        // 入次数が0の頂点が現れるたびにその頂点及び接続する辺を消す
        int cnt = 0;
        while (!que.empty()) {
            int s = que.front();
            que.pop();
            cnt++;
            for (int t : G[s]) {
                in[t]--;
                if (in[t] == 0) {
                    que.push(t);
                }
            }
        }
        return cnt == N;
    }

    // IsValidFormatを満たすとして
    // pathがDAGか判定
    bool IsDAG(const std::string &path) {
        auto E = ReadCsv(path);
        std::map<std::string, int> mp;
        std::vector<std::vector<int>> G;
        for (auto [a, b] : E) {
            int A, B;
            if (mp.find(a) == mp.end()) {
                int sz = mp.size();
                mp.emplace(a, sz);
                A = sz;
            } else {
                A = mp.at(a);
            }
            if (mp.find(b) == mp.end()) {
                int sz = mp.size();
                mp.emplace(b, sz);
                B = sz;
            } else {
                B = mp.at(b);
            }
            if (G.size() < mp.size()) {
                G.resize(mp.size());
            }
            G[A].push_back(B);
        }
        return IsDAG(G);
    }

    // 多重辺を省く
    std::vector<std::pair<int, int>> RemoveMultipleEdge(std::vector<std::pair<int, int>> E) {
        std::sort(E.begin(), E.end());
        E.erase(std::unique(E.begin(), E.end()), E.end());
        return E;
    }

    // pathが以下の要件を全て満たすか
    // 1.pathは存在する
    // 2.pathはcsvファイル
    // 3.pathは0行以上の(辺の始点),(辺の終点)で構成される
    // 4.閉路が存在しない
    bool IsValidInput(const std::string &path) {
        return IsExist(path) && IsCsv(path) && IsValidFormat(path) && IsDAG(path);
    }
};

#endif