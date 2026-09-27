#!/usr/bin/env bash
#
# Experimento do Item 4 - Escalabilidade paralela com Pthreads.
#
# Roda a multiplicacao paralela variando o numero de threads em {1,2,4,8,16}
# e calcula, em relacao ao tempo com 1 thread (T1):
#     Speedup    Sp = T1 / Tp
#     Eficiencia Ep = Sp / p
#
# Faz isso para as duas versoes paralelas:
#   - matmul_pthreads        (paralelo puro)
#   - matmul_pthreads_bloco  (paralelo + blocagem)
#
# Alimenta a Tabela 3 e o grafico de Speedup do relatorio.
#
# Uso: ./scripts/exp_item4.sh [N] [BLK]
#   N   = dimensao da matriz (padrao: 1024)
#   BLK = tamanho do bloco para a versao com blocagem (padrao: 64)

set -euo pipefail
cd "$(dirname "$0")/.."

N="${1:-1024}"
BLK="${2:-64}"
THREADS=(1 2 4 8 16)

extrai_tempo() { sed -n 's/.*Tempo: \([0-9.]*\) s.*/\1/p' | head -1; }

# args: <rotulo> <comando de execucao como string, sem o numero de threads>
# o numero de threads e anexado ao final do comando.
roda_serie() {
    local titulo="$1"; shift
    local montar="$1"; shift   # funcao que monta o comando dado T

    printf "\n--- %s (N=%s) ---\n\n" "$titulo" "$N"
    printf "%-8s | %-12s | %-10s | %-12s\n" "Threads" "Tempo (s)" "Speedup" "Eficiencia"
    printf -- "---------+--------------+------------+-------------\n"

    local t1=""
    for p in "${THREADS[@]}"; do
        local tp
        tp=$($montar "$p" | extrai_tempo)
        if [[ "$p" == "1" ]]; then t1="$tp"; fi
        awk -v p="$p" -v tp="$tp" -v t1="$t1" 'BEGIN{
            sp = (tp>0)? t1/tp : 0;
            ep = sp/p;
            printf "%-8s | %-12s | %-10.2f | %-12.2f\n", p, tp, sp, ep;
        }'
    done
}

cmd_puro()  { bin/matmul_pthreads_O3 "$N" "$1"; }
cmd_bloco() { bin/matmul_pthreads_bloco_O3 "$N" "$1" "$BLK"; }

printf "\n=== ITEM 4: Escalabilidade com Pthreads ===\n"

roda_serie "Pthreads puro"          cmd_puro
roda_serie "Pthreads + Blocagem"    cmd_bloco

printf "\nObs.: Speedup ideal seria linear (Sp = p). O afastamento do ideal em\n"
printf "muitas threads costuma vir da saturacao da largura de banda de memoria\n"
printf "e da disputa pelo cache L3 compartilhado.\n\n"
