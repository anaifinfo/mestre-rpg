#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "combate.h"

static int rolaDadosVidaMonstro(const Monstro *esp)
{
    if (esp == NULL) {
        return 1;
    }
    int pv = 0;
    if (validaDado(esp->dadosVida)) {
        pv = rolaDado(esp->dadosVida);
    } else if (esp->dadosVida.quantidade > 0 && esp->dadosVida.faces > 0) {
        Dado d = {esp->dadosVida.quantidade, esp->dadosVida.faces, esp->dadosVida.modificador};
        for (int i = 0; i < d.quantidade; i++) {
            pv += (rand() % d.faces) + 1;
        }
        pv += d.modificador;
    }
    return (pv < 1) ? 1 : pv;
}

void inicializaCombate(Combate *combate)
{
    if (combate == NULL) {
        return;
    }
    memset(combate, 0, sizeof(*combate));
    combate->vencedor = LADO_NENHUM;
}

int preparaCombate(Combate *combate,
                   CadastroPersonagens *cadastro,
                   const GrupoAventura *grupo,
                   const Encontro *encontro,
                   const Bestiario *bestiario)
{
    if (combate == NULL || cadastro == NULL || grupo == NULL ||
        encontro == NULL || bestiario == NULL) {
        return 0;
    }

    inicializaCombate(combate);

    for (int i = 0; i < grupo->quantidade && i < MAX_PERSONAGENS_GRUPO; i++) {
        int idPers = grupo->idsPersonagens[i];
        Personagem *p = buscarPersonagem(cadastro, idPers);
        if (p == NULL) {
            continue;
        }
        int pvMax = calcularVidaMaximaTotal(p);
        if (pvMax < 1) {
            pvMax = (p->vidaMaxima > 0) ? p->vidaMaxima : 1;
        }

        PersonagemCombate *pc = &combate->personagens[combate->quantidadePersonagens];
        pc->ficha = p;
        pc->pontosVidaMaximos = pvMax;
        pc->pontosVidaAtuais = (p->hp > 0) ? p->hp : pvMax;
        if (pc->pontosVidaAtuais > pc->pontosVidaMaximos) {
            pc->pontosVidaAtuais = pc->pontosVidaMaximos;
        }
        pc->iniciativa = 0;
        pc->estado = (pc->pontosVidaAtuais > 0) ? COMBATENTE_ATIVO : COMBATENTE_DERROTADO;
        combate->quantidadePersonagens++;
    }

    int exemplar = 1;
    for (int i = 0; i < encontro->quantidadeEspecies && i < MAX_ESPECIES_ENCONTRO; i++) {
        const ComponenteEncontro *comp = &encontro->componentes[i];
        const Monstro *esp = consultaMonstroPorId(bestiario, comp->idMonstro);
        if (esp == NULL || comp->quantidade <= 0) {
            continue;
        }

        for (int q = 0; q < comp->quantidade && combate->quantidadeMonstros < MAX_MONSTROS_ENCONTRO; q++) {
            int pvMonstro = rolaDadosVidaMonstro(esp);
            MonstroCombate *mc = &combate->monstros[combate->quantidadeMonstros];
            mc->numeroExemplar = exemplar++;
            mc->especie = esp;
            mc->pontosVidaMaximos = pvMonstro;
            mc->pontosVidaAtuais = pvMonstro;
            mc->iniciativa = 0;
            mc->estado = COMBATENTE_ATIVO;
            combate->quantidadeMonstros++;
        }
    }

    if (combate->quantidadePersonagens == 0 || combate->quantidadeMonstros == 0) {
        return 0;
    }

    combate->rodadaAtual = 0;
    combate->encerrado = 0;
    combate->vencedor = LADO_NENHUM;

    rolaIniciativas(combate);
    montaOrdemCombate(combate);
    return 1;
}

int contaPersonagensAtivos(const Combate *combate)
{
    if (combate == NULL) {
        return 0;
    }
    int ativos = 0;
    for (int i = 0; i < combate->quantidadePersonagens; i++) {
        if (combate->personagens[i].estado == COMBATENTE_ATIVO &&
            combate->personagens[i].pontosVidaAtuais > 0) {
            ativos++;
        }
    }
    return ativos;
}

