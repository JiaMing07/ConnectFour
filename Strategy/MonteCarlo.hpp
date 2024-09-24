#ifndef STRATEGY_MONTECARLO_H
#define STRATEGY_MONTECARLO_H

#include <iostream>
#include <cmath>
#include <stack>
#include "Node.hpp"
#include "utils.hpp"

using namespace std;

#ifdef OUTPUT
const double max_time_per_step = 2550;
#else
const double max_time_per_step = 2550;
#endif


struct UCB1
{
    /**
     * @brief w/n + c * sqrt( lnN / n )
     * @param _node pointer of the child node
     * @param N node->parent->visited
     * @param c empirical constant (increase it to explore more)
     * @return ~inf (~1e9) or limited value
     */
    double operator()(Node* _node, const int N, double c = sqrt(2))
    {
        double w = _node->get_value();
        int n = _node->get_visited();
        if (n == 0)
            return 1e9 + randint(0, 20);
        else
            return w / n + c * sqrt(log(N) / n);
    }
};

int path_stack[100];
int top_of_stack = 0;

template <class Eval = UCB1>
class MonteCarlo
{
    int root = 0;   // root of MonteCarlo Tree
    const Board& init_board;

    int select_best_child(const int _current)
    {
        const int N = Node::Pool[_current].get_visited();
        Children& children = Node::Pool[_current].get_children();
        int best_child = 0;
        double best_value = -1e9;
        // select the child with maximum value
        for (int i = 0; i < children.size; i++)
        {
            int child_index = children.children[i];
            double tmp = Eval()(&Node::Pool[child_index], N);
            if (tmp > best_value)
            {
                best_value = tmp;
                best_child = child_index;
            }
        }
        return best_child;
    }

    [[nodiscard]] int select_child_with_max_visited(const int _current) const
    {
        Children& children = Node::Pool[_current].get_children();
        int choice = 0;
        double max_visited = -0x7fffffff;
        for (int i = 0; i < children.size; i++)
        {
            int child_index = children.children[i];
            double tmp = Node::Pool[child_index].get_visited();
            if (tmp > max_visited)
            {
                max_visited = tmp;
                choice = child_index;
            }
        }
        return choice;
    }

public:
    /**
     * @brief init MonteCarlo Tree Search from a certain state
     * @param _board   init state of board
     */
    MonteCarlo(const Board& _board, const bool _min): init_board(_board)
    {
        root = Node::new_node(-1, _min, 0);
        Node::Pool[root].visit();
    }

    /**
     * @brief perform MCTS algorithm
     * @return where to play
     */
    int play()
    {
        timespec start;
        record_time(start);
        int cc = 0;
#ifdef OUTPUT_BOARD
        cerr << "--------init_board---------\n";
        init_board.print();
        cerr << "---------------------------\n";
#endif
        while (cc % 5000 || delta_time_from(start) < max_time_per_step)
        {
            cc++;
            int current = root; // start from root
            top_of_stack = 0;
            Board board(init_board);
            // Selection
            while (!Node::Pool[current].is_leaf()) // current node is not a leaf node
            {
                // select the best child node and move to it
                path_stack[top_of_stack++] = current;
                current = select_best_child(current);
                board.play_at(Node::Pool[current].last_step()); // update board with node
            } // until move to a leaf node
            // Expansion
            if (Node::Pool[current].is_visited() && Node::Pool[current].get_end_value() == INIT_END_VALUE)
            {
                // expand new (leaf, children) nodes from current
                Node::Pool[current].expand_children(board);
                // current moves to the 1st (or random) children
                path_stack[top_of_stack++] = current;
//                int8_t children_num = Node::Pool[current].get_children().size;
//                int8_t choice = randint(0, children_num);
//                current = Node::Pool[current].get_children().children[choice];
                current = Node::Pool[current].get_children().children[0];
                board.play_at(Node::Pool[current].last_step());
            }
            // assert(current->get_end_value() > INIT_END_VALUE ||
            //      current->get_visited() == 0 && current->get_value() == 0);
            // Simulation (rollout)
            double rollout_result = Node::Pool[current].rollout(board);
            Node::Pool[current].add_value(rollout_result);
            Node::Pool[current].visit();
            // Backpropagation
            while (top_of_stack)
            {
                int parent = path_stack[--top_of_stack];
                Node::Pool[parent].add_value(rollout_result);
                Node::Pool[parent].visit();
            }
        }
        // make the choice among direct children of root
        int best_child = select_child_with_max_visited(root);
#ifdef OUTPUT
        cerr << "cc: " << cc << "\n";
        cerr << "max_layer: " << max_layer << "\n";
        cerr << "node_num: " << Node::pool_top << "\n";
        cerr << "Children info: \n";
#endif
#ifdef OUTPUT_CHILDREN
        int index = 0;
        auto& children = Node::Pool[root].get_children();
        for (int i = 0; i < children.size; i++)
        {
            cerr << "\t" << index++ << ": " << Node::Pool[children.children[i]].get_visited()
            << ", " << Node::Pool[children.children[i]].get_value() << ' ' << Node::Pool[children.children[i]].is_min_node() << " " << Node::Pool[children.children[i]].last_step() <<"\n";
        }
        cerr << "---- > children of best_child\n";
        auto& children2 = Node::Pool[best_child].get_children();
        for (int i = 0; i < children2.size; i++)
        {
            cerr << "\t\t" << Node::Pool[children2.children[i]].last_step() << ": " << Node::Pool[children2.children[i]].get_visited()
                 << ", " << Node::Pool[children2.children[i]].get_value() << ' ' << Node::Pool[children2.children[i]].is_min_node()
                 << " " << Node::Pool[children.children[i]].last_step() << "\n";
        }
#endif
        Node::pool_top = 0;
        max_layer = 0;
        return Node::Pool[best_child].last_step();
    }

//    ~MonteCarlo()
//    {
//        if (root)
//        {
//            delete root;
//            root = nullptr;
//        }
//    }
};

#endif //STRATEGY_MONTECARLO_H
