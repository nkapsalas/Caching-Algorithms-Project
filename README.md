# Caching Algorithms Project

A high-performance cache simulation framework written in C that implements and compares 8 different caching replacement algorithms. This project evaluates cache performance through parallel execution on real-world memory access traces.

## Project Overview

This simulator processes compressed memory access traces and runs them through multiple caching algorithms simultaneously using multithreading. It measures and compares performance metrics including **hit rates**, **cache misses**, and **execution time** for each algorithm.

### Algorithms Implemented

| Algorithm | File | Type | Description |
|-----------|------|------|-------------|
| **LRU** | `LRU.c` | Recency-based | Evicts the least recently used item |
| **LFU** | `LFU.c` | Frequency-based | Evicts the least frequently used item |
| **LFU-k** | `LFU-k.c` | Frequency-based | Tracks only k last accesses (k=2) |
| **FF** | `FF.c` | First-in-first-out | FIFO eviction policy |
| **LRFU** | `LRFU.c` | Hybrid | Combines recency and frequency with decay |
| **LIRS** | `LIRS.c` | Recency-based | Low Inter-reference Recency Set algorithm |
| **2Q** | `_2Q.c` | Multi-queue | Two-queue approach with history tracking |
| **ARC** | `ARC.c` | Adaptive | Adaptive Replacement Cache - self-tuning algorithm |

## Project Structure

```
Caching-Algorithms-Project/
├── *.c                    # Algorithm implementations
├── cache.h                # Common data structures and interfaces
├── main.c                 # Main simulator and thread coordinator
├── get_wss.c              # Working set size calculator
├── Makefile               # Build configuration
├── run_experiments.sh     # Automated experiment script
├── plot_*.m               # MATLAB visualization scripts
├── results.txt            # Pre-computed results summary
└── README.md              # This file
```

## Dependencies

### Required
- **C compiler** (gcc/clang with C99 standard)
- **pthread library** (POSIX threads)
- **zstd** - Zstandard compression utility for decompressing traces

### Optional
- **MATLAB/Octave** - For generating performance plots

### Build Tools
```bash
# Ubuntu/Debian
sudo apt-get install build-essential libpthread-stubs0-dev zstandard

# macOS
brew install zstd
```

## Building the Project

### Using Make

```bash
# Compile all executables
make

# Clean all object files and executables
make clean
```

The Makefile automatically builds all algorithm implementations and the main simulator.

## Running Experiments

### Quick Start

```bash
# Run single algorithm with a specific trace file
./main <trace_file.zst> <cache_capacity>

# Example
./main trace.zst 1000
```

### Automated Full Suite

```bash
# Run all 3 input streams against all 8 algorithms
# Note: Execution takes approximately 10 minutes
bash run_experiments.sh
```

This script:
1. Calculates working set size using `get_wss`
2. Executes all 8 algorithms on all 3 input streams in parallel
3. Generates `results.txt` with comprehensive performance report

## Input Trace Format

Traces must be in compressed zstd format (`.zst`) containing binary request records:

```c
typedef struct {
    uint32_t timestamp;
    uint64_t obj_id;
    uint32_t obj_size;
    int64_t next_access_vtime;  // -1 if no next access
} request_t;
```

Each record is **24 bytes**. The `next_access_vtime` field enables optimal (Belady) algorithm simulation.

## Output

### Console Output

During execution, the program displays:
- Real-time trace loading progress
- Thread spawning information
- Final performance table with:
  - Cache hits / misses count
  - Hit rate percentage
  - Execution time in seconds

### Example Output
```
**********************************************************************
                   FINAL SIMULATION CACHE RESULTS OVERVIEW                    
**********************************************************************
Algorithm    | Total Cache Hits   | Total Cache Misses | Hit Rate (%) |Time (Seconds)
------------------------------------------------------------------------
LRU          | 45234              | 54766              | 45.23%       |0.12
LIRS         | 48921              | 51079              | 48.92%       |0.18
ARC          | 49450              | 50550              | 49.45%       |0.21
...
```

### Results File

`results.txt` - Detailed results from the latest full experiment run

## Performance Visualization

Generate performance plots using MATLAB scripts:

```matlab
% Plot cache hit rates across algorithms
plot_hit_rate

% Plot execution time comparison
plot_elapsed_time
```

These scripts process the `results.txt` file and generate comparative visualizations.

## Key Features

- ✅ **Parallel Execution** - All 8 algorithms run simultaneously using pthreads
- ✅ **Memory Efficient** - Dynamic buffer allocation for trace streaming
- ✅ **Accurate Timing** - High-resolution clock measurements via `clock_gettime()`
- ✅ **Real-world Traces** - Support for actual memory access patterns
- ✅ **Comprehensive Metrics** - Hit rate, miss count, and performance timing
- ✅ **Optimal Algorithm Support** - Can simulate omniscient Belady algorithm with trace hints

## Configuration

Edit `main.c` to adjust:
- `cache_capacity` - Cache size in items (default: 1000)
- `lrfu_lambda` - LRFU decay parameter (default: 0.01)

Or pass cache capacity as command-line argument:
```bash
./main trace.zst 2000  # 2000-item cache
```

## Dependencies Library Details

- **uthash.h** - Hash table library for O(1) cache lookups
- **utlist.h** - Linked list library for queue/list operations

These are included and require no external installation.

## Notes

- Execution of the full experiment suite requires ~10 minutes and significant system load
- Pre-computed `results.txt` is included for reference
- Thread-safe implementations ensure parallel algorithm independence
- Each algorithm runs independently; no inter-algorithm communication occurs

## Future Enhancements

- [ ] Support for additional trace formats
- [ ] Real-time visualization dashboard
- [ ] Statistical significance testing
- [ ] Configurable algorithm parameters from command-line
- [ ] Support for object size variation in eviction decisions

## References

- LRU: Cormen, Leiserson, Rivest, Stein - "Introduction to Algorithms"
- LIRS: Song Jiang, Xiaodong Zhang - "LIRS: an efficient low inter-reference recency set replacement policy"
- ARC: Nimrod Megiddo, Dharmendra S. Modha - "ARC: A Self-Tuning, Low Overhead Replacement Cache"
- 2Q: Theodore Johnson, Dennis Shasha - "2Q: A Low Overhead High Performance Buffer Management Replacement Algorithm"

---

**Author:** nkapsalas  
**Language:** C  
**Build Tool:** Make  
**License:** [Add your license here]
