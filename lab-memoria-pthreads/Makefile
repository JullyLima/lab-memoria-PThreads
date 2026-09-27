# Makefile - Laboratorio de Hierarquia de Memoria e Pthreads
#
# Compila cada programa em DOIS niveis de otimizacao:
#   *_O0  -> gcc -O0  (sem otimizacoes, baseline)
#   *_O3  -> gcc -O3  (otimizacao maxima: auto-vetorizacao SIMD, unrolling, etc.)
#
# Todos os binarios sao gerados no diretorio ./bin.
#
# Alvos uteis:
#   make          -> compila todos os programas nos dois niveis
#   make clean    -> remove os binarios
#   make run2     -> roda o experimento do Item 2 (varredura linha vs coluna)
#   make run3     -> roda o experimento do Item 3 (matmul + cachegrind)
#   make run4     -> roda o experimento do Item 4 (speedup com pthreads)

CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11
O0FLAGS := -O0
O3FLAGS := -O3 -march=native
LDLIBS  := -pthread

SRCDIR := src
BINDIR := bin

# util.c e compartilhado por todos os programas.
COMMON := $(SRCDIR)/util.c
COMMON_H := $(SRCDIR)/util.h

# Nomes-base dos programas (cada .c em src/, exceto util.c).
PROGS := varredura_linha varredura_coluna \
         matmul_padrao matmul_bloco \
         matmul_pthreads matmul_pthreads_bloco

# Lista final de binarios: cada programa em _O0 e _O3.
BINS := $(addprefix $(BINDIR)/,$(addsuffix _O0,$(PROGS))) \
        $(addprefix $(BINDIR)/,$(addsuffix _O3,$(PROGS)))

.PHONY: all clean run2 run3 run4

all: $(BINS)

$(BINDIR):
	mkdir -p $(BINDIR)

# Regra para os binarios -O0
$(BINDIR)/%_O0: $(SRCDIR)/%.c $(COMMON) $(COMMON_H) | $(BINDIR)
	$(CC) $(CFLAGS) $(O0FLAGS) $< $(COMMON) -o $@ $(LDLIBS)

# Regra para os binarios -O3
$(BINDIR)/%_O3: $(SRCDIR)/%.c $(COMMON) $(COMMON_H) | $(BINDIR)
	$(CC) $(CFLAGS) $(O3FLAGS) $< $(COMMON) -o $@ $(LDLIBS)

clean:
	rm -rf $(BINDIR)

# Os scripts sao chamados via "bash" (e nao "./script.sh") de proposito:
# assim funcionam mesmo que o bit de execucao (chmod +x) tenha se perdido ao
# transferir/descompactar os arquivos - evita o erro "permission denied" (127).
run2: all
	bash scripts/exp_item2.sh

run3: all
	bash scripts/exp_item3.sh

run4: all
	bash scripts/exp_item4.sh
