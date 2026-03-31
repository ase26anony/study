/* test_cache_coverage.c - Comprehensive test for GCC i386 driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different CPU targets to trigger various cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
/* Targets: 0x49 (Xeon DP, not MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
/* Targets: 0x78-0x87 */
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
/* Targets: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#endif
static void benchmark_cache(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int element_count = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(element_count * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (int i = 0; i < element_count; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Cache-thrashing benchmark */
    for (int iter = 0; iter < iterations; iter++) {
        int index = 0;
        int stride = 17; /* Prime stride to avoid cache line conflicts */
        
        for (int i = 0; i < element_count * 4; i++) {
            index = (index + stride) % element_count;
            buffer[index] = buffer[index] * 3 + 1;
            MEMORY_BARRIER();
        }
    }
    
    /* Compute final result to prevent dead code elimination */
    for (int i = 0; i < element_count; i += 64) {
        result ^= buffer[i];
    }
    
    printf("Benchmark result: %d\n", result);
    free(buffer);
}

/* Function variants for different cache descriptor cases */
#ifdef COVER_CASE_0x0A
__attribute__((target("arch=pentium3")))
void test_case_0x0a(void) {
    /* Pentium III Tualatin: L1 Data Cache: 8KB, 2-way, 32-byte line */
    benchmark_cache(100, 8192); /* Use 8MB to exceed L2 */
}
#endif

#ifdef COVER_CASE_0x0C
__attribute__((target("arch=pentium3")))
void test_case_0x0c(void) {
    /* Pentium III Coppermine: L1 Data Cache: 16KB, 4-way, 32-byte line */
    benchmark_cache(100, 16384);
}
#endif

#ifdef COVER_CASE_0x21
__attribute__((target("arch=pentium3")))
void test_case_0x21(void) {
    /* Pentium III: L2 Cache: 256KB, 8-way, 64-byte line */
    benchmark_cache(50, 262144);
}
#endif

#ifdef COVER_CASE_0x2C
__attribute__((target("arch=pentium4")))
void test_case_0x2c(void) {
    /* Pentium 4: L1 Data Cache: 32KB, 8-way, 64-byte line */
    benchmark_cache(100, 32768);
}
#endif

#ifdef COVER_CASE_0x39
__attribute__((target("arch=pentium4")))
void test_case_0x39(void) {
    /* Pentium 4: L2 Cache: 128KB, 4-way, 64-byte line */
    benchmark_cache(50, 131072);
}
#endif

#ifdef COVER_CASE_0x49
__attribute__((target("arch=nocona")))
void test_case_0x49(void) {
    /* Xeon DP (not MP): L2 Cache: 4096KB, 16-way, 64-byte line */
    benchmark_cache(20, 4194304);
}
#endif

#ifdef COVER_CASE_0x60
__attribute__((target("arch=nocona")))
void test_case_0x60(void) {
    /* Pentium 4 with EM64T: L1 Data Cache: 16KB, 8-way, 64-byte line */
    benchmark_cache(100, 16384);
}
#endif

#ifdef COVER_CASE_0x78
__attribute__((target("arch=k8")))
void test_case_0x78(void) {
    /* AMD K8: L2 Cache: 1024KB, 4-way, 64-byte line */
    benchmark_cache(30, 1048576);
}
#endif

#ifdef COVER_CASE_0x7A
__attribute__((target("arch=k8")))
void test_case_0x7a(void) {
    /* AMD K8: L2 Cache: 256KB, 8-way, 64-byte line */
    benchmark_cache(50, 262144);
}
#endif

#ifdef COVER_CASE_0x82
__attribute__((target("arch=k8")))
void test_case_0x82(void) {
    /* AMD K8: L2 Cache: 256KB, 8-way, 32-byte line */
    benchmark_cache(50, 262144);
}
#endif

#ifdef COVER_CASE_0x48
__attribute__((target("arch=core2")))
void test_case_0x48(void) {
    /* Intel Core 2: L2 Cache: 3072KB, 12-way, 64-byte line */
    benchmark_cache(25, 3145728);
}
#endif

/* Main function with multi-versioning */
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2")))
int main(int argc, char **argv) {
    int test_iterations = 10;
    
    if (argc > 1) {
        test_iterations = atoi(argv[1]);
    }
    
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Force compiler to consider different cache configurations */
    volatile int mode = 0;
    
    /* Test different buffer sizes to trigger various cache optimizations */
    int buffer_sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    int num_sizes = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int size_kb = buffer_sizes[i];
        
        /* Vary access patterns based on buffer size */
        int *data = (int*)malloc(size_kb * 1024);
        if (!data) continue;
        
        int elements = (size_kb * 1024) / sizeof(int);
        
        /* Different access patterns to stress cache */
        for (int pattern = 0; pattern < 3; pattern++) {
            int sum = 0;
            
            switch (pattern) {
                case 0: /* Sequential */
                    for (int j = 0; j < elements; j++) {
                        data[j] = j;
                        sum += data[j];
                    }
                    break;
                    
                case 1: /* Strided */
                    for (int j = 0; j < elements; j += 16) {
                        data[j] = j * 3;
                        sum += data[j];
                    }
                    break;
                    
                case 2: /* Pseudo-random */
                    {
                        uint32_t rng = 123456789;
                        for (int j = 0; j < elements; j++) {
                            rng = rng * 1664525 + 1013904223;
                            int idx = rng % elements;
                            data[idx] = j;
                            sum += data[idx];
                        }
                    }
                    break;
            }
            
            MEMORY_BARRIER();
            mode = sum; /* Use result to prevent optimization */
        }
        
        free(data);
    }
    
    /* Call architecture-specific test functions if compiled with defines */
#ifdef COVER_CASE_0x0A
    test_case_0x0a();
#endif
#ifdef COVER_CASE_0x0C
    test_case_0x0c();
#endif
#ifdef COVER_CASE_0x21
    test_case_0x21();
#endif
#ifdef COVER_CASE_0x2C
    test_case_0x2c();
#endif
#ifdef COVER_CASE_0x39
    test_case_0x39();
#endif
#ifdef COVER_CASE_0x49
    test_case_0x49();
#endif
#ifdef COVER_CASE_0x60
    test_case_0x60();
#endif
#ifdef COVER_CASE_0x78
    test_case_0x78();
#endif
#ifdef COVER_CASE_0x7A
    test_case_0x7a();
#endif
#ifdef COVER_CASE_0x82
    test_case_0x82();
#endif
#ifdef COVER_CASE_0x48
    test_case_0x48();
#endif
    
    printf("Test completed.\n");
    return 0;
}
