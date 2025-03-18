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

// 文字列sをdelimでsを分割
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

// 入力ファイルpath_inを読み込んで辺集合を返す
std::vector<std::pair<std::string, std::string>> read_csv(std::string path_in) {
    std::ifstream ifs(path_in, std::ios::in);
    std::string s;
    std::vector<std::pair<std::string, std::string>> E;
    while (std::getline(ifs, s)) {
        if (!s.empty() && s.back() == '\r') { // todo: osに依存しない改行文字
            s.pop_back();
        }
        if (!s.empty()) {
            auto e = split(s, ',');
            E.push_back({e[0], e[1]});
        }
    }
    return E;
}

// path_outに出力
void write_csv(std::string path_out, std::vector<std::tuple<std::string, int, int>> P) {
    std::ofstream ofs(path_out);
    for (auto [name, x, y] : P) {
        ofs << name << ',' << x << ',' << y << std::endl;
    }
    ofs.close();
}

// 工程名と番号を1対1対応させるmap
struct process_map {
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