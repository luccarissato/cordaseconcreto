#ifndef GAME_ITEMS_H
#define GAME_ITEMS_H

#include "item.h"

/* =============================================================================
 * ITENS DE CURA (HP) - COMIDAS TÍPICAS
 * ============================================================================= */

/* Cura básica (30 HP) - Caldo simples e nutritivo */
extern Item caldinhoFeijao;

/* Cura média (50 HP) - Tapioca recheada */
extern Item tapiocaRecheada;

/* Cura alta (70 HP) - Prato completo de buchada */
extern Item pratoBuchada;

/* Cura muito alta (100 HP) - Prato especial */
extern Item cozidoNordestino;

/* Cura máxima (150 HP) - Banquete completo */
extern Item churrascadaCompleta;

/* =============================================================================
 * ITENS DE MANA - DOCES E SOBREMESAS
 * ============================================================================= */

/* Restaura 25 Mana - Doce simples */
extern Item cocada;

/* Restaura 35 Mana - Sobremesa tradicional */
extern Item canjica;

/* Restaura 45 Mana - Doce artesanal */
extern Item broaMilho;

/* Restaura 60 Mana - Doce especial */
extern Item boloDeRolo;

/* Restaura 80 Mana - Sobremesa completa */
extern Item pudimLeite;

/* =============================================================================
 * ITENS MISTOS (HP + MANA) - COMBINAÇÕES ESPECIAIS
 * ============================================================================= */

/* Cura 20 HP + 10 Mana - Lanche leve */
extern Item cafeBoloRolo;

/* Cura 30 HP + 20 Mana - Combinação clássica */
extern Item queijoGoiabada;

/* Cura 40 HP + 30 Mana - Sobremesa pernambucana */
extern Item cartola;

/* Cura 60 HP + 40 Mana - Refeição completa */
extern Item baiaoDeDois;

/* Cura HP e Mana ao máximo - Item raro */
extern Item festaDeSaoJoao;

/* =============================================================================
 * ITENS DE REVIVE - RESSUSCITAM PERSONAGENS DERROTADOS
 * ============================================================================= */

/* Revive com 25% HP - Erva medicinal básica */
extern Item ervaCidreira;

/* Revive com 50% HP - Mistura de ervas */
extern Item chaCamomila;

/* Revive com 75% HP - Preparado especial */
extern Item garrafadaNordestina;

/* Revive com 100% HP - Item raro e poderoso */
extern Item aguaBenta;

/* =============================================================================
 * ITENS DE BUFF - APLICAM CONDIÇÕES POSITIVAS
 * ============================================================================= */

/* Aplica Regeneração por 5 turnos - Recupera HP gradualmente */
extern Item cachaçaMedicinal;

/* Aplica Força Aumentada por 5 turnos - +50% dano */
extern Item rapadura;

/* Aplica Defesa Aumentada por 5 turnos - +50% defesa */
extern Item cuscuzReforçado;

/* Aplica Velocidade Aumentada por 5 turnos - +50% velocidade */
extern Item cafeExpresso;

/* Aplica Regeneração + Força por 3 turnos - Combo */
extern Item feijoada;

/* =============================================================================
 * ITENS DE CURA DE STATUS - REMOVEM CONDIÇÕES NEGATIVAS
 * ============================================================================= */

/* Cura Envenenamento */
extern Item antiveneno;

/* Cura Queimadura */
extern Item pomadaCicatrizante;

/* Cura Paralisia */
extern Item massagemRelaxante;

/* Cura Sono */
extern Item estimulante;

/* Cura Confusão */
extern Item aguaFria;

/* Cura Cegueira */
extern Item colirioNatural;

/* Cura todos os debuffs de uma vez */
extern Item panaceiaUniversal;

/* =============================================================================
 * ITENS DE STAT BOOST - AUMENTAM ATRIBUTOS PERMANENTEMENTE
 * ============================================================================= */

/* Aumenta HP base em +20 */
extern Item vitaminaFortitude;

/* Aumenta Mana base em +15 */
extern Item elixirMental;

/* Aumenta Força em +3 */
extern Item proteínaConcentrada;

/* Aumenta Defesa em +3 */
extern Item mineralResistente;

/* Aumenta Velocidade em +3 */
extern Item suplementoVelocidade;

/* Aumenta Fortitude em +2 - Afeta HP máximo */
extern Item tonicoPerseveranca;

/* Aumenta Mente em +2 - Afeta Mana máxima */
extern Item chaIluminacao;

/* =============================================================================
 * ITENS DE APLICAR STATUS - USADOS EM COMBATE
 * ============================================================================= */

/* Aplica Envenenamento no alvo por 4 turnos */
extern Item poDeEnvenenar;

/* Aplica Queimadura no alvo por 3 turnos */
extern Item pimentaMalagueta;

/* Aplica Paralisia no alvo por 2 turnos */
extern Item raioParalisante;

/* Aplica Sono no alvo por 2 turnos */
extern Item poDoSono;

/* Aplica Confusão no alvo por 3 turnos */
extern Item cogumelo;

/* Aplica Cegueira no alvo por 3 turnos */
extern Item bombaDeFumaça;

//itens nao consumiveis
/* Primeira metade de uma chave misteriosa */
extern Item pedaçoDeChave1;

/* Segunda metade de uma chave misteriosa */
extern Item pedaçoDeChave2;

/* =============================================================================
 * ITENS DE CHAVE / QUEST
 * =============================================================================
 */
/* Moeda do cais - item de quest */
extern Item moedaDoCais;

/* Coração de Barro - item de quest */
extern Item coracaoDeBarro;

/* Casca do Mangue - item de quest */
extern Item cascaDoMangue;

// initAllGameItems - Inicializa todos os itens do jogo
void initAllGameItems(void);

// getItemByName - Busca um item pelo nome
Item* getItemByName(const char* name);

//getAllHealItems - Retorna array de ponteiros para itens de cura
Item** getAllHealItems(int* count);

// getAllBuffItems - Retorna array de ponteiros para itens de buff
Item** getAllBuffItems(int* count);

#endif
