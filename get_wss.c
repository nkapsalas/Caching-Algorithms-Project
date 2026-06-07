#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "uthash.h"

// Trace format
typedef struct __attribute__((packed)) {
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;
} request_t;

// Counter struct
typedef struct {
    uint64_t obj_id;
    UT_hash_handle hh;
} unique_node_t;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <trace_file.zst>\n", argv[0]);
        return 1;
    }

    char zstd_command[512];
    snprintf(zstd_command, sizeof(zstd_command), "zstd -dcf %s", argv[1]);

    FILE *pipe_in = popen(zstd_command, "rb");
    if (!pipe_in) {
        fprintf(stderr, "Error opening pipe.\n");
        return 1;
    }

    unique_node_t *hash_set = NULL;
    request_t req;
    uint64_t total_requests = 0;

    // Read Input stream
    while (fread(&req, sizeof(request_t), 1, pipe_in) == 1) {
        total_requests++;
        unique_node_t *found_item = NULL;
        
        // Check Lookup H_table
        HASH_FIND(hh, hash_set, &req.obj_id, sizeof(uint64_t), found_item);
        
        //If not found add to set
        if (found_item == NULL) {
            found_item = (unique_node_t *)malloc(sizeof(unique_node_t));
            found_item->obj_id = req.obj_id;
            HASH_ADD(hh, hash_set, obj_id, sizeof(uint64_t), found_item);
        }
    }
    
    pclose(pipe_in);
    
    // Print Final count for Bash script
    printf("%u\n", HASH_COUNT(hash_set));
    
    // Free memory
    unique_node_t *current, *tmp;
    HASH_ITER(hh, hash_set, current, tmp) {
        HASH_DEL(hash_set, current);
        free(current);
    }

    return 0;
}