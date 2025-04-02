#include "CheckLib.hpp"
#include "Lib.hpp"

// 文字列sをdelimで分割
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

// サンプルのスコア計算用
int main() {
    std::string path_in = "../testcase/case1.csv";
    std::string path_out = "../testcase/case1_ans.csv";

    std::vector<std::pair<int, int>> E;
    ProcessMap mp;
    for (auto [s, t] : CheckLib::read_csv(path_in)) {
        int sid = mp.register_process(s);
        int tid = mp.register_process(t);
        E.push_back({sid, tid});
    }
    const int N = mp.size();
    std::vector<std::pair<int, int>> pos(N);

    // 位置を読み込む
    std::ifstream ifs(path_out, std::ios::in);
    std::string s;
    while (std::getline(ifs, s)) {
        if (!s.empty()) {
            auto e = split(s, ',');
            int id = mp.get_id(e[0]);
            pos[id].first = std::stoi(e[1]);
            pos[id].second = std::stoi(e[2]);
        }
    }
    double score = calc_score(pos, E);
    std::cout << "score is " << score << '\n';
    std::cout << "lensum is " << sum_edge_length(pos, E) << '\n';
    std::cout << "cross is " << count_edge_cross(pos, E) << '\n';
    std::cout << "penetration is " << count_bad_penetration(pos, E) << '\n';

}