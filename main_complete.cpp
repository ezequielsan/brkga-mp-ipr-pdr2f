/******************************************************************************
 * main_complete.cpp: comprehensive code for the 2-Strong Roman Domination
 *                    Problem (PDR2F) using the BRKGA-MP-IPR framework.
 *
 * Adaptado de examples/tsp/src/single_obj/main_complete.cpp. A leitura dos
 * argumentos foi reescrita sem a dependencia docopt.
 *
 * Usage:
 *   main_complete --config <arq> --seed <n> --stop_rule <G|I> --stop_arg <n>
 *                 --maxtime <s> --instance <arq>
 *                 [--threads <n>] [--warmstart <n>] [--no_evolution]
 *                 [--no_verify] [--quiet]
 *****************************************************************************/

#include "pdr2f/pdr2f_instance.hpp"
#include "decoders/pdr2f_decoder.hpp"
#include "heuristics/greedy_pdr2f.hpp"
#include "brkga_mp_ipr.hpp"
#include "distances/pdr2f_distance.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace std::chrono;

//-------------------------[ Some control constants ]-------------------------//

// Vies de Torres (2026): com 2 pais (1 elite + 1 nao elite), o filho herda a
// chave do pai elite com probabilidade BIAS. Usado com 'bias_type CUSTOM'.
const double BIAS = 0.7106;

// Controls stop criteria.
enum class StopRule {
    GENERATIONS = 'G',
    IMPROVEMENT = 'I',
    UNKNOWN = 'U',
};

//-------------------------[ Simple logging function ]------------------------//

void log(const string& message) {
    auto start_time = system_clock::to_time_t(system_clock::now());
    string ss(ctime(&start_time));
    ss.pop_back();  // Workaround to skip unwanted end-of-line.
    cout << "\n[" << ss << "] " << message << endl;
}

//-----------------------[ Feasibility verification ]-------------------------//

/**
 * \brief Checks the local characterization of a 2-strong Roman dominating
 * function (Torres, 2026, Secao 2.2.1):
 *   1. f(v) = 0  ==> v tem ao menos um vizinho com rotulo >= 2;
 *   2. f(v) = 2  ==> v e' o unico protetor de, no maximo, um vizinho.
 */
bool is_feasible(const PDR2F_Instance& instance,
                 const vector<unsigned>& f) {
    const unsigned n = instance.num_nodes;

    // num_ap[v]: numero de protetores (vizinhos com rotulo >= 2) de v.
    vector<unsigned> num_ap(n, 0);
    for(unsigned v = 0; v < n; ++v)
        for(const auto u : instance.adj[v])
            if(f[u] >= 2)
                ++num_ap[v];

    for(unsigned v = 0; v < n; ++v) {
        if(f[v] > 3)
            return false;

        if(f[v] == 0 && num_ap[v] == 0)
            return false;

        if(f[v] == 2) {
            unsigned dependents = 0;
            for(const auto u : instance.adj[v])
                if(f[u] == 0 && num_ap[u] == 1)
                    ++dependents;

            if(dependents > 1)
                return false;
        }
    }

    return true;
}

//---------------------------[ Argument parsing ]-----------------------------//

/// Reads "--key value" and "--flag" pairs into a map.
map<string, string> parse_arguments(int argc, char* argv[]) {
    map<string, string> args;

    for(int i = 1; i < argc; ++i) {
        string token(argv[i]);
        if(token.rfind("--", 0) != 0)
            throw logic_error("Unexpected argument: '" + token + "'");

        if(i + 1 < argc && string(argv[i + 1]).rfind("--", 0) != 0)
            args[token] = argv[++i];
        else
            args[token] = "true";       // flags without value.
    }

    return args;
}

/// Returns the value of a required argument.
string get_required(const map<string, string>& args, const string& key) {
    const auto it = args.find(key);
    if(it == args.end())
        throw logic_error("Missing required argument: '" + key + "'");
    return it->second;
}

/// Returns the value of an optional argument, or the default one.
string get_optional(const map<string, string>& args, const string& key,
                const string& default_value) {
    const auto it = args.find(key);
    return (it == args.end())? default_value : it->second;
}

//--------------------------------[ Main ]------------------------------------//

