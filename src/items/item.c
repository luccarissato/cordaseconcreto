/**
 * =============================================================================
 * ITEM.C - Implementação do Sistema de Itens
 * =============================================================================
 * 
 * Este arquivo contém a implementação de todas as funções relacionadas
 * ao uso e gerenciamento de itens no jogo.
 * 
 * O sistema suporta múltiplos tipos de itens:
 * - Itens de cura (HP e Mana)
 * - Itens de buff (aplicam condições positivas)
 * - Itens de cura de status (removem condições negativas)
 * - Itens de revive (ressuscitam personagens derrotados)
 * - Itens de stat boost (aumentam atributos permanentemente)
 * - Itens de aplicar status (para uso em combate)
 * 
 * =============================================================================
 */

#include "item.h"
#include "inventory.h"
#include <stdio.h>
#include <stdlib.h>

#include "../entities/player.h"
#include "../entities/stats.h"
#include "../entities/status_condition.h"
#include "../utils/list.h"

/* =============================================================================
 * FUNÇÕES AUXILIARES (ESTÁTICAS)
 * ============================================================================= */

/**
 * clampHP - Garante que o HP está dentro dos limites válidos
 * @param stats: Ponteiro para as estatísticas do personagem
 * 
 * Ajusta currentHP para estar entre 0 e maxHP.
 */
static void clampHP(Stats* stats) {
    if (stats->currentHP > stats->maxHP) {
        stats->currentHP = stats->maxHP;
    }
    if (stats->currentHP < 0) {
        stats->currentHP = 0;
    }
}

/**
 * clampMana - Garante que a Mana está dentro dos limites válidos
 * @param stats: Ponteiro para as estatísticas do personagem
 * 
 * Ajusta currentMana para estar entre 0 e maxMana.
 */
static void clampMana(Stats* stats) {
    if (stats->currentMana > stats->maxMana) {
        stats->currentMana = stats->maxMana;
    }
    if (stats->currentMana < 0) {
        stats->currentMana = 0;
    }
}

/* =============================================================================
 * FUNÇÕES PRINCIPAIS DE USO DE ITENS
 * ============================================================================= */

/**
 * useItem - Aplica os efeitos de um item em um jogador
 * 
 * Esta função é o ponto central do sistema de itens. Ela identifica o tipo
 * do item e chama as funções apropriadas para aplicar seus efeitos.
 * 
 * Fluxo de processamento por tipo:
 * 1. ITEM_HEAL: Adiciona hpRestore ao HP atual
 * 2. ITEM_MANA: Adiciona manaRestore à Mana atual
 * 3. ITEM_BUFF: Aplica condição de status positiva via statusList
 * 4. ITEM_CURE: Remove condições negativas via statusList
 * 5. ITEM_REVIVE: Restaura HP de personagem derrotado
 * 6. ITEM_STAT_BOOST: Aumenta atributo permanentemente
 * 7. ITEM_INFLICT_STATUS: Aplica condição negativa (uso em combate)
 */