int contaMonstrosAtivos(const Combate *combate)
{
    if (combate == NULL) {
        return 0;
    }
    int ativos = 0;
    for (int i = 0; i < combate->quantidadeMonstros; i++) {
        if (combate->monstros[i].estado == COMBATENTE_ATIVO &&
            combate->monstros[i].pontosVidaAtuais > 0) {
            ativos++;
        }
    }
    return ativos;
}

int combateTerminou(Combate *combate)
{
    if (combate == NULL) {
        return 1;
    }

    int pAtivos = contaPersonagensAtivos(combate);
    int mAtivos = contaMonstrosAtivos(combate);

    if (pAtivos == 0 && mAtivos == 0) {
        combate->encerrado = 1;
        combate->vencedor = LADO_NENHUM;
        return 1;
    }
    if (mAtivos == 0) {
        combate->encerrado = 1;
        combate->vencedor = LADO_PERSONAGENS;
        return 1;
    }
    if (pAtivos == 0) {
        combate->encerrado = 1;
        combate->vencedor = LADO_MONSTROS;
        return 1;
    }
    if (combate->rodadaAtual >= MAX_RODADAS) {
        combate->encerrado = 1;
        combate->vencedor = LADO_NENHUM;
        return 1;
    }

    return 0;
}

void rolaIniciativas(Combate *combate)
{
    if (combate == NULL) {
        return;
    }

    Dado d20 = {1, 20, 0};

    for (int i = 0; i < combate->quantidadePersonagens; i++) {
        PersonagemCombate *pc = &combate->personagens[i];
        int mod = (pc->ficha != NULL) ? calcularIniciativaTotal(pc->ficha) : 0;
        pc->iniciativa = rolaDado(d20) + mod;
    }

    for (int i = 0; i < combate->quantidadeMonstros; i++) {
        MonstroCombate *mc = &combate->monstros[i];
        int mod = 0;
        if (mc->especie != NULL) {
            mod = (mc->especie->poder.modDia + mc->especie->poder.modNoite) / 2;
        }
        mc->iniciativa = rolaDado(d20) + mod;
    }
}

void montaOrdemCombate(Combate *combate)
{
    if (combate == NULL) {
        return;
    }

    int total = 0;
    for (int i = 0; i < combate->quantidadePersonagens && total < MAX_COMBATENTES; i++) {
        combate->ordem[total].lado = LADO_PERSONAGENS;
        combate->ordem[total].indice = i;
        combate->ordem[total].iniciativa = combate->personagens[i].iniciativa;
        total++;
    }

    for (int i = 0; i < combate->quantidadeMonstros && total < MAX_COMBATENTES; i++) {
        combate->ordem[total].lado = LADO_MONSTROS;
        combate->ordem[total].indice = i;
        combate->ordem[total].iniciativa = combate->monstros[i].iniciativa;
        total++;
    }

    for (int i = 0; i < total - 1; i++) {
        for (int j = i + 1; j < total; j++) {
            int trocar = 0;
            if (combate->ordem[j].iniciativa > combate->ordem[i].iniciativa) {
                trocar = 1;
            } else if (combate->ordem[j].iniciativa == combate->ordem[i].iniciativa &&
                       combate->ordem[j].lado == LADO_PERSONAGENS &&
                       combate->ordem[i].lado == LADO_MONSTROS) {
                trocar = 1;
            }
            if (trocar) {
                EntradaOrdem tmp = combate->ordem[i];
                combate->ordem[i] = combate->ordem[j];
                combate->ordem[j] = tmp;
            }
        }
    }

    combate->quantidadeOrdem = total;
}

