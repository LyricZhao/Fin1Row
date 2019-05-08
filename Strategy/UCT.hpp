# define MAX_COLS 12
# define MAX_ROWS 12
# define MAX_STATES (2 << 20)
# define TIME_LIMIT 2.0
# define RAND_LIMIT (2 << 20)
# define VITALITY_COEFFICIENT 0.8

# define USR_INDEX 1
# define MCH_INDEX 2
# define TIE_INDEX 3

# define USR_WIN_PROFIT -1
# define MCH_WIN_PROFIT  1
# define TIE_PROFIT 0

# include <ctime>
# include <cmath>
# include <cstdio>
# include <vector>
# include <cstring>
# include <cassert>
# include <iostream>
# include <algorithm>

# include "Judge.h"

char display[4] = {'.', 'A', 'B', 'X'};

int node_tot;
int nodes[MAX_STATES][MAX_COLS], cn[MAX_STATES], fa[MAX_STATES];
double cq[MAX_STATES];

/*
 *  Board definition
 *  n cols, m rows
 *  term: 0/Playing, 1/A Wins, 2/B Wins, 3/Tie
 */

struct Board {
    int m, n, nox, noy;
    int la_x, la_y, next, term;
    int board[MAX_ROWS][MAX_COLS], top[MAX_COLS];
    void clear() {
        for(int i = 0; i < MAX_ROWS; ++ i) {
            memset(board[i], 0, sizeof(int) * MAX_COLS);
        } 
        memset(top, 0, sizeof(top));
        return;
    }
    int getTop(int col) {
        for (int i = 0; i < m; ++ i) {
            if (get(col, i) == 0 && !unreachable(col, i)) {
                return i;
            }
        }
        return -1;
    }
    void init(int _m, int _n, int _la_x, int _la_y, int _nox, int _noy, int **_board) {
        term = 0, next = 1, la_x = _la_y, la_y = m - _la_y - 1;
        m = _m, n = _n, nox = _noy, noy = _m - _nox - 1;
        for (int i = 0; i < m; ++ i) {
            memcpy(board[i], _board[i], sizeof(int) * n);
        }
        for (int i = 0; i < n; ++ i) {
            top[i] = getTop(i);
        }
        return;
    }
    inline bool unreachable(int x, int y) {
        return x == nox && y == noy;
    }
    inline int get(int x, int y) {
        return board[m - y - 1][x];
    }
    void place(int x, int player) {
        int y = top[x];
        board[m - y - 1][x] = player;
        top[x] = getTop(x);
        return;
    }
    bool isWin() {
        if (3 - next == USR_INDEX) {
            term = userWin(la_x, la_y, m, n, (int* const*)board) ? USR_INDEX : term;
        } else {
            term = machineWin(la_x, la_y, m, n, (int* const*)board) ? MCH_INDEX : term;
        }
        return (term > 0);
    }
    bool terminal() {
        if (isWin()) {
            return true;
        }
        for (int i = 0; i < n; ++ i) {
            if (top[i] != -1) {
                term = 0;
                return false;
            }
        }
        term = 3;
        return true;
    }
    int expandState(int v) {
        std:: vector<int> availables;
        for (int i = 0; i < n; ++ i) {
            if (top[i] != -1 && !nodes[v][i]) {
                availables.push_back(i);
            }
        }
        if(!availables.size()) {
            return -1;
        }
        return availables[rand() % availables.size()];
    }
    int randomAction(int v) {
        std:: vector<int> availables;
        for (int i = 0; i < n; ++ i) {
            if (top[i] != -1) {
                availables.push_back(i);
            }
        }
        return availables[rand() % availables.size()];
    }
    double profit() {
        if (term == USR_INDEX) return USR_WIN_PROFIT;
        if (term == MCH_INDEX) return MCH_WIN_PROFIT;
        if (term == TIE_INDEX) return TIE_PROFIT;
        assert(0);
        return 1e7;
    }
    void take(int action) {
        place(action, next);
        next = 3 - next;
        return;
    }
    void print() {
        for (int i = 0; i < m; ++ i, puts("")) {
            for (int j = 0; j < n; ++ j) {
                int k = get(j, m - i - 1);
                k = unreachable(i, j) ? 3 : k;
                printf("%c", display[k]);
            }
        }
        return;
    }
} state;

void clear() {
    state.clear();
    memset(nodes, 0, sizeof(int) * MAX_COLS * node_tot);
    memset(cn, 0, sizeof(int) * node_tot);
    memset(fa, 0, sizeof(int) * node_tot);
    memset(cq, 0, sizeof(double) * node_tot);
    node_tot = 1;
    return;
}

void init(int m, int n, int nox, int noy, int **board) {
    clear();
    state.init(m, n, nox, noy, board);
    return;
}

int bestChild(int v, double c) {
    int best = -1, ch;
    double max_profit = -1;
    for (int i = 0; i < state.n; ++ i) if(ch = nodes[v][i]) {
        double profit = ((double)(cq[ch])) / cn[ch] + c * sqrt(2.0 * log(cn[v]) / cn[ch]);
        if (profit > max_profit) {
            max_profit = profit;
            best = ch;
        }
    }
    return best;
}

/* TBC: Change board state */
int expand(int v, int action) {
    nodes[v][action] = node_tot;
    fa[node_tot ++] = v;
    state.take(action);
    return node_tot - 1;
}

int treePolicy(int v) {
    while (!state.terminal()) {
        int action = state.expandState(v);
        if (action != -1) {
            return expand(v, action);
        } else {
            v = bestChild(v, VITALITY_COEFFICIENT);
        }
    }
    return v;
}

void backup(int v, double delta) {
    while (true) {
        cn[v] += 1;
        cq[v] += delta;
        delta = -delta;
        if (v == 0) break;
        v = fa[v];
    }
    return;
}

double defaultPolicy(int v) {
    while (!state.terminal()) {
        int action = state.randomAction();
        state.take(action);
    }
    return state.profit();
}

int calc() {
    clock_t start_time = clock();
    int count = 0;
    while ((clock() - start_time) / CLOCKS_PER_SEC < TIME_LIMIT && (count ++ ) < RAND_LIMIT) {
        int vl = treePolicy(0);
        double delta = defaultPolicy(vl);
        backup(vl, delta);
    }
    return bestChild(0, 0);
}