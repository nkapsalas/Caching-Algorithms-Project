    #include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cache.h"

//GLOBAL VARIABLES
static entry_node *cache_map = NULL;          // Hash Table 
static entry_node **min_heap = NULL;          // Min-Heap array
static int current_size = 0;
static int max_capacity = 1000;
static uint32_t global_time = 0;              // curr time tracker (increments at each step)


// MIN-HEAP HELPER Functions (O(log N) complexity)
//swaps indexes and updates ids
static void swap_nodes(int i, int j){
    entry_node *temp = min_heap[i];
    
    min_heap[i] = min_heap[j];
    min_heap[i]->heap_idx = i; // Ενημέρωση της νέας θέσης
    
    min_heap[j] = temp;
    min_heap[j]->heap_idx = j; // Ενημέρωση της νέας θέσης
}

// Pushes Min to root
static void heapify_up(int index) {
    while (index > 0){
        int parent = (index - 1) / 2;
        // compare with 2nd oldest access history[1]
        if (min_heap[index]->meta.history[1] < min_heap[parent]->meta.history[1]){
            swap_nodes(index, parent);
            index = parent;
        } 
        else{
            break;
        }
    }
}

// Push higher priority entries down the heap to the leaves
static void heapify_down(int index) {
    while (1) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < current_size && 
            min_heap[left]->meta.history[1] < min_heap[smallest]->meta.history[1]) {
            smallest = left;
        }

        if (right < current_size && 
            min_heap[right]->meta.history[1] < min_heap[smallest]->meta.history[1]) {
            smallest = right;
        }

        if (smallest != index) {
            swap_nodes(index, smallest);
            index = smallest; 
        } else {
            break;
        }
    }
}

// LFU-2 Eviction Policy
// Initialisation function
void init_lfuk_cache(int capacity){
    max_capacity = capacity;
    min_heap = (entry_node **)malloc(max_capacity * sizeof(entry_node *));
      if (min_heap == NULL){
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }
}

int process_lfuk_request(uint64_t requested_id, uint32_t size) {
    global_time++; 
    
    entry_node *target_node = NULL;
    HASH_FIND(hh, cache_map, &requested_id, sizeof(uint64_t), target_node);

    if (target_node != NULL){
        // >Cache HIT<        // History Update
        target_node->meta.history[1] = target_node->meta.history[0];
        target_node->meta.history[0] = global_time; // Το νέο πρόσφατο είναι τώρα
        
        heapify_down(target_node->heap_idx);
        
        return 1; // Hit

    } 
    else {
        // >Cache MISS<
        entry_node *new_node;

        if (current_size >= max_capacity) {
            //Reuse root node
            new_node = min_heap[0]; 
            
            HASH_DEL(cache_map, new_node);
            
            // Push to heap
            min_heap[0] = min_heap[current_size - 1];
            min_heap[0]->heap_idx = 0;
            current_size--;
            heapify_down(0);
        } else {
            new_node = (entry_node *)malloc(sizeof(entry_node));
            if (new_node == NULL) {
                fprintf(stderr, "Memory allocation failed!\n");
                exit(1);
            }
        }

        new_node->obj_id = requested_id;
        new_node->obj_size = size;
        new_node->meta.history[0] = global_time;
        new_node->meta.history[1] = 0; 
        
        // Place at leave and Push up
        new_node->heap_idx = current_size;
        min_heap[current_size] = new_node;
        current_size++;
        
        heapify_up(new_node->heap_idx); 
    
        HASH_ADD(hh, cache_map, obj_id, sizeof(uint64_t), new_node);
        
        return 0; // Miss
    }
 
}
//Free() Helper
void free_lfuk_cache(){
    entry_node *current_node, *tmp;
    HASH_ITER(hh, cache_map, current_node, tmp){
        HASH_DEL(cache_map, current_node);
        free(current_node);
    }
    free(min_heap);
    min_heap = NULL;
    current_size = 0;
}