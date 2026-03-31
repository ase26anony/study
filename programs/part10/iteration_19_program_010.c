/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID leaf 2 cache detection
 * logic by compiling with different -march flags for various x86 CPUs.
 * Each CPU model may return different cache descriptor bytes (0x0a, 0x0c, etc.)
 * which are handled in driver-i386.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Memory barrier to prevent optimization */
#define MB() __asm__ __volatile__("" ::: "memory")

/* Target-specific function declarations */
#ifdef TEST_PENTIUM3
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
__attribute__((target("arch=core2")))
#endif
#ifdef TEST_NEHALEM
__attribute__((target("arch=nehalem")))
#endif
static void cache_thrash_benchmark(int iterations, int buffer_size_kb);

/* Main benchmark function that uses cache heavily */
static void run_cache_benchmark(const char* cpu_name, int buffer_size_kb) {
    volatile int result = 0;
    const int iterations = 100;
    
    printf("Running cache benchmark for %s with %dKB buffer\n", 
           cpu_name, buffer_size_kb);
    
    /* Force memory allocation and initialization */
    size_t buffer_size = buffer_size_kb * 1024 / sizeof(int);
    int* buffer = (int*)malloc(buffer_size * sizeof(int));
    
    if (!buffer) {
        printf("Memory allocation failed for %dKB\n", buffer_size_kb);
        return;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = rand();
    }
    
    MB(); /* Memory barrier */
    
    /* Cache thrashing benchmark */
    clock_t start = clock();
    
    /* Access pattern designed to stress cache associativity */
    for (int iter = 0; iter < iterations; iter++) {
        /* Linear access (good for prefetch) */
        for (size_t i = 0; i < buffer_size; i += 64) {
            buffer[i] = buffer[i] * 3 + 1;
        }
        
        MB();
        
        /* Strided access (tests associativity) */
        for (size_t i = 0; i < buffer_size; i += 17) {
            buffer[i] = buffer[i] * 5 - 2;
        }
        
        MB();
        
        /* Reverse access */
        for (size_t i = buffer_size - 1; i > 0; i -= 31) {
            buffer[i] = buffer[i] * 7 + 3;
        }
        
        MB();
    }
    
    clock_t end = clock();
    
    /* Prevent dead code elimination */
    for (size_t i = 0; i < buffer_size; i += 128) {
        result ^= buffer[i];
    }
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds, Checksum: %d\n", elapsed, result);
    
    free(buffer);
}

/* Function with target attribute for multi-versioning */
#if defined(USE_MULTIVERSION) && __GNUC__ >= 4
__attribute__((target_clones("pentium3,pentium4,nocona,k8,core2,nehalem")))
#endif
void multi_version_benchmark() {
    /* This function will be compiled for multiple targets */
    run_cache_benchmark("multi-version", 2048);
}

int main(int argc, char** argv) {
    printf("=== GCC Cache Detection Test Program ===\n");
    printf("Compiled with: ");
    
#ifdef TEST_PENTIUM3
    printf("-march=pentium3 (Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24)\n");
    /* Pentium III: L1: 16KB, L2: 256KB/512KB */
    run_cache_benchmark("Pentium III", 256);  /* L2 size */
    run_cache_benchmark("Pentium III", 16);   /* L1 size */
#endif

#ifdef TEST_PENTIUM4
    printf("-march=pentium4 (Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x39-0x3e, 0x41-0x45)\n");
    /* Pentium 4: Various cache configurations */
    run_cache_benchmark("Pentium 4", 512);
    run_cache_benchmark("Pentium 4", 1024);
#endif

#ifdef TEST_NOCONA
    printf("-march=nocona (Targets cases: 0x49, 0x60, 0x66-0x68, 0x78-0x87)\n");
    /* Intel Nocona (Xeon DP): L2: 1MB-2MB, may trigger 0x49 if not Xeon MP */
    run_cache_benchmark("Nocona", 1024);
    run_cache_benchmark("Nocona", 2048);
#endif

#ifdef TEST_K8
    printf("-march=k8 (Targets cases: 0x40, 0x48, 0x49, 0x4e, 0x78-0x87)\n");
    /* AMD K8: L1: 64KB, L2: 512KB-1MB */
    run_cache_benchmark("AMD K8", 512);
    run_cache_benchmark("AMD K8", 1024);
#endif

#ifdef TEST_CORE2
    printf("-march=core2 (Targets cases: 0x49, 0x66, 0x67, 0x68, 0x78-0x87)\n");
    /* Intel Core 2: L1: 32KB, L2: 2MB-4MB */
    run_cache_benchmark("Core 2", 2048);
    run_cache_benchmark("Core 2", 4096);
#endif

#ifdef TEST_NEHALEM
    printf("-march=nehalem (Targets cases: 0x49, 0x4e, 0x78-0x87)\n");
    /* Intel Nehalem: L1: 32KB, L2: 256KB, L3: 4MB-8MB */
    run_cache_benchmark("Nehalem", 8192);
#endif

#ifdef USE_MULTIVERSION
    printf("Using multi-versioning (target_clones)\n");
    multi_version_benchmark();
#endif

#ifndef TEST_PENTIUM3
#ifndef TEST_PENTIUM4
#ifndef TEST_NOCONA
#ifndef TEST_K8
#ifndef TEST_CORE2
#ifndef TEST_NEHALEM
#ifndef USE_MULTIVERSION
    /* Default: run generic benchmark */
    printf("Generic x86-64 (will use native CPU detection)\n");
    run_cache_benchmark("Generic", 1024);
    run_cache_benchmark("Generic", 8192);
#endif
#endif
#endif
#endif
#endif
#endif
#endif

    /* Additional test to ensure all code paths are used */
    volatile int final_check = 0;
    
    /* Array sizes that match specific cache cases */
    int test_sizes[] = {8, 16, 24, 32, 128, 256, 384, 512, 1024, 2048, 3072, 4096, 6144};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int* temp_buf = (int*)malloc(test_sizes[i] * 1024);
        if (temp_buf) {
            /* Touch memory to ensure allocation */
            for (int j = 0; j < test_sizes[i] * 1024 / sizeof(int); j += 64) {
                temp_buf[j] = j;
                final_check ^= temp_buf[j];
            }
            free(temp_buf);
        }
    }
    
    printf("Final checksum: %d\n", final_check);
    printf("=== Test complete ===\n");
    
    return 0;
}

/* Cache thrashing implementation */
static void cache_thrash_benchmark(int iterations, int buffer_size_kb) {
    size_t elements = buffer_size_kb * 1024 / sizeof(int);
    int* data = (int*)malloc(elements * sizeof(int));
    
    if (!data) return;
    
    /* Initialize */
    for (size_t i = 0; i < elements; i++) {
        data[i] = (int)(i * 3);
    }
    
    volatile int sink = 0;
    
    /* Different access patterns to trigger various cache behaviors */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pattern 1: Sequential with different strides */
        for (size_t i = 0; i < elements; i += 1) {
            data[i] = data[i] * 2 + 1;
        }
        MB();
        
        /* Pattern 2: Larger stride (cache line size) */
        for (size_t i = 0; i < elements; i += 16) {  /* 64 bytes if int=4 */
            data[i] = data[i] * 3 - 2;
        }
        MB();
        
        /* Pattern 3: Prime number stride for associativity test */
        for (size_t i = 0; i < elements; i += 17) {
            data[i] = data[i] * 5 + 3;
        }
        MB();
        
        /* Collect results to prevent elimination */
        for (size_t i = 0; i < elements; i += 128) {
            sink ^= data[i];
        }
    }
    
    free(data);
}
