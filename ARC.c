#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cache.h"
#include "utlist.h"

// Macros for dynamic adaptation calculations
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// GLOBAL VARIABLES
static entry_node *cache_map = NULL;   //Hash Table for cache
static entry_node *t1_head = NULL;     // T1: Recent entries (Physical Cache)
static entry_node *t2_head = NULL;     // T2: Frequent entries (Physical Cache)
static entry_node *b1_head = NULL;     // B1: Ghost list for T1 (History)
static entry_node *b2_head = NULL;     // B2: Ghost list for T2 (History)

static int size_t1 = 0;
static int size_t2 = 0;
static int size_b1 = 0;
static int size_b2 = 0;
static int p = 0;                      // Target capacity for T1
static int max_capacity = 1000;        // Total physical capacity (c)


// ARC REPLACEMENT (O(1) Complexity)
// Main funct of ARC. Balances physical cache space between T1 and T2
// based on the continuously adjusting target parameter 'p'.
void replace(int hit_in_b2){
    // If T1 has items, and it exceeds target 'p' (or ties with 'p' on a B2 hit)
    if (size_t1 > 0 && (size_t1 > p || (hit_in_b2 && size_t1 == p))) {
        
        // Evict the LRU item from T1 to the MRU of B1
        entry_node *lru_t1 = t1_head->prev;
        
        CDL_DELETE(t1_head, lru_t1);
        size_t1--;
        
        lru_t1->meta.status = LIST_B1;
        CDL_PREPEND(b1_head, lru_t1);
        size_b1++;
        
    } 
    else{
        
        // Evict the LRU item from T2 to the MRU of B2
        entry_node *lru_t2 = t2_head->prev;
        
        CDL_DELETE(t2_head, lru_t2);
        size_t2--;
        
        lru_t2->meta.status = LIST_B2;
        CDL_PREPEND(b2_head, lru_t2);
        size_b2++;
    }
}

//ARC Helpers
void init_arc_cache(int capacity){
    max_capacity = capacity;
    p = 0; 
}

int process_arc_request(uint64_t requested_id, uint32_t size){
    entry_node *target_node = NULL;
    
    // O(1) Lookup
    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL){
        
        // -CACHE HIT: Found in Physical Cache (T1 or T2)-
        if (target_node->meta.status == LIST_T1 || target_node->meta.status == LIST_T2){
            
            // Remove from current physical list
            if (target_node->meta.status == LIST_T1){
                CDL_DELETE(t1_head, target_node);
                size_t1--;
            } 
            else{
                CDL_DELETE(t2_head, target_node);
                size_t2--;
            }
            
            // Promote to MRU of T2
            target_node->meta.status = LIST_T2;
            CDL_PREPEND(t2_head, target_node);
            size_t2++;
            
            return 1; // Physical Hit
        }
        
        // -GHOST HIT: Found in History (B1 or B2)-
        else if (target_node->meta.status == LIST_B1){
            
            // Hit in B1 means Recency takes priority.
            // Increase the target capacity (p) for T1.
            int delta = (size_b1 >= size_b2) ? 1 : (size_b2 / size_b1);
            p = MIN(max_capacity, p + delta);
            
            replace(0); // Make physical space
            
            // Move item from B1 to Physical Cache T2
            CDL_DELETE(b1_head, target_node);
            size_b1--;
            
            target_node->meta.status = LIST_T2;
            CDL_PREPEND(t2_head, target_node);
            size_t2++;
            
            return 0; // Miss (Ghost, data must be fetched)
        }
        
        else if (target_node->meta.status == LIST_B2){
            
            // Hit in B2 means Frequency takes priority
            // Decrease the target capacity (p) for T1.
            int delta = (size_b2 >= size_b1) ? 1 : (size_b1 / size_b2);
            p = MAX(0, p - delta);
            
            replace(1); // Make physical space (Pass 1 since hit was in B2)
            
            // Move item from B2 to Physical Cache T2
            CDL_DELETE(b2_head, target_node);
            size_b2--;
            
            target_node->meta.status = LIST_T2;
            CDL_PREPEND(t2_head, target_node);
            size_t2++;
            
            return 0; // Miss (Ghost so not in physical mem)
        }
        
    } else {
        // -CACHE MISS- 
        //(New Item)
        // Case A: L1 (T1 + B1) holds exactly 'c' elements
        if (size_t1 + size_b1 == max_capacity){
            if (size_t1 < max_capacity) {
                // Delete LRU ghost from B1 completely
                entry_node *evicted_b1 = b1_head->prev;
                CDL_DELETE(b1_head, evicted_b1);
                size_b1--;
                HASH_DEL(cache_map, evicted_b1);
                free(evicted_b1);
                
                replace(0);
            } else {
                // T1 is full, no ghosts. Delete LRU from T1.
                entry_node *evicted_t1 = t1_head->prev;
                CDL_DELETE(t1_head, evicted_t1);
                size_t1--;
                HASH_DEL(cache_map, evicted_t1);
                free(evicted_t1);
            }
        } 
        //L1 (T1 + B1) has fewer than 'c' elements
        else if (size_t1 + size_b1 < max_capacity){
            int total_cache_size = size_t1 + size_t2 + size_b1 + size_b2;
            
            if (total_cache_size >= max_capacity){
                if (total_cache_size == 2 * max_capacity){
                    // Maximum history size reached. Delete LRU from B2.
                    entry_node *evicted_b2 = b2_head->prev;
                    CDL_DELETE(b2_head, evicted_b2);
                    size_b2--;
                    HASH_DEL(cache_map, evicted_b2);
                    free(evicted_b2);
                }
                replace(0);
            }
        }
        
        // Create the new item and add it to T1
        entry_node *new_node = (entry_node *)malloc(sizeof(entry_node));
        if (new_node == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }
        
        new_node->obj_id = requested_id;
        new_node->obj_size = size;
        new_node->meta.status = LIST_T1;
        
        CDL_PREPEND(t1_head, new_node);
        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);
        size_t1++;
        
        return 0; // Miss
    }
    
    return 0;
}

//free() Helper
void free_arc_cache(){
    entry_node *current_node, *tmp;
    
    HASH_ITER(hh, cache_map, current_node, tmp){
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    
    t1_head = NULL;
    t2_head = NULL;
    b1_head = NULL;
    b2_head = NULL;
    
    size_t1 = 0;
    size_t2 = 0;
    size_b1 = 0;
    size_b2 = 0;
    p = 0;
}