void useItem(Player* target, InventoryItem* item) {
    /* Validação de parâmetros */
    if (target == NULL || item == NULL || item->baseItem == NULL) {
        return;
    }
    
    Item* base = item->baseItem;
    
    switch (base->type) {
        case ITEM_HEAL: {
            /* -----------------------------------------------------------------
             * ITEM_HEAL: Restaura HP do personagem
             * -----------------------------------------------------------------
             * Adiciona o valor de hpRestore ao HP atual.
             * Também pode restaurar um pouco de Mana se manaRestore > 0.
             * O HP é limitado ao máximo após a restauração.
             * --------------------------------------------------------------- */
            target->stats.currentHP += base->hpRestore;
            target->stats.currentMana += base->manaRestore;
            
            /* Garante que os valores não ultrapassem os máximos */
            clampHP(&target->stats);
            clampMana(&target->stats);
            break;
        }
        
        case ITEM_MANA: {
            /* -----------------------------------------------------------------
             * ITEM_MANA: Restaura Mana do personagem
             * -----------------------------------------------------------------
             * Adiciona o valor de manaRestore à Mana atual.
             * Pode também restaurar um pouco de HP se hpRestore > 0.
             * --------------------------------------------------------------- */
            target->stats.currentHP += base->hpRestore;
            target->stats.currentMana += base->manaRestore;
            
            clampHP(&target->stats);
            clampMana(&target->stats);
            break;
        }
        
        case ITEM_BUFF: {
            /* -----------------------------------------------------------------
             * ITEM_BUFF: Aplica condição de status positiva
             * -----------------------------------------------------------------
             * Adiciona uma condição de status (buff) ao personagem.
             * Usa a StatusList do jogador para gerenciar a condição.
             * Se a condição já existir, a duração é estendida.
             * --------------------------------------------------------------- */
            addStatusCondition(
                &target->statusList,
                base->statusToApply,
                base->statusDuration,
                base->statusIntensity
            );
            break;
        }
        
        case ITEM_CURE: {
            /* -----------------------------------------------------------------
             * ITEM_CURE: Remove condições de status negativas
             * -----------------------------------------------------------------
             * Remove debuffs do personagem de acordo com o cureType:
             * - CURE_SPECIFIC: Remove apenas a condição especificada
             * - CURE_ALL_DEBUFFS: Remove todas as condições negativas
             * - CURE_ALL: Remove todas as condições (buffs e debuffs)
             * --------------------------------------------------------------- */
            switch (base->cureType) {
                case CURE_SPECIFIC:
                    /* Remove apenas a condição específica indicada */
                    removeStatusCondition(&target->statusList, base->statusToCure);
                    break;
                    
                case CURE_ALL_DEBUFFS:
                    /* Remove todas as condições negativas, mantém buffs */
                    removeAllDebuffs(&target->statusList);
                    break;
                    
                case CURE_ALL:
                    /* Remove absolutamente todas as condições */
                    clearAllStatus(&target->statusList);
                    break;
            }
            break;
        }
        
        case ITEM_REVIVE: {
            /* -----------------------------------------------------------------
             * ITEM_REVIVE: Revive personagem derrotado
             * -----------------------------------------------------------------
             * Só funciona se o personagem estiver com HP = 0.
             * Restaura uma porcentagem do HP máximo definida pelo item.
             * Também pode restaurar um pouco de Mana.
             * --------------------------------------------------------------- */
            if (target->stats.currentHP <= 0) {
                /* Calcula HP a restaurar baseado na porcentagem */
                int hpToRestore = (int)(target->stats.maxHP * base->reviveHPPercent);
                if (hpToRestore < 1) hpToRestore = 1; /* Mínimo 1 HP */
                
                target->stats.currentHP = hpToRestore;
                
                /* Restaura mana adicional se especificado */
                target->stats.currentMana += base->manaRestore;
                
                clampHP(&target->stats);
                clampMana(&target->stats);
                
                /* Remove condições de status ao reviver (começa limpo) */
                clearAllStatus(&target->statusList);
            }
            break;
        }
        
        case ITEM_STAT_BOOST: {
            /* -----------------------------------------------------------------
             * ITEM_STAT_BOOST: Aumenta atributo permanentemente
             * -----------------------------------------------------------------
             * Adiciona um valor permanente ao atributo especificado.
             * Após a mudança, recalcula os stats derivados (maxHP, maxMana).
             * --------------------------------------------------------------- */
            applyStatBoost(target, base);
            break;
        }
        
        case ITEM_INFLICT_STATUS: {
            /* -----------------------------------------------------------------
             * ITEM_INFLICT_STATUS: Aplica condição de status
             * -----------------------------------------------------------------
             * Similar a ITEM_BUFF, mas geralmente usado para aplicar
             * condições negativas em inimigos durante combate.
             * No contexto de uso em aliado, funciona como buff.
             * --------------------------------------------------------------- */
            addStatusCondition(
                &target->statusList,
                base->statusToApply,
                base->statusDuration,
                base->statusIntensity
            );
            break;
        }
    }
}

/**
 * consumeItem - Usa um item e decrementa sua quantidade no inventário
 * 
 * Esta função combina o uso do item com o gerenciamento do inventário.
 * Após aplicar os efeitos do item, decrementa a quantidade.
 * Se a quantidade chegar a zero, o item é removido do inventário.
 * 
 * A remoção utiliza a estrutura de Lista Encadeada, demonstrando
 * o uso da estrutura de dados na lógica do jogo.
 */
void consumeItem(Inventory* inv, ListNode* node, Player* target) {
    /* Validação de parâmetros */
    if (node == NULL || inv == NULL || target == NULL) {
        return;
    }
    
    InventoryItem* invItem = (InventoryItem*) node->data;
    
    if (invItem == NULL) {
        return;
    }
    
    /* Itens-chave não podem ser consumidos */
    if (invItem->baseItem->type == ITEM_KEY) {
        return;
    }
    
    /* Aplica os efeitos do item no jogador */
    useItem(target, invItem);
    
    /* Decrementa a quantidade no inventário */
    invItem->quantity--;
    
    /* Se acabou o estoque, remove o item do inventário */
    if (invItem->quantity <= 0) {
        /* Remove o nó da lista encadeada */
        removeNode(&inv->items, node);
        
        /* Libera a memória do InventoryItem */
        free(invItem);
    }
}

/**
 * canUseItem - Verifica se um item pode ser usado no alvo
 * 
 * Retorna 1 se o item pode ser usado, 0 caso contrário.
 * As verificações dependem do tipo do item:
 * - ITEM_REVIVE: Só pode usar se HP = 0 (personagem derrotado)
 * - ITEM_HEAL: Só pode usar se HP < maxHP (não está cheio)
 * - ITEM_MANA: Só pode usar se Mana < maxMana
 * - Outros tipos: Sempre podem ser usados
 */
