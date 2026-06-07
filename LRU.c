#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "utlist.h" 

//Global variables
static entry_node *cache_map = NULL; //Hash table for cache  
static entry_node *lru_head = NULL;  //DL list for LRU   
static int current_size = 0;
static int max_capacity = 1000;       


void init_lru_cache(int capacity){
    max_capacity = capacity;
}

//LRU eviction Policy
int process_lru_request(uint64_t requested_id, uint32_t size){
    
    entry_node *target_node = NULL;

    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL) {
        // -CACHE HIT-
        // Remove it from its current spot
        CDL_DELETE(lru_head, target_node);
        // Insert at head as Most Recently Used
        CDL_PREPEND(lru_head, target_node);
        
        return 1; // Hit

    } else {
        // -CACHE MISS-
        entry_node *new_node;

        // Eviction Policy & HPC Node Recycling
        if (current_size >= max_capacity){
            // In a circular list, the tail is ALWAYS head->prev
            new_node = lru_head->prev; 
            
            CDL_DELETE(lru_head, new_node);          // Remove from List
            HASH_DEL(cache_map, new_node);           // Remove from Hash
            
            
        } else {
            // Insert New Item (Only happens during the initial fill)
            new_node = (entry_node *)malloc(sizeof(entry_node));
            if (new_node == NULL){
                fprintf(stderr, "Memory allocation failed!\n");
                exit(1);
            }
            current_size++;
        }

        // Setup the new (or recycled) node
        new_node->obj_id = requested_id;
        new_node->obj_size = size;

        CDL_PREPEND(lru_head, new_node);                                 // O(1) List Insert
        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);     // O(1) Hash Insert
        
        return 0; // Miss
    }
}

//Free() Helper
void free_lru_cache(){
    entry_node *current_node, *tmp;
    HASH_ITER(hh, cache_map, current_node, tmp){
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    lru_head = NULL;
    current_size = 0;
}