void exibeOrdem(const Combate *combate)
{
    if (combate == NULL) {
        return;
    }

    printf("\n--- Ordem de Iniciativa (Rodada %d) ---\n", combate->rodadaAtual);
    for (int i = 0; i < combate->quantidadeOrdem; i++) {
        EntradaOrdem e = combate->ordem[i];
        if (e.lado == LADO_PERSONAGENS && e.indice < combate->quantidadePersonagens) {
            const PersonagemCombate *pc = &combate->personagens[e.indice];
            const char *nome = (pc->ficha != NULL) ? pc->ficha->nome : "Personagem";
            printf("%d. [PJ] %s (Ini: %d | PV: %d/%d)%s\n",
                   i + 1, nome, e.iniciativa,
                   pc->pontosVidaAtuais, pc->pontosVidaMaximos,
                   pc->estado == COMBATENTE_ATIVO ? "" : " [DERROTADO]");
        } else if (e.lado == LADO_MONSTROS && e.indice < combate->quantidadeMonstros) {
            const MonstroCombate *mc = &combate->monstros[e.indice];
            const char *nome = (mc->especie != NULL) ? mc->especie->nome : "Monstro";
            printf("%d. [MO] %s #%d (Ini: %d | PV: %d/%d)%s\n",
                   i + 1, nome, mc->numeroExemplar, e.iniciativa,
                   mc->pontosVidaAtuais, mc->pontosVidaMaximos,
                   mc->estado == COMBATENTE_ATIVO ? "" : " [DERROTADO]");
        }
    }
}

int selecionaPersonagemAlvo(const Combate *combate)
{
    if (combate == NULL) {
        return -1;
    }
    for (int i = 0; i < combate->quantidadePersonagens; i++) {
        if (combate->personagens[i].estado == COMBATENTE_ATIVO &&
            combate->personagens[i].pontosVidaAtuais > 0) {
            return i;
        }
    }
    return -1;
}

int selecionaMonstroAlvo(const Combate *combate)
{
    if (combate == NULL) {
        return -1;
    }
    for (int i = 0; i < combate->quantidadeMonstros; i++) {
        if (combate->monstros[i].estado == COMBATENTE_ATIVO &&
            combate->monstros[i].pontosVidaAtuais > 0) {
            return i;
        }
    }
    return -1;
}

int defesaEquivalenteMonstro(const Monstro *monstro)
{
    if (monstro == NULL) {
        return 10;
    }
    int ca = 20 - monstro->thac0 + (10 - monstro->classeArmadura);
    if (ca < 1) {
        ca = 10 + (10 - monstro->classeArmadura);
    }
    return (ca < 1) ? 1 : ca;
}

int bonusAtaqueMonstro(const Monstro *monstro)
{
    if (monstro == NULL) {
        return 0;
    }
    int bonus = (20 - monstro->thac0);
    return (bonus < 0) ? 0 : bonus;
}

int verificaAcerto(int resultadoD20, int bonusAtaque, int defesaAlvo)
{
    if (resultadoD20 == 20) {
        return 1;
    }
    if (resultadoD20 == 1) {
        return 0;
    }
    return (resultadoD20 + bonusAtaque) >= defesaAlvo;
}

int aplicaDanoPersonagem(PersonagemCombate *alvo, int dano)
{
    if (alvo == NULL || dano <= 0) {
        return 0;
    }
    alvo->pontosVidaAtuais -= dano;
    if (alvo->pontosVidaAtuais <= 0) {
        alvo->pontosVidaAtuais = 0;
        alvo->estado = COMBATENTE_DERROTADO;
    }
    return dano;
}

int aplicaDanoMonstro(MonstroCombate *alvo, int dano)
{
    if (alvo == NULL || dano <= 0) {
        return 0;
    }
    alvo->pontosVidaAtuais -= dano;
    if (alvo->pontosVidaAtuais <= 0) {
        alvo->pontosVidaAtuais = 0;
        alvo->estado = COMBATENTE_DERROTADO;
    }
    return dano;
}

