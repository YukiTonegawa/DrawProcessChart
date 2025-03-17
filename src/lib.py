import sys
import networkx as nx
import matplotlib.pyplot as plt


# csvファイルを読み込んで辺集合を返す
def read_csv(path_in):
    with open(path_in) as f:
        # グラフ
        G = nx.DiGraph()
        # 辺集合
        E = []
        for ln in f:
            # 空の行を読み込んだら終了
            if ln == '':
                break
            s, t = ln.split(',')

            E.append((s, t))
        return E
""""
# csvファイルを読み込む
def read_csv(path_in):
    with open(path_in) as f:
        # グラフ
        G = nx.DiGraph()
        # 辺集合
        E = []
        for ln in f:
            # 空の行を読み込んだら終了
            if ln == '':
                break
            s, t = ln.split(',')

            E.append((int(s[1:]), int(t[1:])))
        G.add_edges_from(E)
        pos = nx.spring_layout(G)
        # グラフの可視化
        nx.draw(G, pos,with_labels=True, arrows=True)
        plt.show()

read_csv(sys.argv[1])
"""