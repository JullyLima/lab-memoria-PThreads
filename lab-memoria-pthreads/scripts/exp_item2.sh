#!/usr/bin/env bash
#
# Experimento do Item 2 - Localidade Espacial (Linha vs. Coluna).
#
# Roda varredura_linha e varredura_coluna para N em {512,1024,2048,4096,8192}
# e imprime uma tabela com os tempos e a Razao de Desaceleracao (slowdown):
#
#     slowdown = T_coluna / T_linha
#
# Usa a versao -O3 (otimizada). Os resultados alimentam a Tabela 1 do relatorio.
#
# Uso: ./scripts/exp_item2.sh   (rode "make" antes para gerar os binarios)

set -euo pipefail
cd "$(dirname "$0")/.."

LINHA=bin/varredura_linha_O3
COLUNA=bin/varredura_coluna_O3

if [[ ! -x "$LINHA" || ! -x "$COLUNA" ]]; then
    echo "Binarios nao encontrados. Rode 'make' primeiro." >&2
    exit 1
fi

TAMANHOS=(512 1024 2048 4096 8192)

printf "\n=== ITEM 2: Localidade Espacial (Linha vs. Coluna) ===\n\n"
printf "%-8s | %-14s | %-14s | %-10s\n" "N" "T_linha (s)" "T_coluna (s)" "Slowdown"
printf -- "---------+----------------+----------------+-----------\n"

# extrai o campo "Tempo: X s" da saida de um binario
extrai_tempo() {
    "$1" "$2" | sed -n 's/.*Tempo: \([0-9.]*\) s.*/\1/p'
}

for N in "${TAMANHOS[@]}"; do
    tl=$(extrai_tempo "$LINHA"  "$N")
    tc=$(extrai_tempo "$COLUNA" "$N")
    slowdown=$(awk -v a="$tc" -v b="$tl" 'BEGIN{ if (b>0) printf "%.2fx", a/b; else print "n/a" }')
    printf "%-8s | %-14s | %-14s | %-10s\n" "$N" "$tl" "$tc" "$slowdown"
done

printf "\nDica: quanto maior N, maior tende a ser o slowdown, pois a matriz\n"
printf "deixa de caber nos caches e a varredura por coluna passa a sofrer um\n"
printf "cache miss a cada acesso.\n\n"
