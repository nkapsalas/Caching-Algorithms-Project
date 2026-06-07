#include "uthash.h"
#include <stdint.h>

//General Struct for smooth function of all Algorithms

// Enums for multi-queue algorithms (2Q, LIRS, ARC)
typedef enum {
    LIST_T1, LIST_T2, LIST_B1, LIST_B2, // ARC
    QUEUE_A1IN, QUEUE_A1OUT, QUEUE_AM,  // 2Q
    STATUS_LIR, STATUS_HIR, GHOST_HIR   // LIRS
} location_status_t;

typedef struct entry_node {
    // Both fields from input stream
    // request_t struct
    uint64_t obj_id;          
    uint32_t obj_size;
    
    // Pointers for Doubly-Linked Lists (LRU, LFU buckets, 2Q, ARC, LIRS)
    struct entry_node *prev;
    struct entry_node *next;

    // The uthash handle (Required for O(1) lookups)
    UT_hash_handle hh;

    // Tracks Position in heap (OPT, LFU-k, LRFU)
    int heap_idx;          

    // Algorithm-Specific Data 
    union {
        // LFU
        int frequency;

        // LFU-k (k=2)
        uint32_t history[2];

        // LRFU
        struct {
            double crf;
            uint32_t last_access_time; // timestamp from request_t
        } lrfu;

        // LIRS
        struct {
            location_status_t status;
            struct entry_node *q_prev; // Δείκτης για την List_Q
            struct entry_node *q_next; // Δείκτης για την List_Q
        } lirs;
        
        // OPT (Belady)
        int64_t next_access;           // from request_t.next_access_vtime

        // Multi-Queue Tracking (LIRS, 2Q, ARC)
        location_status_t status;
        
    } meta;

} entry_node;