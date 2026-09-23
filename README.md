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
5. [Arquivos de configuração](#5-arquivos-de-configuração)
6. [Formato das instâncias](#6-formato-das-instâncias)
7. [Rodando experimentos com várias sementes](#7-rodando-experimentos-com-várias-sementes)
8. [Estrutura do projeto](#8-estrutura-do-projeto)
9. [Solução de problemas](#9-solução-de-problemas)
10. [Licença e citação](#10-licença-e-citação)

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

A saída termina com:

```
--> Linking objects...
g++ ... -o main_minimal
```

Outros alvos e opções:

```bash
make clean            # apaga os .o e o executável
make clean && make    # recompila do zero (necessário após editar qualquer .hpp)
make -j4              # compila usando 4 processos
make OPT=debug        # compila com símbolos de depuração, sem otimização
```

> **Importante:** o `Makefile` não rastreia dependências de cabeçalhos. Sempre que um arquivo `.hpp` for alterado, rode `make clean && make`.

O `Makefile` compila com `-DMATING_SEED_ONLY`, o que garante que o resultado dependa **apenas da semente**, e não do número de threads. Isso é essencial para a reprodutibilidade dos experimentos.

---

## 4. Execução

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

### Exemplos

```bash
# BRKGA clássico, com os parâmetros de Torres, parando em 100 gerações
./main_minimal 1 config_torres.conf 180 instances/ad-hoc-net_150_0.6.txt 100

# BRKGA-MP-IPR completo, também em 100 gerações
./main_minimal 1 config_full.conf 180 instances/ad-hoc-net_150_0.6.txt 100

# Parando somente por tempo (30 segundos)
./main_minimal 1 config_torres.conf 30 instances/bcspwr02.txt
```

### Entendendo a saída

```
Reading data...
Reading parameters...
Building BRKGA data and initializing...
Running for 600s or 100 generations...
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

## 5. Arquivos de configuração

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
> 1. `bias_type CUSTOM` só funciona porque o `main_minimal.cpp` chama `setBiasCustomFunction`. Sem essa chamada, a API usaria viés constante (0,5) **sem emitir nenhum aviso**.
> 2. `pr_distance_function_type CUSTOM` exige que `brkga_params.pr_distance_function` receba um `PDR2F_Distance` **antes** de o algoritmo ser construído. Sem isso, com o IPR ligado, a execução aborta com a mensagem "IPR is active but the distance function is not set".

---

## 6. Formato das instâncias

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

## 7. Estrutura do projeto

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
├── instances/                  # grafos de teste
├── brkga_mp_ipr/               # API (NÃO versionada — ver o Passo 2)
├── main_minimal.cpp            # programa principal
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
3. **`PDR2F_Distance`** informa ao IPR quando duas chaves representam rótulos diferentes.
4. **`main_minimal.cpp`** lê os argumentos e a configuração, monta o algoritmo, define o viés, a função de distância e o critério de parada, e chama `run()`.

---

## 8. Licença e citação

Este repositório **não redistribui** a API BRKGA-MP-IPR: ela é baixada separadamente (Passo 2) e mantém a licença do seu autor.

A licença da API exige que qualquer publicação ou software que use a biblioteca cite o artigo:

> C. E. Andrade, R. F. Toso, J. F. Gonçalves, M. G. C. Resende. *The Multi-Parent Biased Random-key Genetic Algorithm with Implicit Path Relinking.* **European Journal of Operational Research**, v. 289, n. 1, p. 17–30, 2021. DOI: [10.1016/j.ejor.2019.11.037](https://doi.org/10.1016/j.ejor.2019.11.037)

### Referências do trabalho

- **API BRKGA-MP-IPR (C++):** https://github.com/ceandrade/brkga_mp_ipr_cpp
- **Decodificador e heurística de reparo:** L. A. Torres. *Algoritmos genéticos para o Problema da Dominação Romana 2-Forte.* Trabalho de Conclusão de Curso, UFC Quixadá, 2026.
- **Heurística construtiva e meta-heurística VNS para o problema:** M. Djukanović et al. *Graph Protection Under Multiple Simultaneous Attacks: A Heuristic Approach*, 2025.