int canUseItem(Player* target, InventoryItem* item) {
    if (target == NULL || item == NULL || item->baseItem == NULL) {
        return 0;
    }
    
    Item* base = item->baseItem;
    
    switch (base->type) {
        case ITEM_REVIVE:
            /* Item de revive só funciona em personagem derrotado */
            return (target->stats.currentHP <= 0);
            
        case ITEM_HEAL:
            /* Item de cura só faz sentido se HP não está cheio */
            /* Mas permitimos usar mesmo cheio (alguns itens curam status também) */
            return 1;
            
        case ITEM_MANA:
            /* Item de mana só faz sentido se mana não está cheia */
            return 1;
            
        case ITEM_CURE:
            /* Item de cura de status: verifica se há debuffs ativos */
            if (base->cureType == CURE_SPECIFIC) {
                return hasStatusCondition(&target->statusList, base->statusToCure);
            }
            /* Para cura geral, permite usar (pode não ter efeito) */
            return 1;
            
        default:
            /* Outros tipos sempre podem ser usados */
            return 1;
    }
}

/**
 * useItemOnStatusList - Aplica efeitos de item em uma StatusList
 * 
 * Versão simplificada de useItem que só afeta condições de status.
 * Útil quando não se tem acesso completo ao Player.
 */
void useItemOnStatusList(StatusList* statusList, Item* item) {
    if (statusList == NULL || item == NULL) {
        return;
    }
    
    switch (item->type) {
        case ITEM_BUFF:
        case ITEM_INFLICT_STATUS:
            addStatusCondition(
                statusList,
                item->statusToApply,
                item->statusDuration,
                item->statusIntensity
            );
            break;
            
        case ITEM_CURE:
            switch (item->cureType) {
                case CURE_SPECIFIC:
                    removeStatusCondition(statusList, item->statusToCure);
                    break;
                case CURE_ALL_DEBUFFS:
                    removeAllDebuffs(statusList);
                    break;
                case CURE_ALL:
                    clearAllStatus(statusList);
                    break;
            }
            break;
            
        default:
            /* Outros tipos de item não afetam apenas StatusList */
            break;
    }
}

/**
 * applyStatBoost - Aplica aumento permanente de atributo
 * 
 * Aumenta permanentemente o atributo especificado pelo item.
 * Após a modificação, recalcula os stats derivados (maxHP, maxMana)
 * para refletir a mudança.
 */
void applyStatBoost(Player* target, Item* item) {
    if (target == NULL || item == NULL) {
        return;
    }
    
    /* Aplica o boost no atributo especificado */
    switch (item->statBoostType) {
        case BOOST_HP:
            /* Aumenta HP base, que afeta maxHP */
            target->stats.baseHP += item->statBoostValue;
            break;
            
        case BOOST_MANA:
            /* Aumenta Mana base, que afeta maxMana */
            target->stats.baseMana += item->statBoostValue;
            break;
            
        case BOOST_FORTITUDE:
            /* Aumenta Fortitude (afeta maxHP indiretamente) */
            target->stats.fortitude += item->statBoostValue;
            break;
            
        case BOOST_MENTE:
            /* Aumenta Mente (afeta maxMana indiretamente) */
            target->stats.mente += item->statBoostValue;
            break;
            
        case BOOST_FORCA:
            /* Aumenta Força (dano físico) */
            target->stats.forca += item->statBoostValue;
            break;
            
        case BOOST_DEFESA:
            /* Aumenta Defesa (redução de dano) */
            target->stats.defesa += item->statBoostValue;
            break;
            
        case BOOST_VELOCIDADE:
            /* Aumenta Velocidade (ordem de turno) */
            target->stats.velocidade += item->statBoostValue;
            break;
    }
    
    /* Recalcula os stats derivados (maxHP, maxMana) */
    /* Guarda os valores atuais de HP/Mana para não resetar */
    int currentHP = target->stats.currentHP;
    int currentMana = target->stats.currentMana;
    
    calculateStats(&target->stats);
    
    /* Restaura os valores atuais, mas limita aos novos máximos */
    /* O aumento de maxHP/maxMana não cura automaticamente */
    target->stats.currentHP = currentHP;
    target->stats.currentMana = currentMana;
    
    /* Se os máximos aumentaram, permite manter o valor atual */
    /* Se ultrapassar o novo máximo, ajusta */
    clampHP(&target->stats);
    clampMana(&target->stats);
}

/**
 * revivePlayer - Revive um jogador com HP = 0
 * 
 * Restaura HP de um personagem derrotado baseado em uma porcentagem
 * do HP máximo. Retorna 1 se reviveu com sucesso, 0 se o personagem
 * não precisava ser revivido (HP > 0).
 */
int revivePlayer(Player* target, float hpPercent) {
    if (target == NULL) {
        return 0;
    }
    
    /* Só revive se o personagem está derrotado */
    if (target->stats.currentHP > 0) {
        return 0; /* Não precisa reviver */
    }
    
    /* Valida a porcentagem (entre 0.1 e 1.0) */
    if (hpPercent < 0.1f) hpPercent = 0.1f;
    if (hpPercent > 1.0f) hpPercent = 1.0f;
    
    /* Calcula HP a restaurar */
    int hpToRestore = (int)(target->stats.maxHP * hpPercent);
    if (hpToRestore < 1) hpToRestore = 1;
    
    target->stats.currentHP = hpToRestore;
    
    /* Limpa condições de status ao reviver */
    clearAllStatus(&target->statusList);
    
    return 1; /* Revivido com sucesso */
}
