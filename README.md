# BRKGA-MP-IPR para o Problema da Dominação Romana 2-Forte (PDR2F)

Implementação em C++ de um BRKGA para o **Problema da Dominação Romana 2-Forte**, construída sobre a API [BRKGA-MP-IPR](https://github.com/ceandrade/brkga_mp_ipr_cpp) de C. E. Andrade.

O decodificador é o de limiares em 4 partes de Torres (2026), com a rotina de reparo de viabilidade (`fixInstance`). O projeto permite executar tanto o **BRKGA clássico**, com os parâmetros calibrados por Torres, quanto o **BRKGA-MP-IPR completo**, com *path relinking* implícito, *shaking*, *reset*, migração e múltiplas populações.

> Trabalho de Conclusão de Curso 2 — Engenharia de Software, UFC Quixadá.

---

## Sumário

1. [Requisitos](#1-requisitos)
2. [Instalação passo a passo](#2-instalação-passo-a-passo)
3. [Compilação](#3-compilação)
4. [Execução](#4-execução)
5. [Critérios de parada](#5-critérios-de-parada)
6. [Warm start com heurística gulosa](#6-warm-start-com-heurística-gulosa)
7. [Arquivos de configuração](#7-arquivos-de-configuração)
8. [Formato das instâncias](#8-formato-das-instâncias)
9. [Experimentos com várias sementes](#9-experimentos-com-várias-sementes)
10. [Estrutura do projeto](#10-estrutura-do-projeto)
11. [Licença e citação](#11-licença-e-citação)

---

## 1. Requisitos

| Requisito | Versão mínima | Observação |
|---|---|---|
| **GCC (g++)** | 10 | A API usa C++20. |
| **GNU Make** | qualquer | Para compilar pelo `Makefile`. |
| **Git** | qualquer | Para clonar este projeto e a API. |
| **OpenMP** | — | Já acompanha o GCC no Linux. |

O projeto foi desenvolvido e testado em **Linux** (Ubuntu, nativo ou via WSL).

### 1.1 Verificando o que já está instalado

```bash
g++ --version
make --version
git --version
```

---

## 2. Instalação passo a passo

### Passo 1 — Clonar este repositório

```bash
git clone https://github.com/ezequielsan/brkga-mp-ipr-pdr2f.git
cd brkga-mp-ipr-pdr2f
```

### Passo 2 — Baixar a API BRKGA-MP-IPR

A API **não faz parte deste repositório**: ela tem licença e autoria próprias, e por isso a pasta `brkga_mp_ipr/` está no `.gitignore`. Ela precisa ser baixada uma vez, logo após o clone.

A API é *header-only*, ou seja, só arquivos de cabeçalho: não há nada para compilar ou instalar no sistema.

```bash
git clone --depth 1 https://github.com/ceandrade/brkga_mp_ipr_cpp /tmp/brkga_api
cp -r /tmp/brkga_api/brkga_mp_ipr .
rm -rf /tmp/brkga_api
```

### Passo 3 — Conferir se os cabeçalhos ficaram no lugar

```bash
ls brkga_mp_ipr/
```

A saída esperada é:

```
brkga_mp_ipr.hpp  chromosome.hpp  fitness_type.hpp  third_part
```

---

## 3. Compilação

Na raiz do projeto:

```bash
make
```

São gerados **dois executáveis**:

| executável | para quê |
|---|---|
| `main_minimal` | execução simples, com argumentos posicionais. Bom para testar rapidamente. |
| `main_complete` | execução para experimentos: warm start, verificação de viabilidade, log de convergência e uma linha CSV com todas as estatísticas. |

Outros alvos e opções:

```bash
make main_minimal     # compila apenas um dos dois
make main_complete
make clean            # apaga os .o e os executáveis
make clean && make    # recompila do zero (necessário após editar qualquer .hpp)
make -j4              # compila usando 4 processos
make OPT=debug        # compila com símbolos de depuração, sem otimização
```

> **Importante:** o `Makefile` não rastreia dependências de cabeçalhos. Sempre que um arquivo `.hpp` for alterado, rode `make clean && make`.

O `Makefile` compila com `-DMATING_SEED_ONLY`, o que garante que o resultado dependa **apenas da semente**, e não do número de threads. Isso é essencial para a reprodutibilidade dos experimentos.

---

## 4. Execução

### 4.1 `main_minimal` — execução rápida

```
./main_minimal <semente> <arquivo-de-config> <tempo-máximo-em-segundos> <instância> [<gerações>]
```

| Argumento | Obrigatório | Descrição |
|---|---|---|
| `semente` | sim | Semente do gerador de números aleatórios. Mesma semente e mesma configuração produzem o mesmo resultado. |
| `arquivo-de-config` | sim | `config_torres.conf` ou `config_full.conf`. |
| `tempo-máximo` | sim | Limite de tempo, **em segundos**. |
| `instância` | sim | Caminho do arquivo do grafo. |
| `gerações` | não | Número máximo de gerações. Se omitido (ou 0), a parada é só por tempo. |

```bash
# BRKGA clássico, parando em 100 gerações ou 900 s, o que vier primeiro
./main_minimal 1 config_torres.conf 900 instances/ad-hoc-net_150_0.6.txt 100

# BRKGA-MP-IPR completo
./main_minimal 1 config_full.conf 900 instances/ad-hoc-net_150_0.6.txt 100

# Parando somente por tempo (30 segundos)
./main_minimal 1 config_torres.conf 30 instances/bcspwr02.txt
```

### 4.2 `main_complete` — execução para experimentos

```
./main_complete --config <arq> --seed <n> --stop_rule <G|I> --stop_arg <n> \
                --maxtime <s> --instance <arq> \
                [--threads <n>] [--warmstart <n>] [--no_evolution] [--no_verify] [--quiet]
```

| Opção | Padrão | Descrição |
|---|---|---|
| `--config` | — | Arquivo de parâmetros. |
| `--seed` | — | Semente do gerador. |
| `--stop_rule` | — | `G` para número de gerações, `I` para gerações sem melhora. |
| `--stop_arg` | — | Valor da regra de parada. |
| `--maxtime` | — | Tempo máximo, em segundos (sempre vale, em OU com a regra acima). |
| `--instance` | — | Arquivo do grafo. |
| `--threads` | 1 | Threads na decodificação. |
| `--warmstart` | 0 | Número de soluções gulosas injetadas na população inicial (ver Seção 6). |
| `--no_evolution` | — | Desliga os operadores evolutivos (vira um multi-start simples). |
| `--no_verify` | — | Não verifica a viabilidade da solução final. |
| `--quiet` | — | Não imprime a lista de vértices por rótulo. |

```bash
# 100 gerações ou 900 s, o que vier primeiro
./main_complete --config config_torres.conf --seed 1 --stop_rule G --stop_arg 100 \
                --maxtime 900 --instance instances/ad-hoc-net_150_0.6.txt

# parando após 50 gerações sem melhora, com 20 soluções gulosas no início
./main_complete --config config_full.conf --seed 1 --stop_rule I --stop_arg 50 \
                --maxtime 900 --instance instances/ad-hoc-net_150_0.6.txt --warmstart 20
```

**Saídas próprias do `main_complete`:**

- **Cabeçalho do experimento**, com todos os parâmetros lidos do config, a semente, a regra de parada e o número de threads.
- **Log de convergência**, uma linha `* geração | custo | tempo` a cada melhora da melhor solução.
- **Verificação de viabilidade** da solução final, pela caracterização local do problema (regra do rótulo 0 e regra do rótulo 2). Sai como `% Feasible: yes/no`, e o programa retorna código de saída **2** se a solução for inviável.
- **Uma linha CSV** com 23 colunas, pronta para virar tabela:

```
Instance,Config,Seed,Cost,Feasible,NumNodes,NumEdges,WarmStart,InitialCost,TotalIterations,
LastUpdateIteration,TotalTime,LastUpdateTime,LargestIterationOffset,StalledIterations,
PRTime,PRCalls,PRNumHomogenities,PRNumImprovBest,PRNumImprovElite,NumExchanges,NumShakes,NumResets
```

Para acumular resultados em um arquivo, basta pegar a última linha:

```bash
./main_complete --config config_torres.conf --seed 1 --stop_rule G --stop_arg 100 \
                --maxtime 900 --instance instances/jean.txt --quiet | tail -1 >> resultados.csv
```

### 4.3 Entendendo a saída

```
Reading data...
Reading parameters...
Building BRKGA data and initializing...
Running for 900s or 100 generations...
Using 1 threads for decoding
Exchanged 2 solutions from each population. Iteration 118. ...
Path relink at 223 iteration. Block size: 18. Type: DIRECT. Distance: CUSTOM. ...

Algorithm status:
best_fitness: 6
current_iteration: 100
last_update_iteration: 37
current_time: 2.28s
...
num_path_relink_calls: 3
num_exchanges: 8
num_shakes: 1
num_resets: 0

Best cost: 6
% Label 0: 0 1 2 3 ...
% Label 1:
% Label 2:
% Label 3: 69 114
```

- **`Best cost`** é o peso ω(f) da melhor função de dominação romana 2-forte encontrada, ou seja, o valor da função objetivo.
- **`% Label k`** lista os vértices que receberam o rótulo *k* na melhor solução.
- As linhas `Exchanged`, `Path relink`, `Shaking` e `Reset` só aparecem quando o recurso correspondente está ligado no arquivo de configuração.
- O aviso `WARNING: distance function set to 'CUSTOM'` é **esperado**: a função de distância é fornecida pelo código (`distances/pdr2f_distance.hpp`), e não pelo arquivo de configuração.

---

## 5. Critérios de parada

A API **sempre** testa dois critérios, mesmo sem pedido explícito:

1. **tempo máximo** (`--maxtime`, ou `maximum_running_time` no config);
2. **gerações sem melhora** (`stall_offset` no config; 0 desliga).

Os programas acrescentam um terceiro:

3. **número de gerações** (o quinto argumento do `main_minimal`, ou `--stop_rule G --stop_arg n` no `main_complete`).

Os critérios são combinados em **OU**: a execução para no primeiro que for satisfeito. Por exemplo, com 100 gerações e 900 s, ela termina quando completar as 100 gerações ou quando estourar os 900 s, o que acontecer antes.

> **Detalhe de medição:** o tempo é conferido **entre** as etapas de uma iteração (após o `evolve`, antes do IPR, do *shaking* e do *reset*) e com granularidade de segundos. Por isso o corte pode passar um pouco do limite pedido.

---

## 6. Warm start com heurística gulosa

O `main_complete` pode iniciar a população com soluções construídas pela heurística gulosa de Djukanović et al. (2025), a mesma usada por Torres (2026) no algoritmo genético dele:

1. todos os vértices começam descobertos, com rótulo 0;
2. a cada passo, escolhe-se o vértice descoberto que cobre mais vértices novos (empate decidido por uma ordem embaralhada);
3. esse vértice recebe rótulo `min(3, ganho)`, e sua vizinhança fechada passa a estar coberta;
4. repete-se até cobrir todos os vértices.

A solução gerada é sempre viável. O embaralhamento faz com que chamadas sucessivas produzam soluções diferentes.

```bash
./main_complete ... --warmstart 20      # gera 20 soluções gulosas
./main_complete ... --warmstart 0       # sem warm start (padrão)
```

**Soluções repetidas são descartadas** antes da injeção. Isso é proposital: injetar muitas cópias quase iguais preenche a elite já na geração 0 e destrói a diversidade da busca. O log informa quantas soluções distintas sobraram e o custo da melhor delas, que também vai para a coluna `InitialCost` do CSV.

> **O warm start nem sempre ajuda.** Em grafos densos, o guloso produz poucas soluções distintas, e o ganho de qualidade inicial pode não compensar a perda de diversidade. Por isso ele é opcional: o efeito deve ser medido por instância, comparando execuções com e sem, nas mesmas sementes.

---

## 7. Arquivos de configuração

Os dois arquivos usam o formato da API: uma linha por parâmetro, com linhas em branco e linhas iniciadas por `#` sendo ignoradas.

### `config_torres.conf` — BRKGA clássico

Reproduz o BRKGA de Torres (2026), com os parâmetros calibrados por irace:

| parâmetro | valor | significado |
|---|---|---|
| `population_size` | 300 | 39 elites + 242 filhos + 19 mutantes |
| `elite_percentage` | 0.13 | 39 elites |
| `mutants_percentage` | 0.0634 | 19 mutantes |
| `num_elite_parents` / `total_parents` | 1 / 2 | um pai elite e um não elite (BRKGA clássico) |
| `bias_type` | CUSTOM | ρ = 0,7106, definido no código |
| `num_independent_populations` | 1 | uma população |
| `ipr_interval`, `shake_interval`, `reset_interval`, `exchange_interval` | 0 | todos os recursos extras desligados |

### `config_full.conf` — BRKGA-MP-IPR completo

Mantém os mesmos parâmetros evolutivos e liga os recursos da API:

| parâmetro | valor | significado |
|---|---|---|
| `num_independent_populations` | 3 | *island model*: 3 populações de 300 indivíduos, ou seja, 900 decodificações por geração |
| `exchange_interval` / `num_exchange_individuals` | 50 / 2 | migração das 2 melhores de cada ilha |
| `ipr_interval` | 100 | *path relinking* implícito |
| `pr_type` | DIRECT | correto para representações por limiares |
| `pr_distance_function_type` | CUSTOM | usa `PDR2F_Distance` (Hamming sobre os 4 rótulos) |
| `pr_minimum_distance` | 3 | número **absoluto** de vértices com rótulos diferentes |
| `shake_interval` / `shaking_type` | 200 / CHANGE | perturbação por alteração de chaves |
| `reset_interval` | 500 | reinício completo das populações |

> **Atenção:** todos os intervalos (`ipr_interval`, `exchange_interval`, `shake_interval`, `reset_interval`) contam **gerações sem melhora** na melhor solução, e não gerações totais. Dentro de uma iteração, a ordem é: IPR, migração, *shaking*, *reset*.

> **Dois pontos que exigem código, e não apenas configuração:**
> 1. `bias_type CUSTOM` só funciona porque os `main` chamam `setBiasCustomFunction`. Sem essa chamada, a API usaria viés constante (0,5) **sem emitir nenhum aviso**.
> 2. `pr_distance_function_type CUSTOM` exige que `brkga_params.pr_distance_function` receba um `PDR2F_Distance` **antes** de o algoritmo ser construído. Sem isso, com o IPR ligado, a execução aborta com a mensagem "IPR is active but the distance function is not set".

> **Comparações justas entre os dois configs:** com 3 populações, cada geração faz o triplo de decodificações. Para comparar, iguale o número total de decodificações (por exemplo, 300 gerações no clássico contra 100 no completo) ou o tempo de execução.

---

## 8. Formato das instâncias

Cada arquivo é uma **lista de arestas**, uma por linha, com dois inteiros separados por espaço:

```
0 1
0 3
1 2
```

Regras:

- Os vértices são numerados de **0 a n−1**, sem buracos na numeração.
- O grafo é não direcionado; arestas repetidas e laços (`u u`) são descartados na leitura.
- O número de vértices é deduzido do maior rótulo encontrado.

As instâncias disponíveis em `instances/` são:

| arquivo | origem |
|---|---|
| `ad-hoc-net_150_0.6.txt` | rede ad-hoc sem fio, 150 vértices |
| `bcspwr02.txt`, `add20.txt`, `baltimore.txt`, `jean.txt`, `100_5.txt` | grafos de teste diversos |
| `cubic_100.txt` | grafo cúbico aleatório com 100 vértices |

---

## 9. Experimentos com várias sementes

O BRKGA é estocástico: uma execução isolada diz pouco. O padrão é rodar várias sementes e reportar melhor, média, mediana, desvio e pior.

### Resumo rápido no terminal

```bash
for s in $(seq 0 19); do
  ./main_complete --config config_torres.conf --seed $s --stop_rule G --stop_arg 100 \
                  --maxtime 900 --instance instances/ad-hoc-net_150_0.6.txt --quiet \
    | tail -1 | cut -d, -f4
done | python3 -c "
import sys, statistics as st
v = [int(x) for x in sys.stdin]
print('melhor', min(v), '| media', round(st.mean(v), 2), '| mediana', st.median(v),
      '| desvio', round(st.pstdev(v), 2), '| pior', max(v))"
```

### Acumulando um CSV completo

```bash
for inst in instances/*.txt; do
  for s in $(seq 0 19); do
    ./main_complete --config config_torres.conf --seed $s --stop_rule G --stop_arg 100 \
                    --maxtime 900 --instance "$inst" --quiet | tail -1 >> resultados.csv
  done
done
```

O cabeçalho das colunas é impresso pelo programa na penúltima linha da saída; basta copiá-lo uma vez para o topo do `resultados.csv`.

---

## 10. Estrutura do projeto

```
brkga-mp-ipr-pdr2f/
├── pdr2f/
│   ├── pdr2f_instance.hpp      # leitura do grafo e listas de adjacência
│   └── pdr2f_instance.cpp
├── decoders/
│   ├── pdr2f_decoder.hpp       # decodificador de limiares e reparo
│   └── pdr2f_decoder.cpp
├── distances/
│   └── pdr2f_distance.hpp      # distância entre cromossomos, usada pelo IPR
├── heuristics/
│   ├── greedy_pdr2f.hpp        # heurística gulosa (warm start)
│   └── greedy_pdr2f.cpp
├── instances/                  # grafos de teste
├── brkga_mp_ipr/               # API (NÃO versionada — ver o Passo 2)
├── main_minimal.cpp            # execução simples
├── main_complete.cpp           # execução para experimentos
├── config_torres.conf          # BRKGA clássico
├── config_full.conf            # BRKGA-MP-IPR completo
└── Makefile
```

### Como as peças se encaixam

1. **`PDR2F_Instance`** carrega o grafo e monta as listas de adjacência.
2. **`PDR2F_Decoder`** transforma cada cromossomo (vetor de chaves em [0, 1)) em uma solução:
   - cada chave vira um rótulo pela faixa em que cai: [0; 0,25) → 0, [0,25; 0,5) → 1, [0,5; 0,75) → 2, [0,75; 1) → 3;
   - `fixInstance` conserta a rotulação para que ela seja uma função de dominação romana 2-forte;
   - o *fitness* é a soma dos rótulos.
3. **`greedy_pdr2f`** constrói soluções gulosas viáveis, usadas como warm start.
4. **`PDR2F_Distance`** informa ao IPR quando duas chaves representam rótulos diferentes.
5. **`main_minimal.cpp` / `main_complete.cpp`** leem os argumentos e a configuração, montam o algoritmo, definem o viés, a função de distância e os critérios de parada, e chamam `run()`.

> **Por que o reparo é aplicado de novo ao final:** a API guarda apenas as chaves e o valor do *fitness*, nunca a solução decodificada, e o nosso decodificador não reescreve o cromossomo. Por isso, para exibir a solução, os `main` refazem o mesmo caminho do `decode`: convertem as chaves em rótulos e aplicam o `fixInstance`. Como a rotina é determinística, a soma resultante é sempre igual ao `best_fitness` informado pela API.

---

## 11. Licença e citação

Este repositório **não redistribui** a API BRKGA-MP-IPR: ela é baixada separadamente (Passo 2) e mantém a licença do seu autor.

A licença da API exige que qualquer publicação ou software que use a biblioteca cite o artigo:

> C. E. Andrade, R. F. Toso, J. F. Gonçalves, M. G. C. Resende. *The Multi-Parent Biased Random-key Genetic Algorithm with Implicit Path Relinking.* **European Journal of Operational Research**, v. 289, n. 1, p. 17–30, 2021. DOI: [10.1016/j.ejor.2019.11.037](https://doi.org/10.1016/j.ejor.2019.11.037)

### Referências do trabalho

- **API BRKGA-MP-IPR (C++):** https://github.com/ceandrade/brkga_mp_ipr_cpp
- **Decodificador e heurística de reparo:** L. A. Torres. *Algoritmos genéticos para o Problema da Dominação Romana 2-Forte.* Trabalho de Conclusão de Curso, UFC Quixadá, 2026.
- **Heurística construtiva e meta-heurística VNS para o problema:** M. Djukanović et al. *Graph Protection Under Multiple Simultaneous Attacks: A Heuristic Approach*, 2025.