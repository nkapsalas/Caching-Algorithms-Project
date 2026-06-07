#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include<time.h>
#include "cache.h"

// TRACE STRUCTURE & SPECS
typedef struct __attribute__((packed)){
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;  // -1 if no next access
} request_t;

// Configuration settings
int cache_capacity = 1000;
double lrfu_lambda = 0.01;

// Shared memory trace buffer
request_t *trace_buffer = NULL;
uint64_t total_requests = 0;

// Thread results tracking
typedef struct{
    const char *algo_name;
    uint64_t hits;
    uint64_t misses;
    double hit_rate;
    double elapsed_seconds;
} result_t;

// External caching inits
extern void init_lru_cache(int capacity);
extern void init_lfuk_cache(int capacity);
extern void init_lfu_cache(int capacity);
extern void init_opt_cache(int capacity);
extern void init_lrfu_cache(int capacity, double decay_param);
extern void init_lirs_cache(int capacity);
extern void init_2q_cache(int total_capacity);
extern void init_arc_cache(int capacity);

// External caching entry requests
extern int process_lru_request(uint64_t requested_id, uint32_t size);
extern int process_lfuk_request(uint64_t requested_id, uint32_t size);
extern int process_lfu_request(uint64_t requested_id, uint32_t size);
extern int process_opt_request(uint64_t requested_id, uint32_t size, int64_t next_vtime);
extern int process_lrfu_request(uint64_t requested_id, uint32_t size);
extern int process_lirs_request(uint64_t requested_id, uint32_t size);
extern int process_2q_request(uint64_t requested_id, uint32_t size);
extern int process_arc_request(uint64_t requested_id, uint32_t size);

// External free routines
extern void free_lru_cache();
extern void free_lfuk_cache();
extern void free_lfu_cache();
extern void free_opt_cache();
extern void free_lrfu_cache();
extern void free_lirs_cache();
extern void free_2q_cache();
extern void free_arc_cache();

//PARALLEL THREAD WORKERS
void* run_lru(void* arg) {
    result_t *res = (result_t*)arg;
    // Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_lru_cache(cache_capacity);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_lru_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size)) res->hits++;
        else res->misses++;
    }
    free_lru_cache();

    // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

void* run_lfuk(void* arg) {
    result_t *res = (result_t*)arg;
    // Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_lfuk_cache(cache_capacity);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_lfuk_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size)) res->hits++;
        else res->misses++;
    }
    free_lfuk_cache();
     // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

void* run_lfu(void* arg) {
    result_t *res = (result_t*)arg;
    // Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_lfu_cache(cache_capacity);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_lfu_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size)) res->hits++;
        else res->misses++;
    }
    free_lfu_cache();
     // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

void* run_opt(void* arg) {
    result_t *res = (result_t*)arg;
    // Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_opt_cache(cache_capacity);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_opt_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size, trace_buffer[i].next_access_vtime)) res->hits++;
        else res->misses++;
    }
    free_opt_cache();
    // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

void* run_lrfu(void* arg) {
    result_t *res = (result_t*)arg;
    // Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_lrfu_cache(cache_capacity, lrfu_lambda);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_lrfu_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size)) res->hits++;
        else res->misses++;
    }
    free_lrfu_cache();
    // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

void* run_lirs(void* arg) {
    result_t *res = (result_t*)arg;
    // Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_lirs_cache(cache_capacity);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_lirs_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size)) res->hits++;
        else res->misses++;
    }
    free_lirs_cache();
    // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

void* run_2q(void* arg) {
    result_t *res = (result_t*)arg;
    // Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_2q_cache(cache_capacity);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_2q_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size)) res->hits++;
        else res->misses++;
    }
    free_2q_cache();
    // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

void* run_arc(void* arg) {
    result_t *res = (result_t*)arg;

    //Private to this thread's stack
    struct timespec start, end;
    // Start the clock
    clock_gettime(CLOCK_MONOTONIC, &start);

    init_arc_cache(cache_capacity);
    for (uint64_t i = 0; i < total_requests; i++) {
        if (process_arc_request(trace_buffer[i].obj_id, trace_buffer[i].obj_size)) res->hits++;
        else res->misses++;
    }
    free_arc_cache();
    // Stop the clock
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calc Elapsed time
    res->elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return (NULL);
}

