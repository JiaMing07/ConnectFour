//
// Created by Colin on 2021/3/29.
//

#ifndef STRATEGY_BOARD_H
#define STRATEGY_BOARD_H

#include <cstring>
#include <vector>
#include "utils.hpp"
using namespace std;

const int8_t ME = 2;
const int8_t OPPONENT = 1;

int8_t enumed_children[15] = { 0 };

class Board
{
    int8_t** _board = nullptr;
    vector<int8_t> top;
    int8_t M = 0, N = 0;
    int8_t last_player = -1;
    int8_t last_step_y = -1;
    int8_t reward = -10;

    void switch_role()
    {
        last_player = (last_player == ME) ? OPPONENT : ME;
    }

    /**
     * @brief judge
     * @param x x of the last piece by the current last_player
     * @param y y of the last piece by the current last_player
     * @return whether the current last_player win
     */
    [[nodiscard]] bool is_win(const int8_t x, const int8_t y, const int8_t _role) const
    {
        //横向检测
        int8_t i, j;
        int8_t count = 0;
        for (i = y; i >= 0; i--)
            if (!(_board[x][i] == _role))
                break;
        count += (y - i);
        for (i = y; i < N; i++)
            if (!(_board[x][i] == _role))
                break;
        count += (i - y - 1);
        if (count >= 4) return true;

        //纵向检测
        count = 0;
        for (i = x; i < M; i++)
            if (!(_board[i][y] == _role))
                break;
        count += (i - x);
        if (count >= 4) return true;

        //左下-右上
        count = 0;
        for (i = x, j = y; i < M && j >= 0; i++, j--)
            if (!(_board[i][j] == _role))
                break;
        count += (y - j);
        for (i = x, j = y; i >= 0 && j < N; i--, j++)
            if (!(_board[i][j] == _role))
                break;
        count += (j - y - 1);
        if (count >= 4) return true;

        //左上-右下
        count = 0;
        for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
            if (!(_board[i][j] == _role))
                break;
        count += (y - j);
        for (i = x, j = y; i < M && j < N; i++, j++)
            if (!(_board[i][j] == _role))
                break;
        count += (j - y - 1);
        if (count >= 4)
            return true;

        return false;
    }

    /**
     * @return whether the board is full
     */
    [[nodiscard]] bool is_full() const
    {
        bool full = true;
        for (int8_t i = 0; i < N; i++)
        {
            if (top[i] > 0)
            {
                full = false;
                break;
            }
        }
        return full;
    }

    [[nodiscard]] int8_t get_reward() const
    {
        if (!(0 <= last_step_y && last_step_y < N))
            return -11;
        int8_t x = top[last_step_y], y = last_step_y;
        if (_board[x][y] == 3)
            x++;
        if (is_win(x, y, last_player))
        {
            if (last_player == ME) // this AI win!
            {
#ifdef OUTPUT_ROLLOUT
                cerr << "Rollout return 1\n";
            print();
#endif
                return 1;
            }
            else
            {
#ifdef OUTPUT_ROLLOUT
                cerr << "Rollout return -1\n";
            print();
#endif
                return -1;
            }
        }
        else if (is_full())
        {
#ifdef OUTPUT_ROLLOUT
            cerr << "Rollout return 0\n";
            print();
#endif
            return 0;
        }
        else
            return -11;
    }

public:
    /**
     * construct a board by int** array
     * @param _role last_player; 1: this AI; 2: provided AI
     */
    Board(int** _b, const int _M, const int _N, const int* _top, const int _last, const int _y):
            M(_M), N(_N),  top(_N), last_player(_last), last_step_y(_y)
    {
        _board = new int8_t*[M];
        for (int8_t i = 0; i < M; i++)
        {
            _board[i] = new int8_t[N];
            for (int8_t j = 0; j < N; j++)
                _board[i][j] = _b[i][j];
        }
        for (int8_t i = 0; i < N; i++)
            top[i] = _top[i];
    }

    // copy constructor
    Board(const Board& _board)
    {
        this->M = _board.M;
        this->N = _board.N;
        this->last_player = _board.last_player;
        this->top = _board.top;
        this->last_step_y = _board.last_step_y;
        this->reward = _board.reward;
        this->_board = new int8_t*[this->M];
        for (int8_t i = 0; i < this->M; i++)
        {
            this->_board[i] = new int8_t[this->N];
            memcpy(this->_board[i], _board._board[i], N);
        }
        top = _board.top;
    }

