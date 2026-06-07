#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cache.h"
#include "utlist.h"

// GLOBAL VARIABLES
static entry_node *cache_map = NULL;   // Hash Table for cache
static entry_node *stack_s = NULL;     // Circular DL-List as Stack S
static entry_node *list_q = NULL;      // Custom DL-List as FIFO queue Q
static int max_capacity = 1000;
static int current_size = 0;           // Tracks ONLY items physically in cache (LIR + not evicted HIR)
// LIRS reserves a large percentage of cache for LIRs 
static int lir_capacity = 0;
static int current_lir_size = 0;

// LIST_Q HELPERS (O(1) complexity)
void q_prepend(entry_node *node){
    if (list_q == NULL){
        list_q = node;
        node->meta.lirs.q_next = node;
        node->meta.lirs.q_prev = node;
    } 
    else{
        node->meta.lirs.q_next = list_q;
        node->meta.lirs.q_prev = list_q->meta.lirs.q_prev;
        list_q->meta.lirs.q_prev->meta.lirs.q_next = node;
        list_q->meta.lirs.q_prev = node;
        list_q = node; // Make it the new head
    }
}

void q_remove(entry_node *node){
    if (node->meta.lirs.q_next == node){
        list_q = NULL; // the only element case
    }
    else{
        node->meta.lirs.q_prev->meta.lirs.q_next = node->meta.lirs.q_next;
        node->meta.lirs.q_next->meta.lirs.q_prev = node->meta.lirs.q_prev;
        if (list_q == node){
            list_q = node->meta.lirs.q_next;
        }
    }
    node->meta.lirs.q_prev = NULL;
    node->meta.lirs.q_next = NULL;
}

// LIRS HELPERS
void evict_q(){
    if (list_q == NULL) return;
    
    // Tail is head->q_prev
    entry_node *eviction_node = list_q->meta.lirs.q_prev; 
    
    q_remove(eviction_node);
    
    // Is this node still in Stack S
    if (eviction_node->next != NULL) {
        // Not in Stack_S Update STATUS to Ghost and keep it in the Hash Map.
        eviction_node->meta.lirs.status = GHOST_HIR;
    } else {
        // It was already pruned from Stack S earlier.
        //Remove no longer used
        HASH_DEL(cache_map, eviction_node);
        free(eviction_node);
    }
    
    current_size--;
}

void stack_prune(){
    if (stack_s == NULL) return;
    
    while (stack_s != NULL && stack_s->prev->meta.lirs.status != STATUS_LIR) {
        entry_node *rem_node = stack_s->prev;
        
        CDL_DELETE(stack_s, rem_node);
        
        // init
        rem_node->next = NULL;
        rem_node->prev = NULL;
        
        if (rem_node->meta.lirs.status == GHOST_HIR) {
            HASH_DEL(cache_map, rem_node);
            free(rem_node);
        }
    }
}

// LIRS eviction policy
void init_lirs_cache(int capacity){
    max_capacity = capacity;
    lir_capacity = (int)(capacity * 0.99); // 99% dedicated to LIRs
    if (lir_capacity == 0) lir_capacity = 1;
}

int process_lirs_request(uint64_t requested_id, uint32_t size){
    entry_node *target_node = NULL;
    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL){
        // >CACHE HIT<
        if (target_node->meta.lirs.status == STATUS_LIR){
            // Hit Case: LIR
            CDL_DELETE(stack_s, target_node);
            CDL_PREPEND(stack_s, target_node);
            stack_prune();
            return 1;
            
        } else if (target_node->meta.lirs.status == STATUS_HIR){
            // Hit Case: non evicted HIR
            
            if (target_node->next != NULL) { 
                CDL_DELETE(stack_s, target_node);
            }
            
            q_remove(target_node);
            
            target_node->meta.lirs.status = STATUS_LIR;
            CDL_PREPEND(stack_s, target_node);
            
            // Demote the bottom LIR to HIR
            if (stack_s != NULL && stack_s->prev != NULL){
                entry_node *bottom_lir = stack_s->prev;
                bottom_lir->meta.lirs.status = STATUS_HIR;
                
                CDL_DELETE(stack_s, bottom_lir);
                bottom_lir->next = NULL; 
                bottom_lir->prev = NULL;
                
                q_prepend(bottom_lir);
            }
            
            stack_prune();
            return 1;
            
        } else if (target_node->meta.lirs.status == GHOST_HIR){
            // Miss Case: Evicted HIR that was logged (History Hit)
            evict_q();
            
            // Only delete if in Stack S 
            if (target_node->next != NULL) {
                CDL_DELETE(stack_s, target_node);
            }
            
            target_node->meta.lirs.status = STATUS_LIR;
            CDL_PREPEND(stack_s, target_node);
            current_size++; // return to physical memory
            
            // Demote the bottom LIR to HIR
            if (stack_s != NULL && stack_s->prev != NULL){
                entry_node *bottom_lir = stack_s->prev;
                bottom_lir->meta.lirs.status = STATUS_HIR;
                
                CDL_DELETE(stack_s, bottom_lir);
                
                bottom_lir->next = NULL; 
                bottom_lir->prev = NULL;
                
                q_prepend(bottom_lir);
            }
            
            stack_prune();
            return 0; // Miss
        }
        
    } 
    else{
        // >CACHE MISS<  
        // (New Item)                        
        if (current_size >= max_capacity){
            evict_q();
        }

        entry_node *new_node = (entry_node *)malloc(sizeof(entry_node));
        
        //Init 
        new_node->next = NULL;
        new_node->prev = NULL;
        new_node->meta.lirs.q_next = NULL;
        new_node->meta.lirs.q_prev = NULL;
        
        new_node->obj_id = requested_id;
        new_node->obj_size = size;
        
        // Initial filling phase: prioritize filling LIR capacity first
        if (current_lir_size < lir_capacity){
            new_node->meta.lirs.status = STATUS_LIR;
            CDL_PREPEND(stack_s, new_node);
            current_lir_size++;
        } 
        else{
            new_node->meta.lirs.status = STATUS_HIR;
            CDL_PREPEND(stack_s, new_node);
            q_prepend(new_node);
        }

        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);
        current_size++;
        
        return 0; // Miss
    }
    
    return 0;
}

//Free() Helper
void free_lirs_cache(){
    entry_node *current_node, *tmp;
    
    HASH_ITER(hh, cache_map, current_node, tmp){
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    
    stack_s = NULL;
    list_q = NULL;
    current_size = 0;
    current_lir_size = 0;
}