//MAIN STREAM DECOMPRESSION AND EXECUTION CONTROL
int main(int argc, char *argv[]) {

    //FORCE PRINT
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <trace_file.zst> <cache_capacity>\n", argv[0]);
        return 1;
    }

    char *trace_filename = argv[1];
    cache_capacity = atoi(argv[2]);

    // Construct the standard decompression command pipe
    char zstd_command[512];
    snprintf(zstd_command, sizeof(zstd_command), "zstd -dcf %s", trace_filename);

    printf("[1/3] Streaming and decompressing input trace file...\n");
    FILE *pipe_in = popen(zstd_command, "rb");
    if (!pipe_in) {
        fprintf(stderr, "Error opening pipe wrapper for zstd decompression.\n");
        return 1;
    }

    // Dynamic array allocation parameters
    uint64_t buffer_capacity = 1000000; // Initial 1M requests chunk
    trace_buffer = (request_t *)malloc(buffer_capacity * sizeof(request_t));
    
    request_t temporary_req;
    // Single-pass decompression block read
    while (fread(&temporary_req, sizeof(request_t), 1, pipe_in) == 1) {
        if (total_requests >= buffer_capacity) {
            buffer_capacity *= 2;
            
            // Ασφαλής δέσμευση μνήμης
            request_t *temp_ptr = (request_t *)realloc(trace_buffer, buffer_capacity * sizeof(request_t));
            if (temp_ptr == NULL) {
                fprintf(stderr, "\n[FATAL ERROR] Out of Memory! Failed to allocate RAM for %llu requests.\n", buffer_capacity);
                free(trace_buffer);
                pclose(pipe_in);
                return 1;
            }
            trace_buffer = temp_ptr;
        }
        trace_buffer[total_requests++] = temporary_req;
    }
    pclose(pipe_in);
    printf("Successfully preloaded %llu requests into shared memory layout.\n", total_requests);

    // Initializing execution tracking structs
    pthread_t workers[8];
    result_t pipeline_results[8] = {
        {"LRU", 0, 0, 0.0, 0.0},    {"LFU-k", 0, 0, 0.0, 0.0}, 
        {"LFU", 0, 0, 0.0, 0.0},    {"OPT", 0, 0, 0.0, 0.0},
        {"LRFU", 0, 0, 0.0, 0.0},   {"LIRS", 0, 0, 0.0, 0.0},  
        {"2Q", 0, 0, 0.0, 0.0},     {"ARC", 0, 0, 0.0, 0.0}
    };

    void* (*worker_functions[8])(void*) = {
        run_lru, run_lfuk, run_lfu, run_opt,
        run_lrfu, run_lirs, run_2q, run_arc
    };

    printf("[2/3] Spawning 8 computational threads for simultaneous cache simulation...\n");
    for (int i = 0; i < 8; i++) {
        if (pthread_create(&workers[i], NULL, worker_functions[i], &pipeline_results[i]) != 0) {
            fprintf(stderr, "Error spawning thread for %s engine\n", pipeline_results[i].algo_name);
            return 1;
        }
    }

    printf("[3/3] Waiting for all simulations to finalize entries...\n");
    for (int i = 0; i < 8; i++) {
        pthread_join(workers[i], NULL);
        pipeline_results[i].hit_rate = ((double)pipeline_results[i].hits / total_requests) * 100.0;
    }

    //PERFORMANCE ANALYTICS
    printf("\n**********************************************************************\n");
    printf("                       FINAL SIMULATION CACHE RESULTS OVERVIEW                    \n");
    printf("************************************************************************\n");
    printf("%-12s | %-18s | %-18s | %-12s | %-15s\n", "Algorithm", "Total Cache Hits", "Total Cache Misses", "Hit Rate (%)","Time (Seconds)");
    printf("------------------------------------------------------------------------\n");
    for (int i = 0; i < 8; i++) {
        printf("%-12s | %-18llu | %-18llu | %-11.2f%% |%-15.2f\n",
               pipeline_results[i].algo_name,
               pipeline_results[i].hits,
               pipeline_results[i].misses,
               pipeline_results[i].hit_rate,
               pipeline_results[i].elapsed_seconds);
    }
    printf("************************************************************************\n");

    free(trace_buffer);
    return 0;
}