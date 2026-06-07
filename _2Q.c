#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cache.h"
#include "utlist.h"

// GLOBAL VARIABLES
static entry_node *cache_map = NULL;   // uthash Hash Table pointer
static entry_node *a1in_head = NULL;   // Circular DL-List (FIFO for recent entries)
static entry_node *a1out_head = NULL;  // Circular DL-List (FIFO Ghost list)
static entry_node *am_head = NULL;     // Circular DL-List (LRU for main entries)

// Capacity trackers
static int max_a1in = 0;
static int max_a1out = 0;
static int max_am = 0;
// Current size trackers
static int size_a1in = 0;
static int size_a1out = 0;
static int size_am = 0;

// 2Q Helpers
void init_2q_cache(int total_capacity){
    // Standard 2Q parameters
    max_a1in = (int)(total_capacity * 0.25);
    if (max_a1in == 0) max_a1in = 1;
    
    max_am = total_capacity - max_a1in;
    max_a1out = (int)(total_capacity * 0.50); 
}

int process_2q_request(uint64_t requested_id, uint32_t size){
    entry_node *target_node = NULL;
    
    // O(1) Lookup
    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL){
        
        // -CACHE HIT: Entry is in Am (Main Cache)-
        if (target_node->meta.status == QUEUE_AM){
            // Am is an LRU list, so we insert it at head (MRU)
            CDL_DELETE(am_head, target_node);
            CDL_PREPEND(am_head, target_node);
            return 1; // Hit
        }
        
        // -GHOST HIT: Entry is in A1out (History)-
        else if (target_node->meta.status == QUEUE_A1OUT){
            // Promote to Am
            CDL_DELETE(a1out_head, target_node);
            size_a1out--;

            // If Am is full, evict its LRU item (tail)
            if (size_am >= max_am){
                entry_node *evicted_am = am_head->prev;
                CDL_DELETE(am_head, evicted_am);
                HASH_DEL(cache_map, evicted_am);
                free(evicted_am);
                size_am--;
            }

            // Move the target into Am
            target_node->meta.status = QUEUE_AM;
            CDL_PREPEND(am_head, target_node);
            size_am++;
          
            // Its actual data was not in memory, so it is a Cache Miss.
            return 0; // Miss
        }
        
        // -CACHE HIT: Entry is in A1in (Trial Queue)-
        else if (target_node->meta.status == QUEUE_A1IN){
            // A1in is strictly FIFO in standard 2Q so no changes
            return 1; // Hit
        }
        
    } 
    else{
        // -Cache Miss- 
        //(New Item)
        // If A1in is full, we must demote its oldest item to A1out
        if (size_a1in >= max_a1in){
            entry_node *demoted_node = a1in_head->prev; // Tail of A1in
            CDL_DELETE(a1in_head, demoted_node);
            size_a1in--;

            // If A1out is full, evict its oldest HIR 
            if (size_a1out >= max_a1out){
                entry_node *evicted_ghost = a1out_head->prev;
                CDL_DELETE(a1out_head, evicted_ghost);
                HASH_DEL(cache_map, evicted_ghost);
                free(evicted_ghost);
                size_a1out--;
            }
            
            // Add the demoted node to A1out
            demoted_node->meta.status = QUEUE_A1OUT;
            CDL_PREPEND(a1out_head, demoted_node);
            size_a1out++;
        }
        
        // Create the new item and add it to A1in
        entry_node *new_node = (entry_node *)malloc(sizeof(entry_node));
        if (new_node == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }
        
        new_node->obj_id = requested_id;
        new_node->obj_size = size;
        new_node->meta.status = QUEUE_A1IN;
        
        CDL_PREPEND(a1in_head, new_node);
        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);
        size_a1in++;
        
        return 0; // Miss
    }
    
    return 0;
}

//free() Helper
void free_2q_cache(){
    entry_node *current_node, *tmp;
    
    HASH_ITER(hh, cache_map, current_node, tmp){
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    
    a1in_head = NULL;
    a1out_head = NULL;
    am_head = NULL;
    
    size_a1in = 0;
    size_a1out = 0;
    size_am = 0;
}