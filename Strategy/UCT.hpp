# define MAX_COLS 12
# define MAX_ROWS 12
# define MAX_ACTIONS 200
# define MAX_STATES (1 << 23)
# define TIME_LIMIT 2.7
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
# include <stack>
# include <iostream>
# include <algorithm>

# include "Judge.h"

const float eps = 1e-6;
char display[4] = {'.', 'A', 'B', 'X'};

int last_action, root, mch_node_trans, usr_node_trans, usr_last_action;
std:: stack<int> available_ids;
int nodes[MAX_STATES][MAX_COLS], cn[MAX_STATES], fa[MAX_STATES], depth[MAX_STATES];
float cq[MAX_STATES], tmp_profit[MAX_COLS];

clock_t start_time;

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
        n = origin.n, m = origin.m, nox = origin.nox, noy = origin.noy;
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
        clear();
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
    int randomAction() {
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
    bool initState() {
        for (int i = 0; i < m; ++ i) {
            for (int j = 0; j < n; ++ j) {
                if (board[i][j] == MCH_INDEX) return false;
            }
        }
        return true;
    }
    int contain(Board &last) {
        if (nox != last.nox || noy != last.noy || n != last.n || m != last.m) return -1;
        int a_count = 0, b_count = 0, usr_action;
        for (int i = 0; i < m; ++ i) {
            for (int j = 0; j < n; ++ j) {
                if ((last.board[i][j] | board[i][j]) != board[i][j]) return -1;
                if (last.board[i][j] != board[i][j]) {
                    a_count += (board[i][j] == USR_INDEX);
                    b_count += (board[i][j] == MCH_INDEX);
                    if (board[i][j] == USR_INDEX) usr_action = j;
                }
            }
        }
        if(a_count == 1 && b_count == 1) {
            return usr_action;
        }
        return -1;
    }
} state, origin, last;

/* All global clear */
void clear() {
    root = 1;
    cn[1] = cq[1] = fa[1] = depth[1] = 0;
    memset(nodes[1], 0, sizeof(int) * MAX_COLS);
    while(!available_ids.empty()) available_ids.pop();
    for (int i = MAX_STATES - 1; i > 1; -- i) available_ids.push(i);
    return;
}

void init(int m, int n, int la_x, int la_y, int nox, int noy, int **board) {
    start_time = clock();
    usr_last_action = la_y;
    state.init(m, n, la_x, la_y, nox, noy, board);
    origin.init(m, n, la_x, la_y, nox, noy, board);
    if (origin.initState()) {
        clear();
        last.init(m, n, la_x, la_y, nox, noy, board);
        last.n = -1;
    }
    return;
}

int bestChild(int v, float c) {
    int ch;
    float max_profit = -1e10;
    std:: vector<int> bests;
    for (int i = 0; i < state.n; ++ i) {
        ch = nodes[v][i];
        if (ch) {
            tmp_profit[i] = ((depth[ch] & 1) ? 1.0 : -1.0) * cq[ch] / cn[ch] + c * sqrt(2.0 * log(cn[v]) / cn[ch]);
        }
        else tmp_profit[i] = -1e13;
        if (tmp_profit[i] > max_profit) max_profit = tmp_profit[i];
    }
    for (int i = 0; i < state.n; ++ i) {
        if (nodes[v][i] && fabs(tmp_profit[i] - max_profit) < eps) {
            bests.push_back(i);
        }
    }
    return bests[bests.size() >> 1];
}

inline int expand(int v, int action) {
    int id = available_ids.top();
    available_ids.pop();
    nodes[v][action] = id;
    depth[id] = depth[v] ^ 1;
    fa[id] = v, cn[id] = cq[id] = 0;
    memset(nodes[id], 0, sizeof(int) * MAX_COLS);
    state.take(action);
    return id;
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
        if (v == root) break;
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

void transferDFS(int v) {
    int ch;
    for (int i = 0; i < state.n; ++ i) {
        ch = nodes[v][i];
        if (ch) {
            if (ch == usr_node_trans) {
                continue;
            }
            transferDFS(ch);
        }
    }
    memset(nodes[v], 0, sizeof(int) * MAX_COLS);
    available_ids.push(v);
    return;
}

int calc() {
    int usr_step = origin.contain(last);
    float reused_ratio = 0;
    if (usr_step != -1) {
        mch_node_trans = nodes[root][last_action];
        usr_node_trans = nodes[mch_node_trans][usr_step];
        if (!mch_node_trans || !usr_node_trans) {
            clear();
        } else {
            reused_ratio = 1.0 * cn[usr_node_trans] / cn[root];
            transferDFS(root);
            root = usr_node_trans;
            fa[root] = depth[root] = 0;
        }
    }

    std:: cerr << "Calculating ... ";
    int count = 0;
    while (((float)(clock() - start_time)) / CLOCKS_PER_SEC < TIME_LIMIT && (count ++ ) < RAND_LIMIT) {
        int vl = treePolicy(root);
        int delta = defaultPolicy(vl);
        backup(vl, delta);
        state.copy(origin);
    }
    int action = bestChild(root, 0);
    std:: cerr << "done! (" << count - 1 << " times, " << (1.0 * cq[root] / cn[root] + 1) / 2.0 << " confidence, " << reused_ratio << " reused, " << cn[root] << " nodes)" << std:: endl;
    last.copy(origin);
    last_action = action;
    return action;
}