int main(int argc, char* argv[]) {
    const string usage =
R"(
Usage:
  main_complete --config <config_file> --seed <seed> --stop_rule <G|I>
                --stop_arg <arg> --maxtime <max_time> --instance <file>
                [--threads <n>] [--warmstart <n>]
                [--no_evolution] [--no_verify] [--quiet]

Options:
  --config <arq>     Arquivo texto com os parametros do BRKGA-MP-IPR.
  --seed <n>         Semente do gerador de numeros aleatorios.
  --stop_rule <arg>  Regra de parada, combinada em OU com --maxtime:
                     - (G)enerations: numero de geracoes;
                     - (I)mprovement: geracoes sem melhora (stall).
  --stop_arg <n>     Valor para a regra de parada.
  --maxtime <s>      Tempo maximo, em segundos.
  --instance <arq>   Arquivo da instancia (lista de arestas).
  --threads <n>      Threads na decodificacao [padrao: 1].
  --warmstart <n>    Numero de solucoes gulosas injetadas na populacao inicial
                     (0 = sem warm start) [padrao: 0]. Solucoes repetidas sao
                     descartadas, para nao destruir a diversidade.
  --no_evolution     Desliga os operadores evolutivos (multi-start simples).
  --no_verify        Nao verifica a viabilidade da solucao final.
  --quiet            Nao imprime os rotulos da melhor solucao.
)";

    string config_file;
    string instance_file;
    unsigned seed = 0;
    StopRule stop_rule = StopRule::UNKNOWN;
    unsigned stop_arg = 0;
    seconds max_time {0};
    unsigned num_threads = 1;
    unsigned num_warmstart = 0;
    bool perform_evolution = true;
    bool verify = true;
    bool quiet = false;

    // Parse the command line arguments.
    try {
        const auto args = parse_arguments(argc, argv);

        config_file = get_required(args, "--config");
        instance_file = get_required(args, "--instance");
        seed = unsigned(stoul(get_required(args, "--seed")));
        stop_rule = StopRule(toupper(get_required(args, "--stop_rule")[0]));
        stop_arg = unsigned(stoul(get_required(args, "--stop_arg")));
        max_time = seconds {stol(get_required(args, "--maxtime"))};
        num_threads = unsigned(stoul(get_optional(args, "--threads", "1")));
        num_warmstart = unsigned(stoul(get_optional(args, "--warmstart", "0")));
        perform_evolution = (args.count("--no_evolution") == 0);
        verify = (args.count("--no_verify") == 0);
        quiet = (args.count("--quiet") > 0);

        if(stop_rule != StopRule::GENERATIONS &&
           stop_rule != StopRule::IMPROVEMENT)
            throw logic_error("Incorrect stop rule. Must be either "
                              "(G)enerations or (I)mprovement");

        if(stop_arg == 0)
            throw logic_error("'stop_arg' must be > 0");

        if(max_time <= seconds {0})
            throw logic_error("'maxtime' must be > 0");

        if(num_threads == 0 || num_threads > 64)
            throw logic_error("'threads' must be in [1, 64]");
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "ERROR: " << e.what() << "\n"
        << string(40, '*') << "\n"
        << usage
        << endl;
        return 1;
    }

    // Main algorithm.
    try {
        log("Experiment started");

        cout
        << "> Instance: '" << instance_file << "'"
        << "\n> Loading config file: '" << config_file << "'"
        << endl;

        ////////////////////////////////////////
        // Load config file and show basic info.
        ////////////////////////////////////////

        auto [brkga_params, control_params] =
            BRKGA::readConfiguration(config_file);

        // As duas paradas valem em OU: o tempo maximo e' sempre testado pela
        // API; a regra escolhida entra como geracoes (criterio proprio) ou
        // como geracoes sem melhora (stall_offset).
        control_params.maximum_running_time = max_time;
        if(stop_rule == StopRule::IMPROVEMENT)
            control_params.stall_offset = stop_arg;

        // NOTE: com 'pr_distance_function_type CUSTOM' no config, o IPR so'
        // funciona se a distancia for fornecida ANTES de construir o
        // algoritmo (o construtor copia os parametros).
        using BRKGA::PathRelinking::DistanceFunctionType;
        if(brkga_params.pr_distance_function_type == DistanceFunctionType::CUSTOM) {
            brkga_params.pr_distance_function =
                shared_ptr<BRKGA::DistanceFunctionBase> {new PDR2F_Distance};
        }

        cout
        << "> Algorithm parameters:\n" << brkga_params
        << "> Control parameters:\n" << control_params
        << "\n> Seed: " << seed
        << "\n> Stop rule: "
        << (stop_rule == StopRule::GENERATIONS? "Generations" : "Improvement")
        << "\n> Stop argument: " << stop_arg
        << "\n> Warm-start solutions: " << num_warmstart
        << "\n> Number of threads for decoding: " << num_threads;
        if(!perform_evolution)
            cout << "\n> Simple multi-start: on (no evolutionary operators)";
        cout << endl;

        ////////////////////////////////////////
        // Load instance.
        ////////////////////////////////////////

        log("Reading PDR2F data");

        auto instance = PDR2F_Instance(instance_file);

        size_t num_edges = 0;
        for(const auto& neighbors : instance.adj)
            num_edges += neighbors.size();
        num_edges /= 2;

        cout
        << "Number of nodes: " << instance.num_nodes << "\n"
        << "Number of edges: " << num_edges
        << endl;

        ////////////////////////////////////////
        // Build the BRKGA data structures.
        ////////////////////////////////////////

        log("Building BRKGA");

        // NOTE: o exemplo do TSP reduz a populacao para 10 * num_nodes. Aqui a
        // populacao e' mantida como no config, igual a Torres (2026).
        const auto chromosome_size = instance.num_nodes;

        cout
        << "Population size: " << brkga_params.population_size << "\n"
        << "Chromosome size: " << chromosome_size
        << endl;

        PDR2F_Decoder decoder(instance);

        BRKGA::BRKGA_MP_IPR<PDR2F_Decoder> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed, chromosome_size,
            brkga_params, num_threads, perform_evolution
        );

        // NOTE: com 'bias_type CUSTOM' no config, esta chamada e' obrigatoria.
        // Sem ela, a API usa em silencio o vies CONSTANT (0.5).
        if(brkga_params.bias_type == BRKGA::BiasFunctionType::CUSTOM) {
            algorithm.setBiasCustomFunction(
                [](const unsigned r) { return r == 1? BIAS : 1.0 - BIAS; }
            );
        }

        // Callback para acompanhar a convergencia. Retornar 'true' MANTEM a
        // otimizacao rodando (o laco faz 'run &= callback(status)').
        algorithm.addNewSolutionObserver(
            [](const BRKGA::AlgorithmStatus& status) {
                cout
                << "* " << status.current_iteration << " | "
                << status.best_fitness << " | "
                << status.current_time
                << endl;
                return true;
            }
        );

        // Parada por numero de geracoes (a de tempo ja' e' sempre testada).
        if(stop_rule == StopRule::GENERATIONS) {
            algorithm.setStoppingCriteria(
                [&](const BRKGA::AlgorithmStatus& status) {
                    return status.current_iteration == stop_arg;
                }
            );
        }

        //////////////////////////////////////////////////
        // Injecting the initial/incumbent solutions.
        //////////////////////////////////////////////////

        BRKGA::fitness_t initial_cost = 0.0;
        unsigned num_injected = 0;

        if(num_warmstart > 0) {
            log("Generating initial solutions");

            mt19937 rng(seed);
            rng.discard(rng.state_size);

            // Descarta solucoes repetidas: injetar muitas copias iguais
            // destroi a diversidade da elite logo na geracao 0.
            vector<BRKGA::Chromosome> initial_population;
            vector<vector<unsigned>> seen;

            for(unsigned i = 0; i < num_warmstart; ++i) {
                const auto [cost, labels] = greedy_pdr2f(instance, rng);

                if(i == 0 || cost < initial_cost)
                    initial_cost = cost;

                if(find(seen.begin(), seen.end(), labels) != seen.end())
                    continue;
                seen.push_back(labels);

                // Codifica cada rotulo l como uma chave em [l/4, (l+1)/4).
                BRKGA::Chromosome chromosome(chromosome_size);
                for(unsigned v = 0; v < chromosome_size; ++v) {
                    chromosome[v] =
                        (labels[v] + generate_canonical<double, 48>(rng)) / 4.0;
                }
                initial_population.push_back(chromosome);
            }

            num_injected = unsigned(initial_population.size());

            cout
            << "Greedy solutions: " << num_warmstart
            << " (" << num_injected << " distinct)\n"
            << "Initial cost (best greedy): " << initial_cost
            << endl;

            log("Injecting initial solutions");
            algorithm.setInitialPopulation(initial_population);
        }

        ////////////////////////////////////////
        // Optimizing.
        ////////////////////////////////////////

        log("Optimizing...");
        cout << "* Iteration | Cost | CurrentTime" << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        log("End of optimization");
        cout << "\n> Final status:" << final_status << endl;

        ////////////////////////////////////////
        // Extracting the best solution.
        ////////////////////////////////////////

        // Decodifica de novo o melhor cromossomo (chaves -> rotulos -> reparo).
        vector<unsigned> labels(instance.num_nodes);
        for(unsigned v = 0; v < instance.num_nodes; ++v)
            labels[v] = PDR2F_Decoder::label(final_status.best_chromosome[v]);
        decoder.fixInstance(labels);

        const bool feasible = verify? is_feasible(instance, labels) : true;

        cout
        << "\n% Best solution cost: "
        << setiosflags(ios::fixed) << setprecision(0)
        << final_status.best_fitness
        << endl;

        if(verify)
            cout << "% Feasible: " << (feasible? "yes" : "NO") << endl;

        if(!quiet) {
            for(unsigned label = 0; label < 4; ++label) {
                unsigned count = 0;
                for(unsigned v = 0; v < instance.num_nodes; ++v)
                    if(labels[v] == label)
                        ++count;

                cout << "% Label " << label << " (" << count << " nodes):";
                for(unsigned v = 0; v < instance.num_nodes; ++v)
                    if(labels[v] == label)
                        cout << " " << v;
                cout << endl;
            }
        }

        ////////////////////////////////////////
        // One-liner for the CSV table.
        ////////////////////////////////////////

        string instance_name(instance_file);
        size_t pos = instance_name.rfind('/');
        if(pos != string::npos)
            instance_name = instance_name.substr(pos + 1);

        pos = instance_name.rfind('.');
        if(pos != string::npos)
            instance_name = instance_name.substr(0, pos);

        string config_name(config_file);
        pos = config_name.rfind('/');
        if(pos != string::npos)
            config_name = config_name.substr(pos + 1);

        cout <<
        "\nInstance,"
        "Config,"
        "Seed,"
        "Cost,"
        "Feasible,"
        "NumNodes,"
        "NumEdges,"
        "WarmStart,"
        "InitialCost,"
        "TotalIterations,"
        "LastUpdateIteration,"
        "TotalTime,"
        "LastUpdateTime,"
        "LargestIterationOffset,"
        "StalledIterations,"
        "PRTime,"
        "PRCalls,"
        "PRNumHomogenities,"
        "PRNumImprovBest,"
        "PRNumImprovElite,"
        "NumExchanges,"
        "NumShakes,"
        "NumResets"
        << endl;

        cout
        << instance_name << ","
        << config_name << ","
        << seed << ","
        << setiosflags(ios::fixed) << setprecision(0)
        << final_status.best_fitness << ","
        << (verify? (feasible? "yes" : "NO") : "-") << ","
        << instance.num_nodes << ","
        << num_edges << ","
        << num_injected << ","
        << initial_cost << ","
        << final_status.current_iteration << ","
        << final_status.last_update_iteration << ","
        << setiosflags(ios::fixed) << setprecision(2)
        << final_status.current_time.count() << ","
        << final_status.last_update_time.count() << ","
        << setiosflags(ios::fixed) << setprecision(0)
        << final_status.largest_iteration_offset << ","
        << final_status.stalled_iterations << ","
        << setiosflags(ios::fixed) << setprecision(2)
        << final_status.path_relink_time.count() << ","
        << setiosflags(ios::fixed) << setprecision(0)
        << final_status.num_path_relink_calls << ","
        << final_status.num_homogenities << ","
        << final_status.num_best_improvements << ","
        << final_status.num_elite_improvements << ","
        << final_status.num_exchanges << ","
        << final_status.num_shakes << ","
        << final_status.num_resets
        << endl;

        if(verify && !feasible)
            return 2;
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