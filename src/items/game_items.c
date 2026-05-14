#include "game_items.h"
#include <string.h>
#include <stdio.h>

/* =============================================================================
 * ITENS DE CURA (HP) - COMIDAS TÍPICAS
 * =============================================================================
 * Estes itens restauram HP do personagem. A quantidade de cura varia
 * de acordo com a "qualidade" do prato.
 * ============================================================================= */

/* Caldinho de Feijão - Cura básica (30 HP)
 * Um caldo simples, nutritivo e reconfortante */
Item caldinhoFeijao = {
    .name = "Caldinho de Feijao",
    .description = "Caldo nutritivo que restaura 30 HP.",
    .type = ITEM_HEAL,
    .hpRestore = 30,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Tapioca Recheada - Cura média (50 HP)
 * Tapioca com recheio que sustenta bem */
Item tapiocaRecheada = {
    .name = "Tapioca Recheada",
    .description = "Tapioca nutritiva que restaura 50 HP.",
    .type = ITEM_HEAL,
    .hpRestore = 50,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Prato de Buchada - Cura alta (70 HP)
 * Prato típico nordestino, muito nutritivo */
Item pratoBuchada = {
    .name = "Prato de Buchada",
    .description = "Prato robusto que restaura 70 HP.",
    .type = ITEM_HEAL,
    .hpRestore = 70,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Cozido Nordestino - Cura muito alta (100 HP)
 * Prato completo com diversos ingredientes */
Item cozidoNordestino = {
    .name = "Cozido Nordestino",
    .description = "Prato completo que restaura 100 HP.",
    .type = ITEM_HEAL,
    .hpRestore = 100,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Churrascada Completa - Cura máxima (150 HP)
 * Banquete de carnes variadas */
Item churrascadaCompleta = {
    .name = "Churrascada Completa",
    .description = "Banquete que restaura 150 HP.",
    .type = ITEM_HEAL,
    .hpRestore = 150,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* =============================================================================
 * ITENS DE MANA - DOCES E SOBREMESAS
 * =============================================================================
 * Estes itens restauram Mana do personagem, permitindo usar mais
 * habilidades especiais em combate.
 * ============================================================================= */

/* Cocada - Restaura 25 Mana
 * Doce de coco simples e energético */
Item cocada = {
    .name = "Cocada",
    .description = "Doce energetico que restaura 25 Mana.",
    .type = ITEM_MANA,
    .hpRestore = 0,
    .manaRestore = 25,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Canjica - Restaura 35 Mana
 * Sobremesa tradicional de milho */
Item canjica = {
    .name = "Canjica",
    .description = "Sobremesa que restaura 35 Mana.",
    .type = ITEM_MANA,
    .hpRestore = 0,
    .manaRestore = 35,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Broa de Milho - Restaura 45 Mana
 * Doce artesanal com sabor marcante */
Item broaMilho = {
    .name = "Broa de Milho",
    .description = "Broa artesanal que restaura 45 Mana.",
    .type = ITEM_MANA,
    .hpRestore = 0,
    .manaRestore = 45,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Bolo de Rolo - Restaura 60 Mana
 * Doce pernambucano tradicional */
Item boloDeRolo = {
    .name = "Bolo de Rolo",
    .description = "Doce tradicional que restaura 60 Mana.",
    .type = ITEM_MANA,
    .hpRestore = 0,
    .manaRestore = 60,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Pudim de Leite - Restaura 80 Mana
 * Sobremesa completa e energética */
Item pudimLeite = {
    .name = "Pudim de Leite",
    .description = "Sobremesa completa que restaura 80 Mana.",
    .type = ITEM_MANA,
    .hpRestore = 0,
    .manaRestore = 80,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Café com Bolo de Rolo - 20 HP + 10 Mana
 * Lanche leve e revigorante */
Item cafeBoloRolo = {
    .name = "Cafe com Bolo de Rolo",
    .description = "Lanche que restaura 20 HP e 10 Mana.",
    .type = ITEM_HEAL,
    .hpRestore = 20,
    .manaRestore = 10,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Queijo com Goiabada (Romeu e Julieta) - 30 HP + 20 Mana
 * Combinação clássica brasileira */
Item queijoGoiabada = {
    .name = "Queijo com Goiabada",
    .description = "Classico que restaura 30 HP e 20 Mana.",
    .type = ITEM_HEAL,
    .hpRestore = 30,
    .manaRestore = 20,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Cartola - 40 HP + 30 Mana
 * Sobremesa pernambucana com banana, queijo e açúcar */
Item cartola = {
    .name = "Cartola",
    .description = "Sobremesa que restaura 40 HP e 30 Mana.",
    .type = ITEM_HEAL,
    .hpRestore = 40,
    .manaRestore = 30,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Baião de Dois - 60 HP + 40 Mana
 * Prato completo de arroz com feijão */
Item baiaoDeDois = {
    .name = "Baiao de Dois",
    .description = "Refeicao que restaura 60 HP e 40 Mana.",
    .type = ITEM_HEAL,
    .hpRestore = 60,
    .manaRestore = 40,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Festa de São João - Restaura HP e Mana ao máximo
 * Item raro representando um banquete junino */
Item festaDeSaoJoao = {
    .name = "Festa de Sao Joao",
    .description = "Banquete junino que restaura HP e Mana ao maximo.",
    .type = ITEM_HEAL,
    .hpRestore = 9999,  /* Valor alto para garantir cura total */
    .manaRestore = 9999,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* =============================================================================
 * ITENS DE REVIVE - RESSUSCITAM PERSONAGENS DERROTADOS
 * =============================================================================
 * Estes itens só funcionam em personagens com HP = 0.
 * Restauram uma porcentagem do HP máximo ao reviver.
 * ============================================================================= */

/* Erva Cidreira - Revive com 25% HP
 * Erva medicinal básica da região */
Item ervaCidreira = {
    .name = "Erva Cidreira",
    .description = "Revive aliado com 25%% do HP maximo.",
    .type = ITEM_REVIVE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0.25f
};

/* Chá de Camomila - Revive com 50% HP
 * Preparado calmante e revigorante */
Item chaCamomila = {
    .name = "Cha de Camomila",
    .description = "Revive aliado com 50%% do HP maximo.",
    .type = ITEM_REVIVE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0.50f
};

/* Garrafada Nordestina - Revive com 75% HP
 * Mistura tradicional de ervas medicinais */
Item garrafadaNordestina = {
    .name = "Garrafada Nordestina",
    .description = "Revive aliado com 75%% do HP maximo.",
    .type = ITEM_REVIVE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0.75f
};

/* Água Benta - Revive com 100% HP
 * Item raro com poder divino */
Item aguaBenta = {
    .name = "Agua Benta",
    .description = "Revive aliado com 100%% do HP maximo.",
    .type = ITEM_REVIVE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 1.00f
};

/* =============================================================================
 * ITENS DE BUFF - APLICAM CONDIÇÕES POSITIVAS
 * =============================================================================
 * Estes itens aplicam buffs temporários que melhoram os atributos
 * do personagem durante o combate.
 * ============================================================================= */

/* Cachaça Medicinal - Aplica Regeneração por 5 turnos
 * Bebida que acelera a recuperação */
Item cachaçaMedicinal = {
    .name = "Cachaca Medicinal",
    .description = "Aplica Regeneracao por 5 turnos (+10%% HP/turno).",
    .type = ITEM_BUFF,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_REGEN,
    .statusDuration = 5,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Rapadura - Aplica Força Aumentada por 5 turnos
 * Doce energético que aumenta o vigor */
Item rapadura = {
    .name = "Rapadura",
    .description = "Aplica Forca Aumentada por 5 turnos (+50%% dano).",
    .type = ITEM_BUFF,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_STRENGTH_UP,
    .statusDuration = 5,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Cuscuz Reforçado - Aplica Defesa Aumentada por 5 turnos
 * Refeição que fortalece o corpo */
Item cuscuzReforçado = {
    .name = "Cuscuz Reforcado",
    .description = "Aplica Defesa Aumentada por 5 turnos (+50%% def).",
    .type = ITEM_BUFF,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_DEFENSE_UP,
    .statusDuration = 5,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Café Expresso - Aplica Velocidade Aumentada por 5 turnos
 * Estimulante que acelera os reflexos */
Item cafeExpresso = {
    .name = "Cafe Expresso",
    .description = "Aplica Velocidade Aumentada por 5 turnos (+50%% vel).",
    .type = ITEM_BUFF,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_SPEED_UP,
    .statusDuration = 5,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Feijoada - Aplica Regeneração E cura 50 HP
 * Prato completo que dá energia e recuperação */
Item feijoada = {
    .name = "Feijoada Completa",
    .description = "Cura 50 HP e aplica Regeneracao por 3 turnos.",
    .type = ITEM_BUFF,
    .hpRestore = 50,            /* Também cura HP imediatamente */
    .manaRestore = 0,
    .statusToApply = STATUS_REGEN,
    .statusDuration = 3,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* =============================================================================
 * ITENS DE CURA DE STATUS - REMOVEM CONDIÇÕES NEGATIVAS
 * =============================================================================
 * Estes itens curam debuffs específicos ou todos os debuffs de uma vez.
 * São essenciais para combates contra inimigos que aplicam status.
 * ============================================================================= */

/* Antídoto - Cura Envenenamento
 * Preparado que neutraliza venenos */
Item antiveneno = {
    .name = "Antidoto",
    .description = "Cura a condicao de Envenenamento.",
    .type = ITEM_CURE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_POISON,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Pomada Cicatrizante - Cura Queimadura
 * Unguento que alivia queimaduras */
Item pomadaCicatrizante = {
    .name = "Pomada Cicatrizante",
    .description = "Cura a condicao de Queimadura.",
    .type = ITEM_CURE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_BURN,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Massagem Relaxante - Cura Paralisia
 * Técnica que restaura os músculos */
Item massagemRelaxante = {
    .name = "Massagem Relaxante",
    .description = "Cura a condicao de Paralisia.",
    .type = ITEM_CURE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_PARALYSIS,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Estimulante - Cura Sono
 * Substância que desperta imediatamente */
Item estimulante = {
    .name = "Estimulante",
    .description = "Cura a condicao de Sono.",
    .type = ITEM_CURE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_SLEEP,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Água Fria - Cura Confusão
 * Choque que limpa a mente */
Item aguaFria = {
    .name = "Agua Fria",
    .description = "Cura a condicao de Confusao.",
    .type = ITEM_CURE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_CONFUSION,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Colírio Natural - Cura Cegueira
 * Preparado que restaura a visão */
Item colirioNatural = {
    .name = "Colirio Natural",
    .description = "Cura a condicao de Cegueira.",
    .type = ITEM_CURE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_BLIND,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Panaceia Universal - Cura TODOS os debuffs
 * Item raro que remove qualquer condição negativa */
Item panaceiaUniversal = {
    .name = "Panaceia Universal",
    .description = "Cura TODAS as condicoes negativas de uma vez.",
    .type = ITEM_CURE,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,           /* Não especifica um tipo */
    .cureType = CURE_ALL_DEBUFFS,          /* Cura todos os debuffs */
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* =============================================================================
 * ITENS DE STAT BOOST - AUMENTAM ATRIBUTOS PERMANENTEMENTE
 * =============================================================================
 * Estes itens aumentam permanentemente os atributos do personagem.
 * São raros e valiosos, devendo ser usados estrategicamente.
 * ============================================================================= */

/* Vitamina Fortitude - Aumenta HP base em +20
 * Suplemento que fortalece a constituição */
Item vitaminaFortitude = {
    .name = "Vitamina Fortitude",
    .description = "Aumenta HP base permanentemente em +20.",
    .type = ITEM_STAT_BOOST,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 20,
    .reviveHPPercent = 0
};

/* Elixir Mental - Aumenta Mana base em +15
 * Poção que expande a mente */
Item elixirMental = {
    .name = "Elixir Mental",
    .description = "Aumenta Mana base permanentemente em +15.",
    .type = ITEM_STAT_BOOST,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_MANA,
    .statBoostValue = 15,
    .reviveHPPercent = 0
};

/* Proteína Concentrada - Aumenta Força em +3
 * Suplemento que fortalece os músculos */
Item proteínaConcentrada = {
    .name = "Proteina Concentrada",
    .description = "Aumenta Forca permanentemente em +3.",
    .type = ITEM_STAT_BOOST,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_FORCA,
    .statBoostValue = 3,
    .reviveHPPercent = 0
};

/* Mineral Resistente - Aumenta Defesa em +3
 * Substância que endurece o corpo */
Item mineralResistente = {
    .name = "Mineral Resistente",
    .description = "Aumenta Defesa permanentemente em +3.",
    .type = ITEM_STAT_BOOST,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_DEFESA,
    .statBoostValue = 3,
    .reviveHPPercent = 0
};

/* Suplemento de Velocidade - Aumenta Velocidade em +3
 * Fórmula que acelera os reflexos */
Item suplementoVelocidade = {
    .name = "Suplemento Velocidade",
    .description = "Aumenta Velocidade permanentemente em +3.",
    .type = ITEM_STAT_BOOST,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_VELOCIDADE,
    .statBoostValue = 3,
    .reviveHPPercent = 0
};

/* Tônico da Perseverança - Aumenta Fortitude em +2
 * Elixir que fortalece a resistência */
Item tonicoPerseveranca = {
    .name = "Tonico Perseveranca",
    .description = "Aumenta Fortitude permanentemente em +2.",
    .type = ITEM_STAT_BOOST,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_FORTITUDE,
    .statBoostValue = 2,
    .reviveHPPercent = 0
};

/* Chá da Iluminação - Aumenta Mente em +2
 * Bebida que expande a consciência */
Item chaIluminacao = {
    .name = "Cha da Iluminacao",
    .description = "Aumenta Mente permanentemente em +2.",
    .type = ITEM_STAT_BOOST,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_MENTE,
    .statBoostValue = 2,
    .reviveHPPercent = 0
};

/* =============================================================================
 * ITENS DE APLICAR STATUS - USADOS EM COMBATE
 * =============================================================================
 * Estes itens aplicam condições negativas em inimigos durante o combate.
 * São úteis para enfraquecer oponentes fortes.
 * ============================================================================= */

/* Pó de Envenenar - Aplica Envenenamento por 4 turnos
 * Substância tóxica extraída de plantas */
Item poDeEnvenenar = {
    .name = "Po de Envenenar",
    .description = "Aplica Envenenamento no alvo por 4 turnos.",
    .type = ITEM_INFLICT_STATUS,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_POISON,
    .statusDuration = 4,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Pimenta Malagueta - Aplica Queimadura por 3 turnos
 * Pimenta extremamente ardida */
Item pimentaMalagueta = {
    .name = "Pimenta Malagueta",
    .description = "Aplica Queimadura no alvo por 3 turnos.",
    .type = ITEM_INFLICT_STATUS,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_BURN,
    .statusDuration = 3,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Raio Paralisante - Aplica Paralisia por 2 turnos
 * Dispositivo que causa choque */
Item raioParalisante = {
    .name = "Raio Paralisante",
    .description = "Aplica Paralisia no alvo por 2 turnos.",
    .type = ITEM_INFLICT_STATUS,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_PARALYSIS,
    .statusDuration = 2,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Pó do Sono - Aplica Sono por 2 turnos
 * Substância sonífera potente */
Item poDoSono = {
    .name = "Po do Sono",
    .description = "Aplica Sono no alvo por 2 turnos.",
    .type = ITEM_INFLICT_STATUS,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_SLEEP,
    .statusDuration = 2,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Cogumelo Alucinógeno - Aplica Confusão por 3 turnos
 * Fungo que causa desorientação */
Item cogumelo = {
    .name = "Cogumelo",
    .description = "Aplica Confusao no alvo por 3 turnos.",
    .type = ITEM_INFLICT_STATUS,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_CONFUSION,
    .statusDuration = 3,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Bomba de Fumaça - Aplica Cegueira por 3 turnos
 * Artefato que obscurece a visão */
Item bombaDeFumaça = {
    .name = "Bomba de Fumaca",
    .description = "Aplica Cegueira no alvo por 3 turnos.",
    .type = ITEM_INFLICT_STATUS,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_BLIND,
    .statusDuration = 3,
    .statusIntensity = 1.0f,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Pedaço de Chave 1 - Primeira metade de uma chave misteriosa */
Item pedaçoDeChave1 = {
    .name = "Pedaco de Chave 1",
    .description = "Primeira metade de uma chave misteriosa.",
    .type = ITEM_KEY,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

/* Pedaço de Chave 2 - Segunda metade de uma chave misteriosa */
Item pedaçoDeChave2 = {
    .name = "Pedaco de Chave 2",
    .description = "Segunda metade de uma chave misteriosa.",
    .type = ITEM_KEY,
    .hpRestore = 0,
    .manaRestore = 0,
    .statusToApply = STATUS_NONE,
    .statusDuration = 0,
    .statusIntensity = 0,
    .statusToCure = STATUS_NONE,
    .cureType = CURE_SPECIFIC,
    .statBoostType = BOOST_HP,
    .statBoostValue = 0,
    .reviveHPPercent = 0
};

void initAllGameItems(void) {
    /* Os itens já são inicializados estaticamente com designated initializers */
    /* Esta função existe para futuras expansões dinâmicas */
}

/* Array auxiliar para busca de itens por nome */
static Item* allItems[] = {
    /* Itens de Cura HP */
    &caldinhoFeijao,
    &tapiocaRecheada,
    &pratoBuchada,
    &cozidoNordestino,
    &churrascadaCompleta,
    
    /* Itens de Mana */
    &cocada,
    &canjica,
    &broaMilho,
    &boloDeRolo,
    &pudimLeite,
    
    /* Itens Mistos */
    &cafeBoloRolo,
    &queijoGoiabada,
    &cartola,
    &baiaoDeDois,
    &festaDeSaoJoao,
    
    /* Itens de Revive */
    &ervaCidreira,
    &chaCamomila,
    &garrafadaNordestina,
    &aguaBenta,
    
    /* Itens de Buff */
    &cachaçaMedicinal,
    &rapadura,
    &cuscuzReforçado,
    &cafeExpresso,
    &feijoada,
    
    /* Itens de Cura de Status */
    &antiveneno,
    &pomadaCicatrizante,
    &massagemRelaxante,
    &estimulante,
    &aguaFria,
    &colirioNatural,
    &panaceiaUniversal,
    
    /* Itens de Stat Boost */
    &vitaminaFortitude,
    &elixirMental,
    &proteínaConcentrada,
    &mineralResistente,
    &suplementoVelocidade,
    &tonicoPerseveranca,
    &chaIluminacao,
    
    /* Itens de Aplicar Status */
    &poDeEnvenenar,
    &pimentaMalagueta,
    &raioParalisante,
    &poDoSono,
    &cogumelo,
    &bombaDeFumaça,
    
    NULL
};

Item* getItemByName(const char* name) {
    if (name == NULL) return NULL;
    
    int i = 0;
    while (allItems[i] != NULL) {
        if (strcmp(allItems[i]->name, name) == 0) {
            return allItems[i];
        }
        i++;
    }
    
    return NULL;
}

/* Arrays estáticos para retorno das funções de categoria */
static Item* healItems[] = {
    &caldinhoFeijao,
    &tapiocaRecheada,
    &pratoBuchada,
    &cozidoNordestino,
    &churrascadaCompleta,
    &cafeBoloRolo,
    &queijoGoiabada,
    &cartola,
    &baiaoDeDois,
    &festaDeSaoJoao,
    NULL
};

static Item* buffItems[] = {
    &cachaçaMedicinal,
    &rapadura,
    &cuscuzReforçado,
    &cafeExpresso,
    &feijoada,
    NULL
};

/**
 * getAllHealItems - Retorna array de ponteiros para itens de cura
 */
Item** getAllHealItems(int* count) {
    if (count != NULL) {
        *count = 10;  /* Número de itens de cura */
    }
    return healItems;
}

/**
 * getAllBuffItems - Retorna array de ponteiros para itens de buff
 */
Item** getAllBuffItems(int* count) {
    if (count != NULL) {
        *count = 5;  /* Número de itens de buff */
    }
    return buffItems;
}
