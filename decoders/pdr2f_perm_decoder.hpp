  /******************************************************************************
 * pdr2f_perm_decoder.hpp: Interface for PDR2F_Perm_Decoder class.
 *
 * Decodificador da familia PERMUTACAO (Londe et al., 2023) para o Problema da
 * Dominacao Romana 2-Forte (PDR2F).
 *
 * O cromossomo nao guarda rotulos: a ordenacao das chaves define uma ordem de
 * prioridade dos vertices, e uma regra fixa constroi a solucao percorrendo
 * essa ordem. O BRKGA, portanto, aprende a ORDEM, e nao os rotulos.
 *****************************************************************************/

#ifndef PDR2F_PERM_DECODER_HPP_
#define PDR2F_PERM_DECODER_HPP_

#include "pdr2f/pdr2f_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"

#include <utility>
#include <vector>

/**
 * \brief Interface for PDR2F_Perm_Decoder class.
 *
 * Construcao (proposta P2):
 *
 * \verbatim
 * para cada vertice v na ordem dada pelo cromossomo:
 *     se v ja' esta' coberto: segue para o proximo
 *     ganho = |N[v] \ Cobertos|          (v esta' descoberto, logo ganho >= 1)
 *     f(v)  = min(3, ganho)
 *     Cobertos = Cobertos + N[v]
 * \endverbatim
 *
 * "Coberto" significa protegido: o vertice foi rotulado ou tem um vizinho com
 * rotulo >= 2. A solucao gerada e' sempre viavel e nao precisa de reparo.
 */
class PDR2F_Perm_Decoder {
public:
    /** \brief Default Constructor.
     * \param instance PDR2F instance.
     */
    PDR2F_Perm_Decoder(const PDR2F_Instance& instance);

    /** \brief Given a chromossome, builds a 2-strong Roman dominating
     *         function.
     *
     * \param chromosome A vector of doubles represent a problem solution.
     * \param rewrite Indicates if the chromosome must be rewritten. Not used
     *                in this decoder, but kept due to API requirements.
     * \return the weight (sum of the labels) of the solution.
     */
    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    /** \brief Rebuilds the labels of a given chromosome.
     *
     * Mesma construcao do decode(), mas devolvendo a rotulacao. Usado para
     * mostrar e verificar a melhor solucao ao final da otimizacao.
     */
    std::vector<unsigned> solution(const BRKGA::Chromosome& chromosome) const;

public:
    /// A reference to a PDR2F instance.
    const PDR2F_Instance& instance;

protected:
    /// Builds the solution into `f`, returning its weight.
    unsigned build(const BRKGA::Chromosome& chromosome,
                   std::vector<unsigned>& f,
                   std::vector<char>& covered,
                   std::vector<std::pair<double, unsigned>>& order) const;
};

#endif // PDR2F_PERM_DECODER_HPP_