int executaAtaquePersonagem(Combate *combate, int atacante, int alvo)
{
    if (combate == NULL ||
        atacante < 0 || atacante >= combate->quantidadePersonagens ||
        alvo < 0 || alvo >= combate->quantidadeMonstros) {
        return 0;
    }

    PersonagemCombate *pc = &combate->personagens[atacante];
    MonstroCombate *mc = &combate->monstros[alvo];

    if (pc->estado != COMBATENTE_ATIVO || mc->estado != COMBATENTE_ATIVO || pc->ficha == NULL) {
        return 0;
    }

    Dado d20 = {1, 20, 0};
    int rolagem = rolaDado(d20);
    int bonusAtq = calcularAtaqueTotal(pc->ficha);
    int defAlvo = defesaEquivalenteMonstro(mc->especie);

    const char *nomePJ = pc->ficha->nome;
    const char *nomeMO = (mc->especie != NULL) ? mc->especie->nome : "Monstro";

    if (!verificaAcerto(rolagem, bonusAtq, defAlvo)) {
        printf("  %s ataca %s #%d (d20=%d %+d vs CA %d) -> ERROU!\n",
               nomePJ, nomeMO, mc->numeroExemplar, rolagem, bonusAtq, defAlvo);
        return 0;
    }

    int danoBase = calcularAtaqueTotal(pc->ficha);
    if (danoBase < 1) {
        danoBase = 1;
    }
    if (rolagem == 20) {
        danoBase *= 2;
    }

    int causado = aplicaDanoMonstro(mc, danoBase);
    printf("  %s %s %s #%d (d20=%d %+d vs CA %d) -> %d de dano! (PV %d/%d)%s\n",
           nomePJ,
           (rolagem == 20) ? "acerta CRITICO em" : "acerta",
           nomeMO, mc->numeroExemplar, rolagem, bonusAtq, defAlvo,
           causado, mc->pontosVidaAtuais, mc->pontosVidaMaximos,
           (mc->estado == COMBATENTE_DERROTADO) ? " [DERROTADO]" : "");

    return causado;
}

int executaAtaquesMonstro(Combate *combate, int atacante, int alvo)
{
    if (combate == NULL ||
        atacante < 0 || atacante >= combate->quantidadeMonstros ||
        alvo < 0 || alvo >= combate->quantidadePersonagens) {
        return 0;
    }

    MonstroCombate *mc = &combate->monstros[atacante];
    PersonagemCombate *pc = &combate->personagens[alvo];

    if (mc->estado != COMBATENTE_ATIVO || pc->estado != COMBATENTE_ATIVO || mc->especie == NULL) {
        return 0;
    }

    int numAtaques = mc->especie->quantidadeAtaques;
    if (numAtaques < 1) {
        numAtaques = 1;
    }
    if (numAtaques > MAX_ATAQUES) {
        numAtaques = MAX_ATAQUES;
    }

    int bonusAtq = bonusAtaqueMonstro(mc->especie);
    int defAlvo = (pc->ficha != NULL) ? calcularDefesaTotal(pc->ficha) : 10;
    Dado d20 = {1, 20, 0};
    int danoTotal = 0;

    const char *nomeMO = mc->especie->nome;
    const char *nomePJ = (pc->ficha != NULL) ? pc->ficha->nome : "Personagem";

    for (int k = 0; k < numAtaques; k++) {
        if (pc->estado != COMBATENTE_ATIVO || combateTerminou(combate)) {
            break;
        }

        int rolagem = rolaDado(d20);
        if (!verificaAcerto(rolagem, bonusAtq, defAlvo)) {
            printf("  %s #%d ataca %s (d20=%d %+d vs CA %d) -> ERROU!\n",
                   nomeMO, mc->numeroExemplar, nomePJ, rolagem, bonusAtq, defAlvo);
            continue;
        }

        Dado dadoDano = {1, 6, 0};
        if (k < mc->especie->quantidadeAtaques && validaDado(mc->especie->ataques[k].dano)) {
            dadoDano = mc->especie->ataques[k].dano;
        } else if (mc->especie->quantidadeAtaques > 0 && validaDado(mc->especie->ataques[0].dano)) {
            dadoDano = mc->especie->ataques[0].dano;
        }

        int dano = rolaDado(dadoDano);
        if (dano < 1) {
            dano = 1;
        }
        if (rolagem == 20) {
            dano *= 2;
        }

        int causado = aplicaDanoPersonagem(pc, dano);
        danoTotal += causado;

        printf("  %s #%d %s %s (d20=%d %+d vs CA %d) -> %d de dano! (PV %d/%d)%s\n",
               nomeMO, mc->numeroExemplar,
               (rolagem == 20) ? "acerta CRITICO em" : "acerta",
               nomePJ, rolagem, bonusAtq, defAlvo,
               causado, pc->pontosVidaAtuais, pc->pontosVidaMaximos,
               (pc->estado == COMBATENTE_DERROTADO) ? " [DERROTADO]" : "");
    }

    return danoTotal;
}

