/******************************************************************************
 * pdr2f_instance.hpp: Interface for PDR2F_Instance class.
 *
 * Estrutura de dados para instancias do Problema da Dominacao Romana 2-Forte
 * (PDR2F). Mesmo molde do TSP_Instance do exemplo da API BRKGA-MP-IPR.
 *****************************************************************************/

#ifndef PDR2F_INSTANCE_HPP_
#define PDR2F_INSTANCE_HPP_

#include <string>
#include <vector>

/**
 * \brief Interface for PDR2F_Instance class.
 *
 * Represents an instance of the 2-Strong Roman Domination Problem. The
 * constructor loads an undirected graph from an edge list, one edge per line:
 *
 * \verbatim
 * u v
 * \endverbatim
 *
 * The vertices are numbered 0, 1, ..., n-1, without holes. For example:
 *
 * \verbatim
 * 0 1
 * 1 2
 * 2 0
 * \endverbatim
 */
class PDR2F_Instance {
public:
    /// Default Constructor.
    PDR2F_Instance(const std::string& filename);

    /// Return the neighbors of vertex v.
    const std::vector<unsigned>& neighbors(unsigned v) const;

public:
    /// Number of nodes.
    unsigned num_nodes;

    /// Adjacency lists (sorted, without repetitions or self-loops).
    std::vector<std::vector<unsigned>> adj;
};

#endif // PDR2F_INSTANCE_HPP_