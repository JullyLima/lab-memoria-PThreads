#!/usr/bin/env bash
#
# Experimento do Item 3 - Multiplicacao Padrao vs. Blocada, com profiling de
# cache via Valgrind/Cachegrind.
#
# Para cada N em {512,1024,1536} e cada nivel de otimizacao {O0,O3}:
#   1. Roda matmul_padrao e matmul_bloco sob cachegrind.
#   2. Extrai as metricas exigidas pelo relatorio:
#        - D1  miss rate (faltas em cache L1 de dados)
#        - LLd misses     (faltas no Last Level Cache de dados => viagens a DRAM)
#        - Tempo de execucao (impresso pelo proprio programa)
#   3. Salva a saida bruta do cachegrind em results/ para anexar como evidencia.
#
# IMPORTANTE: este script depende do Valgrind, que so existe no LINUX.
# No macOS ele nao roda. Execute-o em uma maquina/VM/WSL Linux.
#
# Uso: ./scripts/exp_item3.sh [BLK]
#   BLK = tamanho do bloco para a versao blocada (padrao: 64)

set -euo pipefail
cd "$(dirname "$0")/.."

BLK="${1:-64}"
TAMANHOS=(512 1024 1536)
NIVEIS=(O0 O3)
OUTDIR=out-put
mkdir -p "$OUTDIR"

if ! command -v valgrind >/dev/null 2>&1; then
    echo "ERRO: 'valgrind' nao encontrado." >&2
    echo "O profiling de cache (Item 3) exige Linux com valgrind instalado." >&2
    echo "  Debian/Ubuntu: sudo apt-get install valgrind" >&2
    exit 1
fi

# Roda um binario sob cachegrind, salva o log e extrai as metricas.
# args: <rotulo> <caminho_bin> <arg1> [arg2...]
perfila() {
    local rotulo="$1"; shift
    local bin="$1"; shift
    local log="$OUTDIR/cachegrind_${rotulo}.txt"

    # --cache-sim=yes e OBRIGATORIO: sem ele o cachegrind so conta instrucoes
    # (I refs) e NAO simula o cache de dados, entao D1/LLd nao aparecem.
    valgrind --tool=cachegrind --cache-sim=yes "$bin" "$@" >"$log" 2>&1 || true

    # Tempo vem da linha impressa pelo programa (ex.: "Tempo: 0.2170 s").
    local tempo
    tempo=$(sed -n 's/.*Tempo: \([0-9.]*\) s.*/\1/p' "$log" | head -1)
    # D1 miss rate e LLd misses vem da saida do cachegrind.
    local d1
    d1=$(sed -n 's/.*D1 *miss rate: *\([0-9.]*\)%.*/\1/p' "$log" | head -1)
    local lld
    lld=$(sed -n 's/.*LLd misses: *\([0-9,]*\).*/\1/p' "$log" | head -1)

    printf "%-28s | %-8s | %-10s | %-14s\n" "$rotulo" "${tempo:-?}" "${d1:-?}" "${lld:-?}"
}

printf "\n=== ITEM 3: Matmul Padrao vs. Blocado (BLK=%s) + Cachegrind ===\n\n" "$BLK"
printf "%-28s | %-8s | %-10s | %-14s\n" "Configuracao" "Tempo(s)" "D1 miss %" "LLd misses"
printf -- "-----------------------------+----------+------------+---------------\n"

for lvl in "${NIVEIS[@]}"; do
    for N in "${TAMANHOS[@]}"; do
        perfila "padrao_${lvl}_N${N}" "bin/matmul_padrao_${lvl}" "$N"
        perfila "bloco_${lvl}_N${N}"  "bin/matmul_bloco_${lvl}"  "$N" "$BLK"
    done
done

printf "\nLogs completos do cachegrind salvos em: %s/\n" "$OUTDIR"
printf "Use-os como evidencia (screenshots) na Tabela 2 do relatorio.\n\n"
