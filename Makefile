CC := gcc
INCLUDES := -Ibestiario -Igerador_de_encontros -Igerenciamento_de_combate -Ipersonagens_e_equipamentos
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic $(INCLUDES)

MODULOS := \
	bestiario/monstro.c \
	bestiario/bestiario.c \
	bestiario/menu_bestiario.c \
	personagens_e_equipamentos/item.c \
	personagens_e_equipamentos/inventario.c \
	personagens_e_equipamentos/personagem.c \
	personagens_e_equipamentos/menu_personagens.c \
	gerador_de_encontros/dados.c \
	gerador_de_encontros/grupo.c \
	gerador_de_encontros/encontro.c \
	gerador_de_encontros/menu_encontros.c \
	gerenciamento_de_combate/combate.c \
	gerenciamento_de_combate/menu_combate.c

HEADERS := $(wildcard bestiario/*.h gerador_de_encontros/*.h \
	gerenciamento_de_combate/*.h personagens_e_equipamentos/*.h)

.PHONY: all clean modulos-biblioteca

all: mestre-rpg

mestre-rpg: main.c $(MODULOS) $(HEADERS)
	$(CC) $(CFLAGS) main.c $(MODULOS) -o $@

modulos-biblioteca:
	$(CC) $(CFLAGS) -c $(MODULOS)

clean:
	rm -f mestre-rpg bestiario/bestiario personagens_e_equipamentos/personagens *.o