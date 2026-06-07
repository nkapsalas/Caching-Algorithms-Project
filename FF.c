#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cache.h"

// GLOBAL VARIABLES
static entry_node *cache_map = NULL;          // Hash Table for cache
static entry_node **max_heap = NULL;          // Max-Heap array
static int current_size = 0;
static int max_capacity = 1000;

// MAX-HEAP HELPERS (O(log N) Complexity)

// Helper to handle the -1 
// -1 means "never accessed again", so it gets priority for eviction.
static int64_t get_priority(entry_node *node){
    if (node->meta.next_access == -1){
        return INT64_MAX; // Highest possible future value
    }
    return node->meta.next_access;
}

// Swaps two nodes in the Heap and updates their heap_idx
static void swap_nodes(int i, int j){
    entry_node *temp = max_heap[i];
    
    max_heap[i] = max_heap[j];
    max_heap[i]->heap_idx = i; 
    
    max_heap[j] = temp;
    max_heap[j]->heap_idx = j; 
}

// Pushes node up towards the Root if its future access is LARGER than its parent
static void heapify_up(int index){
    while (index > 0){
        int parent = (index - 1) / 2;
        
        if (get_priority(max_heap[index]) > get_priority(max_heap[parent])){
            swap_nodes(index, parent);
            index = parent;
        } 
        else{
            break;
        }
    }
}

// Iterative sink down to restore Max-Heap
static void heapify_down(int index){
    while (1) {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < current_size && get_priority(max_heap[left]) > get_priority(max_heap[largest])){
            largest = left;
        }

        if (right < current_size && get_priority(max_heap[right]) > get_priority(max_heap[largest])){
            largest = right;
        }

        if (largest != index){
            swap_nodes(index, largest);
            index = largest; 
        } else {
            break; 
        }
    }
}

// fixes the heap structure in both cases
static void update_node_position(int index){
    heapify_up(index);
    heapify_down(index);
}

// Belady's OPT Eviction Policy
void init_opt_cache(int capacity){
    max_capacity = capacity;
    max_heap = (entry_node **)malloc(max_capacity * sizeof(entry_node *));
    if (max_heap == NULL){
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
    }
}

// The signature requires the next_access_vtime from trace file
int process_opt_request(uint64_t requested_id, uint32_t size, int64_t next_vtime){
    entry_node *target_node = NULL;
    
    // O(1) Lookup
    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL){
        // >Cache HIT<
        // Update the item's exact next access time from the trace
        target_node->meta.next_access = next_vtime;
        
        // Fix the Max-Heap structure since priority changed
        update_node_position(target_node->heap_idx);
        
        return 1; // Hit

    } 
    else{
        // >Cache MISS<        
        entry_node *new_node;

        if (current_size >= max_capacity){
            //Recycle root
            new_node = max_heap[0];
            
            HASH_DEL(cache_map, new_node);
            
            // Move last element to the Root
            max_heap[0] = max_heap[current_size - 1];
            max_heap[0]->heap_idx = 0;
            current_size--;
            
            // Sink it down to restore the Max-Heap property
            heapify_down(0);
        } else {
            // Cache is not full yet, so a normal malloc is justified
            new_node = (entry_node *)malloc(sizeof(entry_node));
            if (new_node == NULL){
                fprintf(stderr, "Memory allocation failed!\n");
                exit(1);
            }
        }

        // Initialize the new or recycled node
        new_node->obj_id = requested_id;
        new_node->obj_size = size;
        new_node->meta.next_access = next_vtime; 
        
        // Insert at the end of the tree and Heapify Up
        new_node->heap_idx = current_size;
        max_heap[current_size] = new_node;
        current_size++;
        
        heapify_up(new_node->heap_idx); 
        
        // Insert into the Hash Table
        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);
        
        return 0; // Miss
    }
}

//Free() Helper
void free_opt_cache(){
    entry_node *current_node, *tmp;
    
    HASH_ITER(hh, cache_map, current_node, tmp){
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    
    free(max_heap);
    max_heap = NULL;
    current_size = 0;
}