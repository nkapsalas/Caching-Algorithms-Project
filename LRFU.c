#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>       
#include "cache.h"

// GLOBAL VARIABLES
static entry_node *cache_map = NULL;          // Hash Table for cache
static entry_node **min_heap = NULL;          // Array functioning as the Min-Heap
static int current_size = 0;
static int max_capacity = 1000;
static uint32_t global_time = 0;              // Simulates time
static double lambda = 0.01;                  // Decay parameter 


// LRFU FAST MATH OPTIMIZATION (LOOKUP TABLE)
#define MAX_LOOKUP_DELTA 1000000
static double pow_lookup[MAX_LOOKUP_DELTA];

// Called once during initialization to pre-compute the decay factors
static void init_lookup_table() {
    for (int i = 0; i < MAX_LOOKUP_DELTA; i++) {
        pow_lookup[i] = pow(2.0, -lambda * (double)i);
    }
}

// Replaces the heavy pow() function with an O(1) memory read
static inline double fast_decay(uint32_t delta_t) {
    if (delta_t < MAX_LOOKUP_DELTA) {
        return pow_lookup[delta_t]; // Lightning-fast array lookup
    }
    // Fallback only if delta_t is too large
    return pow(2.0, -lambda * (double)delta_t); 
}


// MIN-HEAP HELPERS (O(log N) Complexity)
// Swaps two nodes in the Heap and updates their heap_idx
static void swap_nodes(int i, int j){
    entry_node *temp = min_heap[i];
    
    min_heap[i] = min_heap[j];
    min_heap[i]->heap_idx = i; 
    
    min_heap[j] = temp;
    min_heap[j]->heap_idx = j; 
}

// Pushes node to Root if its CRF score is SMALLER than its parent
static void heapify_up(int index){
    while (index > 0) {
        int parent = (index - 1) / 2;
        
        // Min-Heap based on the CRF score
        if (min_heap[index]->meta.lrfu.crf < min_heap[parent]->meta.lrfu.crf){
            swap_nodes(index, parent);
            index = parent;
        } else {
            break;
        }
    }
}

// Iterative sink down to restore Min-Heap property 
static void heapify_down(int index){
    while (1) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < current_size && 
            min_heap[left]->meta.lrfu.crf < min_heap[smallest]->meta.lrfu.crf){
            smallest = left;
        }

        if (right < current_size && 
            min_heap[right]->meta.lrfu.crf < min_heap[smallest]->meta.lrfu.crf){
            smallest = right;
        }

        if (smallest != index){
            swap_nodes(index, smallest);
            index = smallest; 
        } else {
            break; 
        }
    }
}

// LRFU Eviction Policy
void init_lrfu_cache(int capacity, double decay_param){
    max_capacity = capacity;
    lambda = decay_param;
    
    // Initialize the Math Lookup Table for extreme speed
    init_lookup_table();
    
    min_heap = (entry_node **)malloc(max_capacity * sizeof(entry_node *));
    if (min_heap == NULL){
        fprintf(stderr, "Failed to allocate memory for Min-Heap\n");
        exit(1);
    }
}

int process_lrfu_request(uint64_t requested_id, uint32_t size){
    global_time++; 
    
    entry_node *target_node = NULL;
    
    // O(1) Lookup
    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL){
        // >Cache HIT<
        // Calculate time elapsed since last access
        uint32_t delta_t = global_time - target_node->meta.lrfu.last_access_time;
        
        // Fetch the pre-computed decay factor 
        double decay_factor = fast_decay(delta_t);
        
        // Update the CRF score
        target_node->meta.lrfu.crf = 1.0 + (target_node->meta.lrfu.crf * decay_factor);
        target_node->meta.lrfu.last_access_time = global_time;
        
        // Since the CRF score just increased (due to the hit), 
        // it needs to sink down the Min-Heap.
        heapify_down(target_node->heap_idx);
        
        return 1; // Hit

    }
    else{
        // >Cache MISS<
        entry_node *new_node;

        if (current_size >= max_capacity){
            //Recycle Root
            new_node = min_heap[0];
            
            HASH_DEL(cache_map, new_node);
            
            // Move the last element of the tree to the Root
            min_heap[0] = min_heap[current_size - 1];
            min_heap[0]->heap_idx = 0;
            current_size--;
            
            // Sink it down to restore the Min-Heap property
            heapify_down(0);
        } else {
            // Cache is not full yet, standard allocation
            new_node = (entry_node *)malloc(sizeof(entry_node));
            if (new_node == NULL){
                fprintf(stderr, "Memory allocation failed!\n");
                exit(1);
            }
        }
        
        // Setup the new or recycled entry
        new_node->obj_id = requested_id;
        new_node->obj_size = size;
        
        // Initialize LRFU fields
        new_node->meta.lrfu.crf = 1.0;                  // Base score
        new_node->meta.lrfu.last_access_time = global_time;
        
        // Insert at the end of the tree and Heapify Up
        new_node->heap_idx = current_size;
        min_heap[current_size] = new_node;
        current_size++;
        
        heapify_up(new_node->heap_idx); 
        
        // Insert into the Hash Table
        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);
        
        return 0; // Miss
    }
}

//Free() Helper Functions
void free_lrfu_cache(){
    entry_node *current_node, *tmp;
    
    HASH_ITER(hh, cache_map, current_node, tmp) {
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    
    free(min_heap);
    min_heap = NULL;
    current_size = 0;
    global_time = 0; // Reset global time for the next simulation
}