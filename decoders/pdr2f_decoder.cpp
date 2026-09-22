/******************************************************************************
 * pdr2f_decoder.cpp: Implementation for PDR2F_Decoder class.
 *****************************************************************************/

#include "decoders/pdr2f_decoder.hpp"

#include <algorithm>
#include <numeric>

using namespace std;
using namespace BRKGA;

//-----------------------------[ Constructor ]--------------------------------//

PDR2F_Decoder::PDR2F_Decoder(const PDR2F_Instance& _instance):
    instance(_instance)
{}

//--------------------------------[ Label ]-----------------------------------//

unsigned PDR2F_Decoder::label(double key) {
    return min(static_cast<unsigned>(key * 4.0), 3u);
}

//-------------------------------[ Decode ]-----------------------------------//

BRKGA::fitness_t PDR2F_Decoder::decode(Chromosome& chromosome,
                                       bool /* not-used */) {
    // Local vector: each call (thread) has its own labels (thread-safe).
    vector<unsigned> f(instance.num_nodes);
    for(unsigned v = 0; v < instance.num_nodes; ++v)
        f[v] = label(chromosome[v]);

    fixInstance(f);

    return accumulate(f.begin(), f.end(), 0.0);
}

//-----------------------------[ Fix instance ]-------------------------------//

void PDR2F_Decoder::fixInstance(vector<unsigned>& f) const {
    const auto& adj = instance.adj;
    const unsigned n = instance.num_nodes;

    // num_ap[v]: number of protectors (neighbors with label >= 2) of each
    // vertex v with label 0. It plays the role of |AP(v)| in Torres (2026).
    vector<unsigned> num_ap(n, 0);
    for(unsigned v = 0; v < n; ++v)
        if(f[v] == 0)
            for(const auto u : adj[v])
                if(f[u] >= 2)
                    ++num_ap[v];

    for(unsigned v = 0; v < n; ++v) {
        if(f[v] == 0) {

            // returns true if any neighbor of v protects it;
            // otherwise, returns false.
            const bool protected_v = any_of(adj[v].begin(), adj[v].end(),
                                            [&](unsigned u) { return f[u] >= 2; });
            if(!protected_v) {
                if(adj[v].size() > 3) {
                    f[v] = 3;
                    // v now protects its neighbors with label 0.
                    for(const auto u : adj[v])
                        if(f[u] == 0)
                            ++num_ap[u];
                }
                else {
                    f[v] = 1;
                }
            }
        }
        else
        if(f[v] == 2) {
            unsigned dependents = 0;
            for(const auto u : adj[v])
                if(f[u] == 0 && num_ap[u] == 1)
                    ++dependents;

            if(dependents > 1)
                f[v] = 3;
        }
    }
}