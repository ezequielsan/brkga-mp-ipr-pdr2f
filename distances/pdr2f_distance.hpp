/******************************************************************************
 * pdr2f_distance.hpp: distance functor for the PDR2F chromosomes.
 *
 * O IPR precisa saber quando duas chaves representam solucoes diferentes. A
 * classe BRKGA::HammingDistance da API binariza as chaves com limiar 0.5 e,
 * por isso, nao distingue os 4 rotulos do decodificador de Torres (2026).
 * Esta classe compara os rotulos, e nao as chaves.
 *
 * Modelada a partir do exemplo TernaryHammingDistance do guia da API.
 *****************************************************************************/

#ifndef PDR2F_DISTANCE_HPP_
#define PDR2F_DISTANCE_HPP_

#include "decoders/pdr2f_decoder.hpp"

// NOTE: BRKGA::DistanceFunctionBase e' declarada no cabecalho principal da
// API. Este arquivo so' deve ser incluido pelo mesmo (unico) .cpp que inclui
// 'brkga_mp_ipr.hpp'; caso contrario, compile com -DBRKGA_MULTIPLE_INCLUSIONS.
#include "brkga_mp_ipr.hpp"

#include <cstddef>
#include <stdexcept>

/**
 * \brief Hamming distance over the 4 labels of the PDR2F decoder.
 *
 * A distancia entre dois cromossomos e' o numero de vertices cujos rotulos
 * diferem (um valor absoluto, entre 0 e o numero de vertices).
 */
class PDR2F_Distance: public BRKGA::DistanceFunctionBase {
public:
    /// Default constructor.
    PDR2F_Distance() = default;

    /// Default destructor.
    virtual ~PDR2F_Distance() = default;

    /** \brief Number of vertices with different labels.
     * \param vector1 first chromosome.
     * \param vector2 second chromosome.
     */
    virtual double distance(const BRKGA::Chromosome& vector1,
                            const BRKGA::Chromosome& vector2) override {
        if(vector1.size() != vector2.size())
            throw std::runtime_error("The size of the vector must "
                                     "be the same!");

        unsigned dist = 0;
        for(std::size_t i = 0; i < vector1.size(); ++i)
            if(PDR2F_Decoder::label(vector1[i]) !=
               PDR2F_Decoder::label(vector2[i]))
                ++dist;

        return double(dist);
    }

    /** \brief Returns true if changing `key1` by `key2` changes the label.
     * \param key1 the first key.
     * \param key2 the second key.
     */
    virtual bool affectSolution(const BRKGA::Chromosome::value_type key1,
                                const BRKGA::Chromosome::value_type key2)
                                override {
        return PDR2F_Decoder::label(key1) != PDR2F_Decoder::label(key2);
    }

    /** \brief Returns true if changing a block of keys changes some label.
     * \param v1_begin begin of the first block of keys.
     * \param v2_begin begin of the second block of keys.
     * \param block_size number of keys to be considered.
     */
    virtual bool affectSolution(BRKGA::Chromosome::const_iterator v1_begin,
                                BRKGA::Chromosome::const_iterator v2_begin,
                                const std::size_t block_size) override {
        for(std::size_t i = 0; i < block_size; ++i, ++v1_begin, ++v2_begin)
            if(PDR2F_Decoder::label(*v1_begin) != PDR2F_Decoder::label(*v2_begin))
                return true;

        return false;
    }
};

#endif // PDR2F_DISTANCE_HPP_