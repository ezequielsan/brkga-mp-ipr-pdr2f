/******************************************************************************
 * pdr2f_instance.cpp: Implementation for PDR2F_Instance class.
 *****************************************************************************/

#include "pdr2f/pdr2f_instance.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <utility>

using namespace std;

//-----------------------------[ Constructor ]--------------------------------//

PDR2F_Instance::PDR2F_Instance(const std::string& filename):
    num_nodes(0),
    adj()
{
    ifstream file(filename, ios::in);
    if(!file)
        throw runtime_error("Cannot open instance file");

    vector<pair<unsigned, unsigned>> edges;
    unsigned u, v;
    while(file >> u >> v) {
        edges.emplace_back(u, v);
        num_nodes = max(num_nodes, max(u, v) + 1);
    }

    if(!file.eof())
        throw runtime_error("Error reading the instance file");

    if(edges.empty())
        throw runtime_error("The instance file has no edges");

    adj.resize(num_nodes);
    for(const auto& [a, b] : edges) {
        if(a != b) {
            adj[a].push_back(b);
            adj[b].push_back(a);
        }
    }

    // Sort and remove repeated edges.
    for(auto& ns : adj) {
        sort(ns.begin(), ns.end());
        ns.erase(unique(ns.begin(), ns.end()), ns.end());
    }
}

//-------------------------------[ Neighbors ]--------------------------------//

const std::vector<unsigned>& PDR2F_Instance::neighbors(unsigned v) const {
    return adj[v];
}