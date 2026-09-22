/******************************************************************************
 * pdr2f_decoder.hpp: Interface for PDR2F_Decoder class.
 *
 * Decodificador do Problema da Dominacao Romana 2-Forte (PDR2F), segundo
 * Torres (2026). Mesmo molde do TSP_Decoder do exemplo da
 * API BRKGA-MP-IPR.
 *****************************************************************************/

#ifndef PDR2F_DECODER_HPP_
#define PDR2F_DECODER_HPP_

#include "pdr2f/pdr2f_instance.hpp"
#include "brkga_mp_ipr/fitness_type.hpp"
#include "brkga_mp_ipr/chromosome.hpp"

#include <vector>

/**
 * \brief Interface for PDR2F_Decoder class.
 *
 * Decodificador de limiares de Torres (2026). Cada chave vira um rotulo pela
 * faixa do intervalo [0, 1) em que cai:
 *
 * \verbatim
 * [0.00, 0.25) -> 0    [0.25, 0.50) -> 1
 * [0.50, 0.75) -> 2    [0.75, 1.00) -> 3
 * \endverbatim
 *
 * Em seguida a rotulacao e' reparada (fixInstance) para se tornar uma funcao
 * de dominacao romana 2-forte, e o fitness e' a soma dos rotulos.
 */
class PDR2F_Decoder {
public:
    /** \brief Default Constructor.
     * \param instance PDR2F instance.
     */
    PDR2F_Decoder(const PDR2F_Instance& instance);

    /** \brief Given a chromossome, builds a 2-strong Roman dominating
     *         function.
     *
     * \param chromosome A vector of doubles represent a problem solution.
     * \param rewrite Indicates if the chromosome must be rewritten. Not used
     *                this decoder, but keep due to API requirements.
     * \return the weight (sum of the labels) of the solution.
     */
    BRKGA::fitness_t decode(BRKGA::Chromosome& chromosome, bool rewrite);

    /// Maps a key to its label (0, 1, 2 or 3).
    static unsigned label(double key);

    /** \brief Repair of Torres (2026, Algoritmo 5), in a single pass:
     *  1. vertex with label 0 and no neighbor with label >= 2:
     *     becomes 3 if its degree is > 3, otherwise becomes 1;
     *  2. vertex with label 2 that is the only protector of more than one
     *     neighbor with label 0: becomes 3.
     */
    void fixInstance(std::vector<unsigned>& f) const;

public:
    /// A reference to a PDR2F instance.
    const PDR2F_Instance& instance;
};

#endif // PDR2F_DECODER_HPP_