int executaTurno(Combate *combate, int posicaoOrdem)
{
    if (combate == NULL || combate->encerrado ||
        posicaoOrdem < 0 || posicaoOrdem >= combate->quantidadeOrdem) {
        return 0;
    }

    if (combateTerminou(combate)) {
        return 0;
    }

    EntradaOrdem entrada = combate->ordem[posicaoOrdem];
    if (entrada.lado == LADO_PERSONAGENS && entrada.indice < combate->quantidadePersonagens) {
        PersonagemCombate *pc = &combate->personagens[entrada.indice];
        if (pc->estado != COMBATENTE_ATIVO) {
            return 0;
        }
        int idxAlvo = selecionaMonstroAlvo(combate);
        if (idxAlvo < 0) {
            combateTerminou(combate);
            return 0;
        }
        executaAtaquePersonagem(combate, entrada.indice, idxAlvo);
    } else if (entrada.lado == LADO_MONSTROS && entrada.indice < combate->quantidadeMonstros) {
        MonstroCombate *mc = &combate->monstros[entrada.indice];
        if (mc->estado != COMBATENTE_ATIVO) {
            return 0;
        }
        int idxAlvo = selecionaPersonagemAlvo(combate);
        if (idxAlvo < 0) {
            combateTerminou(combate);
            return 0;
        }
        executaAtaquesMonstro(combate, entrada.indice, idxAlvo);
    }

    combateTerminou(combate);
    return 1;
}

int executaRodada(Combate *combate)
{
    if (combate == NULL) {
        return 0;
    }
    if (combateTerminou(combate)) {
        return 0;
    }

    combate->rodadaAtual++;
    printf("\n=== Rodada %d ===\n", combate->rodadaAtual);

    for (int i = 0; i < combate->quantidadeOrdem; i++) {
        if (combateTerminou(combate)) {
            break;
        }
        executaTurno(combate, i);
    }

    combateTerminou(combate);
    return 1;
}

int executaCombate(Combate *combate)
{
    if (combate == NULL) {
        return 0;
    }

    while (!combateTerminou(combate)) {
        executaRodada(combate);
    }

    atualizaPersonagensAposCombate(combate);
    return 1;
}

void exibeEstadoCombate(const Combate *combate)
{
    if (combate == NULL) {
        return;
    }

    printf("\n=== Estado do Combate (Rodada %d | %s) ===\n",
           combate->rodadaAtual,
           combate->encerrado ? "Encerrado" : "Em andamento");

    printf("Personagens (%d ativos de %d):\n",
           contaPersonagensAtivos(combate), combate->quantidadePersonagens);
    for (int i = 0; i < combate->quantidadePersonagens; i++) {
        const PersonagemCombate *pc = &combate->personagens[i];
        const char *nome = (pc->ficha != NULL) ? pc->ficha->nome : "Personagem";
        printf("  - %s: PV %d/%d | Ini %d | %s\n",
               nome, pc->pontosVidaAtuais, pc->pontosVidaMaximos, pc->iniciativa,
               pc->estado == COMBATENTE_ATIVO ? "Ativo" : "Derrotado");
    }

    printf("Monstros (%d ativos de %d):\n",
           contaMonstrosAtivos(combate), combate->quantidadeMonstros);
    for (int i = 0; i < combate->quantidadeMonstros; i++) {
        const MonstroCombate *mc = &combate->monstros[i];
        const char *nome = (mc->especie != NULL) ? mc->especie->nome : "Monstro";
        printf("  - %s #%d: PV %d/%d | Ini %d | %s\n",
               nome, mc->numeroExemplar, mc->pontosVidaAtuais, mc->pontosVidaMaximos,
               mc->iniciativa,
               mc->estado == COMBATENTE_ATIVO ? "Ativo" : "Derrotado");
    }
}

int calculaXPDerrotados(const Combate *combate)
{
    if (combate == NULL) {
        return 0;
    }
    int xpTotal = 0;
    for (int i = 0; i < combate->quantidadeMonstros; i++) {
        const MonstroCombate *mc = &combate->monstros[i];
        if (mc->estado == COMBATENTE_DERROTADO && mc->especie != NULL) {
            int xpMonstro = mc->especie->valorXP;
            if (xpMonstro <= 0) {
                xpMonstro = (mc->especie->poder.poderBase > 0) ? mc->especie->poder.poderBase * 10 : 25;
            }
            xpTotal += xpMonstro;
        }
    }
    return xpTotal;
}

