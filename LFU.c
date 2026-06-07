#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cache.h"
#include "utlist.h"

#define MAX_FREQ 10000 //Upper bound for frequency buckets

// GLOBAL VARIABLES
static entry_node *freq_buckets[MAX_FREQ] = {NULL}; // Array of Circular DL-List heads
static entry_node *cache_map = NULL;               // O(1) Lookup Hash Table
static int current_size = 0;
static int max_capacity = 1000;
static int min_freq = 0;                           // Tracks the lowest active frequency

// LFU Bucket System Helper Functions
void init_lfu_cache(int capacity){
    max_capacity = capacity;
    current_size = 0;
    min_freq = 0;
}

int process_lfu_request(uint64_t requested_id, uint32_t size){
    entry_node *target_node = NULL;
    
    // O(1) Lookup
    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL){
        // >Cache HIT<
        int curr_freq = target_node->meta.frequency;

        // Remove from its current frequency bucket
        CDL_DELETE(freq_buckets[curr_freq], target_node);

        // Increment frequency (capped at MAX_FREQ - 1 to prevent segfaults)
        if (curr_freq < MAX_FREQ - 1){
            target_node->meta.frequency++;
        }
        int new_freq = target_node->meta.frequency;

        // Add to the head of the new frequency bucket (MRU)
        CDL_PREPEND(freq_buckets[new_freq], target_node);

        // Update min_freq if the old bucket is now empty
        if (freq_buckets[curr_freq] == NULL && min_freq == curr_freq){
            min_freq = new_freq;
        }

        return 1; // Hit

    } else {
        // >Cache MISS<
        if (current_size >= max_capacity){
            // Eviction: Target the min_freq bucket.
            // Since it's a circular list, head->prev gives us the tail (LRU) in O(1).
            entry_node *evicted_node = freq_buckets[min_freq]->prev;

            // Remove from List and Hash Table
            CDL_DELETE(freq_buckets[min_freq], evicted_node);
            HASH_DEL(cache_map, evicted_node);
            free(evicted_node);
            
            current_size--;
        }

        // Create new node
        entry_node *new_node = (entry_node *)malloc(sizeof(entry_node));
        if (new_node == NULL){
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }

        new_node->obj_id = requested_id;
        new_node->obj_size = size;
        new_node->meta.frequency = 1;
        
        // Insert into the frequency = 1 bucket
        CDL_PREPEND(freq_buckets[1], new_node);
        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);
        
        current_size++;
        
        // A new entry always drops the min_freq back to 1
        min_freq = 1; 

        return 0; // Miss
    }
}

//Free() Helper
void free_lfu_cache(){
    entry_node *current_node, *tmp;
    
    HASH_ITER(hh, cache_map, current_node, tmp) {
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    
    current_size = 0;
    min_freq = 0;
    
    for (int i = 0; i < MAX_FREQ; i++) {
        freq_buckets[i] = NULL;
    }
}