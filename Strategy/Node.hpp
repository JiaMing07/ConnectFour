//
// Created by Colin on 2021/3/29.
//

#ifndef STRATEGY_NODE_H
#define STRATEGY_NODE_H

#include <vector>
#include "Board.hpp"

using namespace std;

const int8_t INIT_END_VALUE = -127;
short max_layer = 0;
int node_num = 0;

double activate[] = { 10,
                      9.64197015, 9.40783903, 9.0376493 , 8.47792998, 7.68637532,
                      6.66666667, 5.5       , 4.33333333, 3.31362468, 2.52207002,
                      1.9623507 , 1.59216097, 1.35802985, 1.21411306, 1.12719486,
                      1.07525846, 1.04442264, 1.02618418, 1.01542093, 1.00907754,
                      1.00534195, 1.00314309, 1.00184914, 1.00108782, 1.00063993,
                      1.00037644, 1.00022144, 1.00013026, 1.00007662, 1.00004507,
                      1.00002651, 1.0000156 , 1.00000917, 1.0000054 , 1.00000317,
                      1.00000187, 1.0000011 , 1.00000065, 1.00000038, 1.00000022,
                      1.00000013, 1.00000008, 1.00000005, 1.00000003, 1.00000002,
                      1.00000001, 1.00000001, 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.        , 1.        ,
                      1.        , 1.        , 1.        , 1.};

struct Children
{
    int children[12] = { 0 };
    int8_t size = 0;

    void emplace_back(const int _v)
    {
        children[size++] = _v;
    }

    void clear()
    {
        size = 0;
    }
};

class Node
{
    Children children;
    int8_t last_step_y;
    bool is_min;
    int visited = 0;
    double value = 0;
    int8_t end_value = INIT_END_VALUE;
    short layer = 0;

    Node(): Node(0, false, 0) {}

    explicit Node(const int8_t& _y, const bool _min, const short _layer):
            last_step_y(_y), is_min(_min), layer(_layer)
    {
        //max_layer = max(max_layer, layer);
        node_num++;
    }

public:
    static const int MAX_NODE_NUM = 3000000;
    static Node Pool[MAX_NODE_NUM];
    static int pool_top;

    static int new_node(const int8_t& _y, const bool _min, const short _layer)
    {
        Pool[pool_top].last_step_y = _y;
        Pool[pool_top].is_min = _min;
        Pool[pool_top].layer = _layer;
        max_layer = max(max_layer, _layer);
        Pool[pool_top].children.clear();
        Pool[pool_top].visited = 0;
        Pool[pool_top].value = 0;
        Pool[pool_top].end_value = INIT_END_VALUE;
        return pool_top++;
    }

    [[nodiscard]] bool is_leaf() const
    {
        return children.size == 0;
    }

    [[nodiscard]] int get_visited() const
    {
        return visited;
    }

    [[nodiscard]] double get_value() const
    {
        return value;
    }

    [[nodiscard]] bool is_min_node() const
    {
        return is_min;
    }

    [[nodiscard]] bool is_visited() const
    {
        return visited;
    }

    [[nodiscard]] int8_t get_end_value() const
    {
        return end_value;
    }

    void visit()
    {
        visited++;
    }

    void add_value(const double& v)
    {
        if (!is_min) // the parent is a min_node
            value -= v;
        else
            value += v;
    }

    Children& get_children()
    {
        return children;
    }

    [[nodiscard]] int last_step() const
    {
        return last_step_y;
    }

    void expand_children(const Board& board)
    {
        int8_t children_size = board.enumerate_children();
        for (int i = 0; i < children_size; i++)
        {
            children.emplace_back(new_node(enumed_children[i], !is_min, layer + 1));
        }
    }

    [[nodiscard]] double rollout(Board& board)
    {
        if (end_value != INIT_END_VALUE) // reach the end and know the result,
            return end_value; // no need to rollout
        else
        {
            auto res = board.rollout();
            if (res.second) // reach the end
                end_value = res.first; // save the result
            return res.first;
        }
    }

//    ~Node()
//    {
//        while (!children.empty())
//        {
//            delete children.back();
//            children.pop_back();
//        }
//    }
};

int Node::pool_top = 0;
const int Node::MAX_NODE_NUM;
Node Node::Pool[MAX_NODE_NUM];

#endif //STRATEGY_NODE_H
