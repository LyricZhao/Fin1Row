# define MAX_COLS 12
# define MAX_ROWS 12
# define MAX_ACTIONS 200
# define MAX_STATES (1 << 20)
# define TIME_LIMIT 2
# define RAND_LIMIT (1 << 20)
# define VITALITY_COEFFICIENT 0.80

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

const float eps = 1e-6;
char display[4] = {'.', 'A', 'B', 'X'};

int node_tot = 0;
int nodes[MAX_STATES][MAX_COLS], cn[MAX_STATES], fa[MAX_STATES], depth[MAX_STATES], cq[MAX_STATES];
float tmp_profit[MAX_COLS];

/*
 *  Board definition
 *  n cols, m rows
 *  term: 0/Playing, 1/A Wins, 2/B Wins, 3/Tie
 *  I'm machine
 */

struct Board {
    int m, n, nox, noy;
    int la_x, la_y, next, term;
    int **board, *top;
    Board() {
        m = 0;
        clear();
    }
    void clear() {
        if (m) {
            for(int i = 0; i < m; ++ i) {
                delete board[i];
            }
            delete board, delete top;
        }
        m = n = nox = noy = la_x = la_y = next = term = 0;
        board = nullptr;
        top = nullptr;
        return;
    }
    void copy(const Board &origin) {
        la_x = origin.la_x, la_y = origin.la_y, next = origin.next, term = origin.term;
        for (int i = 0; i < m; ++ i) {
            memcpy(board[i], origin.board[i], sizeof(int) * n);
        }
        memcpy(top, origin.top, sizeof(int) * n);
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
        m = _m, n = _n, nox = _noy, noy = _m - _nox - 1;
        term = 0, next = MCH_INDEX, la_x = _la_y, la_y = m - _la_x - 1;
        board = new int*[m];
        for (int i = 0; i < m; ++ i) {
            board[i] = new int[n];
            memcpy(board[i], _board[i], sizeof(int) * n);
        }
        top = new int[n];
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
        la_x = x, la_y = y;
        board[m - y - 1][x] = player;
        top[x] = getTop(x);
        return;
    }
    bool isWin() {
        int lx = m - la_y - 1, ly = la_x;
        if (lx == -1) {
            return false;
        }
        if (3 - next == USR_INDEX) {
            term = userWin(lx, ly, m, n, (int* const*)board) ? USR_INDEX : term;
        } else {
            term = machineWin(lx, ly, m, n, (int* const*)board) ? MCH_INDEX : term;
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
    int mustAction() {
        int action = -1, y;
        for (int i = 0; i < n; ++ i) {
            if (top[i] != -1) {
                y = top[i];
                board[m - y - 1][i] = next;
                bool usr_win = false, mch_win = false;
                if (next == USR_INDEX) usr_win = userWin(m - y - 1, i, m, n, (int* const *)board);
                if (next == MCH_INDEX) mch_win = machineWin(m - y - 1, i, m, n, (int* const*)board);
                board[m - y - 1][i] = 0;
                if (!usr_win && !mch_win) continue;
                return action;
            }
        }
        return action;
    }
    int expandState(int v) {
        int must = mustAction();
        if (must != -1) return must;
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
    int randomAction() {
        int must = mustAction();
        if (must != -1) return must;
        std:: vector<int> availables;
        for (int i = 0; i < n; ++ i) {
            if (top[i] != -1) {
                availables.push_back(i);
            }
        }
        return availables[rand() % availables.size()];
    }
    int profit() {
        if (term == USR_INDEX) return USR_WIN_PROFIT;
        if (term == MCH_INDEX) return MCH_WIN_PROFIT;
        if (term == TIE_INDEX) return TIE_PROFIT;
        assert(0);
        return -1;
    }
    void take(int action) {
        place(action, next);
        next = 3 - next;
        return;
    }
    void print() {
        for (int i = 0; i < m; ++ i, std:: cerr << std:: endl) {
            for (int j = 0; j < n; ++ j) {
                int k = get(j, m - i - 1);
                k = unreachable(i, j) ? 3 : k;
                std:: cerr << display[k];
            }
        }
        return;
    }
} state, origin;

void clear() {
    state.clear();
    origin.clear();
    memset(nodes, 0, sizeof(int) * MAX_COLS * node_tot);
    memset(cn, 0, sizeof(int) * node_tot);
    memset(fa, 0, sizeof(int) * node_tot);
    memset(depth, 0, sizeof(int) * node_tot);
    memset(cq, 0, sizeof(float) * node_tot);
    node_tot = 1;
    return;
}

void init(int m, int n, int la_x, int la_y, int nox, int noy, int **board) {
    clear();
    state.init(m, n, la_x, la_y, nox, noy, board);
    origin.init(m, n, la_x, la_y, nox, noy, board);
    return;
}

inline int myAbs(int x) {
    return x > 0 ? x : -x;
}

int bestChild(int v, float c) {
    int best = -1, ch;
    float max_profit = -1e7;
    std:: vector<int> bests;
    for (int i = 0; i < state.n; ++ i) {
        ch = nodes[v][i];
        if (ch) tmp_profit[i] = ((depth[ch] & 1) ? 1.0 : -1.0) * cq[ch] / cn[ch] + c * sqrt(2.0 * log(cn[v]) / cn[ch]);
        else tmp_profit[i] = -1e9;
        if (tmp_profit[i] > max_profit) max_profit = tmp_profit[i];
    }
    for (int i = 0; i < state.n; ++ i) {
        if (fabs(tmp_profit[i] - max_profit) < eps) {
            bests.push_back(i);
        }
    }
    return bests[bests.size() >> 1];
}

inline int expand(int v, int action) {
    nodes[v][action] = node_tot;
    depth[node_tot] = depth[v] + 1;
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
            action = bestChild(v, VITALITY_COEFFICIENT);
            state.take(action);
            v = nodes[v][action];
        }
    }
    return v;
}

void backup(int v, int delta) {
    while (true) {
        cn[v] += 1;
        cq[v] += delta;
        if (v == 0) break;
        v = fa[v];
    }
    return;
}

int defaultPolicy(int v) {
    while (!state.terminal()) {
        int action = state.randomAction();
        state.take(action);
    }
    return state.profit();
}

int calc() {
    std:: cerr << "Calculating ... ";
    clock_t start_time = clock();
    int count = 0;
    while (((double)(clock() - start_time)) / CLOCKS_PER_SEC < TIME_LIMIT && (count ++ ) < RAND_LIMIT) {
        int vl = treePolicy(0);
        int delta = defaultPolicy(vl);
        backup(vl, delta);
        state.copy(origin);
    }
    int action = bestChild(0, 0);
    std:: cerr << "done! (" << count - 1 << " times, " << 1.0 * cn[nodes[0][action]] / cn[0] << " confidence)" << std:: endl;
    return action;
}