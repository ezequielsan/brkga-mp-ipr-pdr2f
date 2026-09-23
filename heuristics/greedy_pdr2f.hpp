/******************************************************************************
 * greedy_pdr2f.hpp: Interface for the greedy heuristic for the PDR2F.
 *
 * Heuristica gulosa de Djukanovic et al. (2025), na forma usada por Torres
 * (2026, Algoritmo 3) para gerar parte da populacao inicial. Papel equivalente
 * ao greedy_tour.hpp do exemplo do TSP.
 *****************************************************************************/

#ifndef GREEDY_PDR2F_HPP_
#define GREEDY_PDR2F_HPP_

#include "pdr2f/pdr2f_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"

#include <random>
#include <utility>
#include <vector>

/**
 * \brief Builds a greedy 2-strong Roman dominating function.
 *
 * Enquanto houver vertice nao coberto:
 *   1. para cada vertice v nao coberto, Ganho(v) = |N[v] - Cobertos|;
 *   2. escolhe o v* de maior ganho (empate: o primeiro na ordem embaralhada);
 *   3. f(v*) = min(3, Ganho(v*));
 *   4. Cobertos = Cobertos + N[v*].
 *
 * A solucao gerada e' sempre viavel: todo vertice de rotulo 0 foi coberto por
 * um vizinho de rotulo >= 2, e um vertice de rotulo 2 cobriu exatamente um
 * vizinho novo.
 *
 * A ordem dos vertices e' embaralhada com o gerador `rng`; assim, chamadas
 * sucessivas podem produzir solucoes diferentes.
 *
 * \param instance a PDR2F instance.
 * \param rng random number generator (kept outside to allow different runs).
 * \return a pair with the solution cost and the labels.
 */
std::pair<BRKGA::fitness_t, std::vector<unsigned>>
greedy_pdr2f(const PDR2F_Instance& instance, std::mt19937& rng);

#endif // GREEDY_PDR2F_HPP_