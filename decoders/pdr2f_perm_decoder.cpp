/******************************************************************************
 * pdr2f_perm_decoder.cpp: Implementation for PDR2F_Perm_Decoder class.
 *****************************************************************************/

#include "decoders/pdr2f_perm_decoder.hpp"

#include <algorithm>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

PDR2F_Perm_Decoder::PDR2F_Perm_Decoder(const PDR2F_Instance& _instance):
    instance(_instance)
{}

//--------------------------------[ Build ]-----------------------------------//

unsigned PDR2F_Perm_Decoder::build(const Chromosome& chromosome,
                                   vector<unsigned>& f,
                                   vector<char>& covered,
                                   vector<pair<double, unsigned>>& order) const {
    const auto& adj = instance.adj;
    const unsigned n = instance.num_nodes;

    // 1. A ordenacao das chaves da' a ordem de prioridade dos vertices.
    for(unsigned v = 0; v < n; ++v)
        order[v] = make_pair(chromosome[v], v);

    sort(order.begin(), order.end());

    // 2. Estado inicial: ninguem rotulado, ninguem coberto.
    fill(f.begin(), f.end(), 0u);
    fill(covered.begin(), covered.end(), char(0));

    unsigned weight = 0;

    // 3. Varredura na ordem do cromossomo.
    for(const auto& [key, v] : order) {
        if(covered[v])
            continue;

        // Mesma regra de rotulação da heurística de Djukanović. A diferença 
        // é qual vértice é escolhido a cada passo: lá era o de maior ganho, 
        // aqui é o próximo da ordem do cromossomo.
        
        // Ganho: o proprio v (descoberto) mais os vizinhos ainda descobertos.
        unsigned gain = 1;
        for(const auto u : adj[v])
            if(!covered[u])
                ++gain;

        f[v] = min(3u, gain);
        weight += f[v];

        // v e sua vizinhanca passam a estar cobertos.
        covered[v] = 1;
        for(const auto u : adj[v])
            covered[u] = 1;
    }

    return weight;
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t PDR2F_Perm_Decoder::decode(Chromosome& chromosome,
                                            bool /* not-used */) {
    // Estruturas locais: cada chamada (thread) tem as suas (thread-safe).
    const unsigned n = instance.num_nodes;
    vector<unsigned> f(n);
    vector<char> covered(n);
    vector<pair<double, unsigned>> order(n);

    return BRKGA::fitness_t(build(chromosome, f, covered, order));
}

//------------------------------[ Solution ]----------------------------------//

vector<unsigned> PDR2F_Perm_Decoder::solution(const Chromosome& chromosome) const {
    const unsigned n = instance.num_nodes;
    vector<unsigned> f(n);
    vector<char> covered(n);
    vector<pair<double, unsigned>> order(n);

    build(chromosome, f, covered, order);

    return f;
}