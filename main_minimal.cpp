/******************************************************************************
 * main_minimal.cpp: minimal code for the 2-Strong Roman Domination Problem
 *                   (PDR2F) using the BRKGA-MP-IPR framework.
 *
 * Adaptado de examples/tsp/src/single_obj/main_minimal.cpp.
 *****************************************************************************/

#include "pdr2f/pdr2f_instance.hpp"
#include "decoders/pdr2f_decoder.hpp"
#include "brkga_mp_ipr.hpp"
#include "distances/pdr2f_distance.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// Vies de Torres (2026): com 2 pais (1 elite + 1 nao elite), o filho herda a
// chave do pai elite com probabilidade BIAS. Usado com 'bias_type CUSTOM'.
const double BIAS = 0.7106;

//-------------------------------[ Main ]------------------------------------//

int main(int argc, char* argv[]) {
    if(argc < 5) {
        cerr
        << "Usage: " << argv[0]
        << " <seed> <config-file> <maximum-running-time>"
        << " <pdr2f-instance-file> [<num-generations>]"
        << endl;
        return 1;
    }

    try {
        ////////////////////////////////////////
        // Read command-line arguments and the instance
        ////////////////////////////////////////

        const unsigned seed = stoi(argv[1]);
        const string config_file = argv[2];
        const string instance_file = argv[4];
        const unsigned num_threads = 1;

        // Numero de geracoes (opcional). 0 = sem limite de geracoes, ou seja,
        // a parada fica so' por tempo. Torres (2026) usou 100 geracoes.
        const unsigned num_generations = (argc > 5)? stoi(argv[5]) : 0;

        cout << "Reading data..." << endl;
        auto instance = PDR2F_Instance(instance_file);

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] =
            BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {stoi(argv[3])};

        // NOTE: com 'pr_distance_function_type CUSTOM' no config, o IPR so'
        // funciona se a distancia for fornecida aqui, ANTES de construir o
        // algoritmo (o construtor copia os parametros).
        using BRKGA::PathRelinking::DistanceFunctionType;
        if(brkga_params.pr_distance_function_type == DistanceFunctionType::CUSTOM) {
            brkga_params.pr_distance_function =
                shared_ptr<BRKGA::DistanceFunctionBase> {new PDR2F_Distance};
        }

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        PDR2F_Decoder decoder(instance);

        // Chromosome size is the number of nodes: each key gives the label
        // of one vertex.
        BRKGA::BRKGA_MP_IPR<PDR2F_Decoder> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            instance.num_nodes, brkga_params, num_threads
        );

        // NOTE: com 'bias_type CUSTOM' no config, esta chamada e' obrigatoria.
        // Sem ela, a API usa em silencio o vies CONSTANT (0.5).
        if(brkga_params.bias_type == BRKGA::BiasFunctionType::CUSTOM) {
            algorithm.setBiasCustomFunction(
                [](const unsigned r) { return r == 1? BIAS : 1.0 - BIAS; }
            );
        }

        // Parada por numero de geracoes, como em Torres (2026). O tempo
        // maximo continua valendo como limite de seguranca.
        if(num_generations > 0) {
            algorithm.setStoppingCriteria(
                [&](const BRKGA::AlgorithmStatus& status) {
                    return status.current_iteration == num_generations;
                }
            );
        }

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time;
        if(num_generations > 0)
            cout << " or " << num_generations << " generations";
        cout << "..." << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        ////////////////////////////////////////
        // Extracting the best solution
        ////////////////////////////////////////

        // Decodes the best chromosome again (keys -> labels -> repair).
        vector<unsigned> labels(instance.num_nodes);
        for(unsigned v = 0; v < instance.num_nodes; ++v)
            labels[v] = PDR2F_Decoder::label(final_status.best_chromosome[v]);
        decoder.fixInstance(labels);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;

        for(unsigned label = 0; label < 4; ++label) {
            cout << "% Label " << label << ":";
            for(unsigned v = 0; v < instance.num_nodes; ++v)
                if(labels[v] == label)
                    cout << " " << v;
            cout << endl;
        }
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
        return 1;
    }
    return 0;
}