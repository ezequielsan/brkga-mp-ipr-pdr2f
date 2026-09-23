/******************************************************************************
 * greedy_pdr2f.cpp: Implementation of the greedy heuristic for the PDR2F.
 *****************************************************************************/

#include "heuristics/greedy_pdr2f.hpp"

#include <algorithm>
#include <numeric>

using namespace std;

//-------------------------------[ Greedy ]-----------------------------------//

pair<BRKGA::fitness_t, vector<unsigned>>
greedy_pdr2f(const PDR2F_Instance& instance, mt19937& rng) {
    const unsigned n = instance.num_nodes;

    vector<unsigned> labels(n, 0);
    vector<char> covered(n, 0);
    unsigned num_covered = 0;

    // Ordem de avaliacao embaralhada: e' o que diversifica as solucoes.
    vector<unsigned> order(n);
    iota(order.begin(), order.end(), 0);
    shuffle(order.begin(), order.end(), rng);

    while(num_covered < n) {
        unsigned best = n;
        unsigned best_gain = 0;

        // Ganho de cada vertice ainda nao coberto: quantos vertices de N[v]
        // (ele proprio e os vizinhos) ainda estao descobertos.
        for(const auto v : order) {
            if(covered[v])
                continue;

            unsigned gain = 1;      // o proprio v, que esta' descoberto.
            for(const auto u : instance.adj[v])
                if(!covered[u])
                    ++gain;

            // Empate: fica o primeiro na ordem embaralhada.
            if(gain > best_gain) {
                best_gain = gain;
                best = v;
            }
        }

        labels[best] = min(3u, best_gain);

        // Cobre N[best].
        if(!covered[best]) {
            covered[best] = 1;
            ++num_covered;
        }
        for(const auto u : instance.adj[best]) {
            if(!covered[u]) {
                covered[u] = 1;
                ++num_covered;
            }
        }
    }

    const auto cost =
        BRKGA::fitness_t(accumulate(labels.begin(), labels.end(), 0u));

    return {cost, labels};
}