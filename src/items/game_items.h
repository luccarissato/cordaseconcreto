/**
 * =============================================================================
 * GAME_ITEMS.H - Definições de Todos os Itens do Jogo
 * =============================================================================
 * 
 * Este arquivo declara todos os itens disponíveis no jogo.
 * Os itens são organizados por categoria para facilitar a manutenção.
 * 
 * CATEGORIAS:
 * 1. Itens de Cura (HP) - Comidas típicas do Nordeste
 * 2. Itens de Mana - Doces e sobremesas
 * 3. Itens Mistos (HP + Mana) - Combinações especiais
 * 4. Itens de Revive - Para ressuscitar personagens derrotados
 * 5. Itens de Buff - Aplicam condições positivas temporárias
 * 6. Itens de Cura de Status - Removem condições negativas
 * 7. Itens de Stat Boost - Aumentam atributos permanentemente
 * 8. Itens de Aplicar Status - Usados em combate
 * 
 * =============================================================================
 */

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

/* =============================================================================
 * FUNÇÕES DE INICIALIZAÇÃO
 * ============================================================================= */

/**
 * initAllGameItems - Inicializa todos os itens do jogo
 * 
 * Deve ser chamada uma vez no início do jogo para configurar
 * corretamente todos os itens com seus valores padrão.
 * Os itens são variáveis globais para fácil acesso.
 */
void initAllGameItems(void);

/**
 * getItemByName - Busca um item pelo nome
 * @param name: Nome do item a buscar
 * @return: Ponteiro para o Item ou NULL se não encontrado
 * 
 * Utiliza busca linear na lista de itens.
 * Útil para sistemas de loot e loja.
 */
Item* getItemByName(const char* name);

/**
 * getAllHealItems - Retorna array de ponteiros para itens de cura
 * @param count: Ponteiro para armazenar a quantidade de itens
 * @return: Array de ponteiros Item*
 */
Item** getAllHealItems(int* count);

/**
 * getAllBuffItems - Retorna array de ponteiros para itens de buff
 * @param count: Ponteiro para armazenar a quantidade de itens
 * @return: Array de ponteiros Item*
 */
Item** getAllBuffItems(int* count);

#endif /* GAME_ITEMS_H */