    // move constructor
    Board(Board&& _board) noexcept
    {
        this->M = _board.M;
        this->N = _board.N;
        this->last_player = _board.last_player;
        this->last_step_y = _board.last_step_y;
        this->_board = _board._board;
        this->reward = _board.reward;
        this->top = move(_board.top);
        _board._board = nullptr;
        //_board.M = _board.N = _board.last_player = 0;
    }

    void set(const int8_t _x, const int8_t _y, const int8_t _v)
    {
        _board[_x][_y] = _v;
    }

    /**
     * @brief play at (_x, _y) and update top
     */
    void play_at(const int8_t _y)
    {
        const int8_t _x = top[_y] - 1;
        switch_role();
        set(_x, _y, last_player);
        last_step_y = _y;
        // update top
        int8_t next_top = _x;
        if (next_top - 1 >= 0 && _board[next_top - 1][_y] == 3)
            next_top--;
        top[_y] = next_top;
    }

    [[nodiscard]] int8_t last_step() const
    {
        return last_step_y;
    }

    /**
     * @brief play a step forward
     */
    [[nodiscard]] int8_t enumerate_children() const
    {
        int8_t size = 0;
        for (int8_t i = 0; i < top.size(); i++)
        {
            if (top[i] > 0) // available to play at (top[i] - 1, i)
                enumed_children[size++] = i;
        }
        return size;
    }

    /**
     * @brief random simulation from current state
     * @return reward(result) of simulation
     */
    [[nodiscard]] intPair rollout()
    {
        reward = get_reward();
        if (reward > -10)
            return { reward, 1 };

        int8_t possible_col[12] = { 0 };
        while (true)
        {
            int8_t col_num = 0;
            for (int8_t i = 0; i < N; i++)
            {
                const int8_t last_top_i = top[i];
                const int8_t last_step_y_bak = last_step_y;
                if (last_top_i > 0)
                {
                    play_at(i); // try to win
                    reward = get_reward();
                    if (reward > -10)
                        return { reward, 0 };
                    else
                    {
                        // unplay
                        set(last_top_i - 1, i, 0);
                        top[i] = last_top_i;
                        switch_role();
                        last_step_y = last_step_y_bak;
                        // add possible choice for random
                        possible_col[col_num++] = i;
                    }
                }
            }
            // can't win now; simulate the opponent
            for (int8_t i = 0; i < N; i++)
            {
                const int8_t last_top_i = top[i];
                const int8_t last_step_y_bak = last_step_y;
                if (last_top_i > 0)
                {
                    switch_role();  // last step is this AI
                    play_at(i); // last step becomes the opponent
                    reward = get_reward();
                    if (reward > -10)
                    {
                        switch_role();  // last step is this AI
                        set(last_top_i - 1, i, last_player);
                        reward = -11;
                        col_num = 0;
                        break;
                    }
                    else
                    {
                        // unplay
                        set(last_top_i - 1, i, 0);
                        top[i] = last_top_i;
                        last_step_y = last_step_y_bak;
                    }
                }
            }
            if (col_num > 0)
            {
                int8_t choice = randint(0, col_num);
                choice = possible_col[choice];
                play_at(choice);
                reward = get_reward();
                if (reward > -10)
                    return { reward, 0 };
            }
        }
    }

    void print() const
    {
        cerr << "----------\n";
        for (int8_t i = 0; i < M; i++)
        {
            for (int8_t j = 0; j < N; j++)
                cerr << int(_board[i][j]) << ", ";
            cerr << "\n";
        }
        for (auto item : top)
        {
            cerr << int(item) << ", ";
        }
        cerr << "\n----------\n";
    }

    ~Board()
    {
        if (_board)
        {
            for (int8_t i = 0; i < M; i++)
            {
                if (_board[i])
                {
                    delete[] _board[i];
                    _board[i] = nullptr;
                }
            }
            delete[] _board;
            _board = nullptr;
        }
    }

};

#endif //STRATEGY_BOARD_H
