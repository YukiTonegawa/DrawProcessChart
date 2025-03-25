#include "CheckLib.hpp"
#include "Lib.hpp"

// サンプルのスコア計算用
int main() {
    std::string path_in = "../testcase/case2.csv";
    std::string path_out = "../testcase/case2_ans.csv";

    std::vector<std::pair<int, int>> E;
    process_map mp;
    for (auto [s, t] : CheckLib::ReadCsv(path_in)) {
        int sid = mp.register_process(s);
        int tid = mp.register_process(t);
        E.push_back({sid, tid});
    }
    /*
    const int N = mp.size();
    std::vector<int> X(N), Y(N);

    // 位置を読み込む
    std::ifstream ifs(path_out, std::ios::in);
    std::string s;
    while (std::getline(ifs, s)) {
        if (!s.empty() && s.back() == '\r') { // todo: osに依存しない改行文字
            s.pop_back();
        }
        if (!s.empty()) {
            auto e = split(s, ',');
            int id = mp.get_id(e[0]);
            X[id] = std::stoi(e[1]);
            Y[id] = std::stoi(e[2]);
        }
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
    */
}