CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic

MODULOS := \
	bestiario/monstro.c \
	bestiario/bestiario.c \
	personagens_e_equipamentos/item.c \
	personagens_e_equipamentos/inventario.c \
	personagens_e_equipamentos/personagem.c \
	gerador_de_encontros/dados.c \
	gerador_de_encontros/grupo.c \
	gerador_de_encontros/encontro.c

HEADERS := $(wildcard bestiario/*.h gerador_de_encontros/*.h \
	gerenciamento_de_combate/*.h personagens_e_equipamentos/*.h)

.PHONY: all clean modulos-biblioteca

all: mestre-rpg

mestre-rpg: main.c bestiario/main.c personagens_e_equipamentos/main.c \
		gerenciamento_de_combate/main.c $(MODULOS) $(HEADERS)
	$(CC) $(CFLAGS) main.c bestiario/main.c \
		personagens_e_equipamentos/main.c gerenciamento_de_combate/main.c \
		$(MODULOS) -o $@

modulos-biblioteca:
	$(CC) $(CFLAGS) -c $(MODULOS)

clean:
	rm -f mestre-rpg bestiario/bestiario personagens_e_equipamentos/personagens *.o