int atualizaPersonagensAposCombate(const Combate *combate)
{
    if (combate == NULL) {
        return 0;
    }
    int atualizados = 0;
    for (int i = 0; i < combate->quantidadePersonagens; i++) {
        const PersonagemCombate *pc = &combate->personagens[i];
        if (pc->ficha != NULL) {
            pc->ficha->hp = pc->pontosVidaAtuais;
            if (pc->ficha->hp < 0) {
                pc->ficha->hp = 0;
            }
            if (pc->ficha->hp > pc->ficha->vidaMaxima) {
                pc->ficha->hp = pc->ficha->vidaMaxima;
            }
            atualizados++;
        }
    }
    return atualizados;
}

void exibeResumoCombate(const Combate *combate)
{
    if (combate == NULL) {
        return;
    }

    printf("\n=== Resumo do Combate ===\n");
    printf("Rodadas executadas: %d\n", combate->rodadaAtual);
    if (!combate->encerrado) {
        printf("Status: Em andamento\n");
    } else if (combate->vencedor == LADO_PERSONAGENS) {
        printf("Resultado: Vitoria dos personagens!\n");
    } else if (combate->vencedor == LADO_MONSTROS) {
        printf("Resultado: Vitoria dos monstros.\n");
    } else {
        printf("Resultado: Combate encerrado sem vencedor definido.\n");
    }
    printf("XP total de monstros derrotados: %d\n", calculaXPDerrotados(combate));
}

void menuCombateFluxo(Combate *combate,
                      CadastroPersonagens *cadastro,
                      const GrupoAventura *grupo,
                      const Encontro *encontro,
                      const Bestiario *bestiario)
{
    int opcao;

    if (combate == NULL || cadastro == NULL || grupo == NULL ||
        encontro == NULL || bestiario == NULL) {
        return;
    }

    do {
        printf("\n=== Gerenciamento de Combate ===\n");
        printf("1 - Preparar combate com o encontro escolhido\n");
        printf("2 - Exibir participantes e ordem\n");
        printf("3 - Executar uma rodada\n");
        printf("4 - Executar ate o encerramento\n");
        printf("5 - Exibir estado atual\n");
        printf("6 - Exibir resumo do ultimo combate\n");
        printf("0 - Voltar ao menu principal\n");
        printf("Escolha: ");

        if (scanf("%d", &opcao) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
            }
            printf("Entrada invalida.\n");
            continue;
        }

        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }

        switch (opcao) {
        case 1:
            if (preparaCombate(combate, cadastro, grupo, encontro, bestiario)) {
                printf("Combate preparado com sucesso!\n");
                exibeEstadoCombate(combate);
                exibeOrdem(combate);
            } else {
                printf("Nao foi possivel preparar o combate (verifique grupo e encontro).\n");
            }
            break;
        case 2:
            if (combate->quantidadeOrdem == 0) {
                printf("Prepare o combate primeiro (opcao 1).\n");
            } else {
                exibeEstadoCombate(combate);
                exibeOrdem(combate);
            }
            break;
        case 3:
            if (combate->quantidadeOrdem == 0) {
                printf("Prepare o combate primeiro (opcao 1).\n");
            } else if (!executaRodada(combate)) {
                printf("O combate ja esta encerrado.\n");
                exibeResumoCombate(combate);
            } else if (combate->encerrado) {
                atualizaPersonagensAposCombate(combate);
                exibeResumoCombate(combate);
            }
            break;
        case 4:
            if (combate->quantidadeOrdem == 0) {
                printf("Prepare o combate primeiro (opcao 1).\n");
            } else {
                executaCombate(combate);
                exibeResumoCombate(combate);
            }
            break;
        case 5:
            if (combate->quantidadeOrdem == 0) {
                printf("Nenhum combate preparado no momento.\n");
            } else {
                exibeEstadoCombate(combate);
            }
            break;
        case 6:
            exibeResumoCombate(combate);
            break;
        case 0:
            printf("Voltando ao menu principal.\n");
            break;
        default:
            printf("Opcao invalida.\n");
            break;
        }
    } while (opcao != 0);
}