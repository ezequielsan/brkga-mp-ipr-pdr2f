#!/usr/bin/env bash
###############################################################################
# run_experiment.sh: roda o main_complete para varias sementes, acumula um CSV
# e imprime estatisticas por instancia e por decodificador.
#
# Uso:
#   ./scripts/run_experiment.sh [opcoes] <instancia> [<instancia> ...]
#
# Opcoes:
#   -c <arq>    arquivo de configuracao        [config_torres.conf]
#   -d "<...>"  decodificadores a testar       ["threshold permutation"]
#   -s <a-b>    intervalo de sementes          [0-19]
#   -g <n>      numero de geracoes             [100]
#   -t <s>      tempo maximo, em segundos      [900]
#   -w <n>      solucoes gulosas (warm start)  [0]
#   -o <arq>    CSV de saida                   [results/resultados.csv]
#
# Exemplos:
#   ./scripts/run_experiment.sh instances/ad-hoc-net_150_0.6.txt
#   ./scripts/run_experiment.sh -c config_full.conf -g 300 instances/*.txt
#   ./scripts/run_experiment.sh -d threshold -s 0-9 instances/jean.txt
###############################################################################

set -u

CONFIG="config_torres.conf"
DECODERS="threshold permutation"
SEEDS="0-19"
GENERATIONS=100
MAXTIME=900
WARMSTART=0
OUTPUT="results/resultados.csv"

while getopts "c:d:s:g:t:w:o:h" opt; do
    case $opt in
        c) CONFIG="$OPTARG" ;;
        d) DECODERS="$OPTARG" ;;
        s) SEEDS="$OPTARG" ;;
        g) GENERATIONS="$OPTARG" ;;
        t) MAXTIME="$OPTARG" ;;
        w) WARMSTART="$OPTARG" ;;
        o) OUTPUT="$OPTARG" ;;
        h) sed -n '2,30p' "$0"; exit 0 ;;
        *) echo "Use -h para a ajuda."; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

if [ $# -eq 0 ]; then
    echo "ERRO: informe ao menos uma instancia. Use -h para a ajuda." >&2
    exit 1
fi

if [ ! -x ./main_complete ]; then
    echo "ERRO: ./main_complete nao encontrado. Rode 'make' antes." >&2
    exit 1
fi

SEED_START="${SEEDS%%-*}"
SEED_END="${SEEDS##*-}"

mkdir -p "$(dirname "$OUTPUT")"

TMP_LOG=$(mktemp)
trap 'rm -f "$TMP_LOG"' EXIT

###############################################################################
# Execucoes
###############################################################################

echo "=============================================================="
echo " config:        $CONFIG"
echo " decodificador: $DECODERS"
echo " sementes:      $SEED_START a $SEED_END"
echo " parada:        $GENERATIONS geracoes OU $MAXTIME s"
echo " warm start:    $WARMSTART"
echo " saida:         $OUTPUT"
echo "=============================================================="

for instance in "$@"; do
    for decoder in $DECODERS; do
        echo ""
        echo "-> $(basename "$instance") | decoder=$decoder"

        for seed in $(seq "$SEED_START" "$SEED_END"); do
            if ! ./main_complete \
                       --config "$CONFIG" \
                       --decoder "$decoder" \
                       --seed "$seed" \
                       --stop_rule G \
                       --stop_arg "$GENERATIONS" \
                       --maxtime "$MAXTIME" \
                       --warmstart "$WARMSTART" \
                       --instance "$instance" \
                       --quiet > "$TMP_LOG" 2>&1; then
                echo "   semente $seed: FALHOU" >&2
                tail -3 "$TMP_LOG" >&2
                continue
            fi

            line=$(tail -1 "$TMP_LOG")

            # A linha do CSV tem 24 campos; qualquer coisa diferente e' erro.
            if [ "$(echo "$line" | awk -F, '{print NF}')" -ne 24 ]; then
                echo "   semente $seed: saida inesperada, linha ignorada" >&2
                continue
            fi

            # Escreve o cabecalho apenas uma vez.
            if [ ! -s "$OUTPUT" ]; then
                echo "Instance,Config,Decoder,Seed,Cost,Feasible,NumNodes,NumEdges,WarmStart,InitialCost,TotalIterations,LastUpdateIteration,TotalTime,LastUpdateTime,LargestIterationOffset,StalledIterations,PRTime,PRCalls,PRNumHomogenities,PRNumImprovBest,PRNumImprovElite,NumExchanges,NumShakes,NumResets" > "$OUTPUT"
            fi

            echo "$line" >> "$OUTPUT"
            printf "   semente %-3s custo %s\n" "$seed" "$(echo "$line" | cut -d, -f5)"
        done
    done
done

###############################################################################
# Estatisticas
###############################################################################

echo ""
echo "=============================================================="
echo " ESTATISTICAS ($OUTPUT)"
echo "=============================================================="

python3 - "$OUTPUT" <<'PYTHON'
import csv, statistics as st, sys
from collections import defaultdict

with open(sys.argv[1]) as fh:
    rows = list(csv.DictReader(fh))

grupos = defaultdict(list)
for r in rows:
    grupos[(r["Instance"], r["Config"], r["Decoder"])].append(r)

cab = (f'{"instancia":<28}{"config":<22}{"decoder":<13}{"n":>3}'
       f'{"melhor":>8}{"media":>9}{"mediana":>9}{"moda":>7}'
       f'{"desvio":>8}{"pior":>7}{"tempo(s)":>10}{"viaveis":>9}')
print(cab)
print("-" * len(cab))

for (inst, cfg, dec), grupo in sorted(grupos.items()):
    custos = [int(float(r["Cost"])) for r in grupo]
    tempos = [float(r["TotalTime"]) for r in grupo]
    viaveis = sum(1 for r in grupo if r["Feasible"] == "yes")

    # A moda pode nao ser unica; nesse caso mostramos a menor.
    modas = st.multimode(custos)

    print(f'{inst[:27]:<28}{cfg[:21]:<22}{dec[:12]:<13}{len(custos):>3}'
          f'{min(custos):>8}{st.mean(custos):>9.2f}{st.median(custos):>9.1f}'
          f'{min(modas):>7}{st.pstdev(custos):>8.2f}{max(custos):>7}'
          f'{st.mean(tempos):>10.2f}{f"{viaveis}/{len(grupo)}":>9}')
PYTHON

echo ""
echo "Linhas acumuladas em: $OUTPUT"