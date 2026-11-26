#ifndef DADOS_H
#define DADOS_H

#include "../bestiario/monstro.h"

// Protótipo obrigatório
/*Aqui rola uma struct Dado (qtd de faces+modificador) usando rand() e reotorna o total obtido (que garante pelo menos 0 caso o modificador seja muito negativo)*/
int rolaDado(Dado dado);

#endif // DADOS_H