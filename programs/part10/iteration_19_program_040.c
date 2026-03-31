/* test_cache_coverage.c - Cover GCC i386 driver cache detection cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away memory accesses */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Memory operation macros that force cache usage */
#define FORCE_CACHE_ACCESS(addr) (*(volatile char *)(addr))
#define CACHE_THRASH_LOOP(ptr, size, stride) do { \
    for (size_t i = 0; i < (size); i += (stride)) { \
        FORCE_CACHE_ACCESS((ptr) + i) = (char)i; \
    } \
    COMPILER_BARRIER(); \
} while(0)

/* Different benchmark functions targeting specific CPU cache configurations */

#ifdef TARGET_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
static void benchmark_pentium3(void) {
    printf("Running Pentium III cache benchmark...\n");
    
    /* L1 cache sizes: 8KB-24KB, L2: 256KB-1024KB */
    size_t l1_size = 24 * 1024;  /* Worst-case L1 */
    size_t l2_size = 1024 * 1024; /* Worst-case L2 */
    
    char *buffer1 = malloc(l2_size * 2);
    char *buffer2 = malloc(l2_size * 2);
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Pattern that should stress L1/L2 detection */
    for (int iter = 0; iter < 100; iter++) {
        /* Sequential access - tests line size */
        for (size_t i = 0; i < l1_size; i += 32) {
            buffer1[i] = buffer1[i] + 1;
        }
        
        /* Strided access - tests associativity */
        for (size_t i = 0; i < l2_size; i += 128) {
            buffer2[i] = buffer2[i] + 1;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile char result = buffer1[0] + buffer2[0];
    (void)result;
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef TARGET_PENTIUM4
/* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
static void benchmark_pentium4(void) {
    printf("Running Pentium 4 cache benchmark...\n");
    
    /* P4 L1: 8KB-32KB, L2: 128KB-2048KB */
    size_t l1_size = 32 * 1024;
    size_t l2_size = 2048 * 1024;
    
    int *buffer = malloc(l2_size * sizeof(int));
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Complex access pattern to defeat prefetching */
    uint32_t seed = 0x12345678;
    for (int iter = 0; iter < 50; iter++) {
        /* Pseudo-random access within L2 */
        for (int i = 0; i < 10000; i++) {
            seed = seed * 1103515245 + 12345;
            size_t idx = (seed >> 16) % (l2_size / sizeof(int));
            buffer[idx] = buffer[idx] + iter;
        }
        
        /* Sequential with different strides */
        for (size_t stride = 64; stride <= 256; stride *= 2) {
            for (size_t i = 0; i < l1_size; i += stride) {
                buffer[i / sizeof(int)] += 1;
            }
        }
        
        COMPILER_BARRIER();
    }
    
    volatile int sum = 0;
    for (size_t i = 0; i < l2_size / sizeof(int); i += 1024) {
        sum += buffer[i];
    }
    (void)sum;
    
    free(buffer);
}
#endif

#ifdef TARGET_NOCONA
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68, 0x78-0x87 */
__attribute__((target("arch=nocona")))
static void benchmark_nocona(void) {
    printf("Running Nocona/Xeon DP cache benchmark...\n");
    
    /* Nocona has larger caches: L2 up to 2MB+, L3 possible */
    size_t l2_size = 4096 * 1024;  /* 4MB for case 0x49 */
    size_t l3_size = 8192 * 1024;  /* 8MB if L3 present */
    
    double *buffer1 = malloc(l2_size);
    double *buffer2 = malloc(l3_size);
    
    if (!buffer1 || !buffer2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Use double for 64-bit operations - relevant for EM64T */
    for (int iter = 0; iter < 30; iter++) {
        /* Matrix-style access pattern */
        const size_t cols = 1024;
        const size_t rows = l2_size / (cols * sizeof(double));
        
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                size_t idx = i * cols + j;
                if (idx < l2_size / sizeof(double)) {
                    buffer1[idx] = buffer1[idx] * 1.01;
                }
            }
        }
        
        /* Transpose access pattern */
        for (size_t j = 0; j < cols; j++) {
            for (size_t i = 0; i < rows; i++) {
                size_t idx = i * cols + j;
                if (idx < l2_size / sizeof(double)) {
                    buffer2[idx] = buffer1[idx] + 0.5;
                }
            }
        }
        
        COMPILER_BARRIER();
    }
    
    volatile double check = buffer1[0] + buffer2[0];
    (void)check;
    
    free(buffer1);
    free(buffer2);
}
#endif

#ifdef TARGET_K8
/* Targets cases: 0x40 series, 0x78-0x87 (AMD-specific) */
__attribute__((target("arch=k8")))
static void benchmark_k8(void) {
    printf("Running AMD K8 cache benchmark...\n");
    
    /* AMD K8: L1 64KB, L2 512KB-1MB typically */
    size_t l1_size = 64 * 1024;
    size_t l2_size = 1024 * 1024;
    
    /* Use different data types to test various alignments */
    struct mixed_data {
        char c;
        int i;
        double d;
    };
    
    struct mixed_data *buffer = malloc(l2_size);
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    size_t elements = l2_size / sizeof(struct mixed_data);
    
    /* AMD benefits from prefetching - create predictable but non-sequential pattern */
    for (int iter = 0; iter < 40; iter++) {
        /* Forward and backward sweeps */
        for (size_t i = 0; i < elements; i++) {
            buffer[i].i = buffer[i].i + iter;
            buffer[i].d = buffer[i].d * 1.0001;
        }
        
        for (size_t i = elements; i > 0; i--) {
            buffer[i-1].c = buffer[i-1].c ^ (char)iter;
        }
        
        /* Every 8th element (cache line boundary testing) */
        for (size_t i = 0; i < elements; i += 8) {
            buffer[i].i = buffer[i].i * 2;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile int total = 0;
    for (size_t i = 0; i < elements; i += 16) {
        total += buffer[i].i;
    }
    (void)total;
    
    free(buffer);
}
#endif

#ifdef TARGET_CORE2
/* Targets cases: 0x66, 0x67, 0x68, and larger L2 cases */
__attribute__((target("arch=core2")))
static void benchmark_core2(void) {
    printf("Running Core 2 cache benchmark...\n");
    
    /* Core 2: L1 32KB, L2 up to 6MB (case 0x4e) */
    size_t l2_size = 6144 * 1024;  /* 6MB for case 0x4e */
    
    /* Use SSE-aligned data */
    float *aligned_buffer;
    if (posix_memalign((void**)&aligned_buffer, 16, l2_size)) {
        printf("Aligned allocation failed\n");
        return;
    }
    
    /* Initialize with pattern */
    size_t floats = l2_size / sizeof(float);
    for (size_t i = 0; i < floats; i++) {
        aligned_buffer[i] = (float)i;
    }
    
    /* Streaming SIMD-like operations */
    for (int iter = 0; iter < 25; iter++) {
        /* Block processing */
        const size_t block = 256;  /* 1KB blocks */
        for (size_t base = 0; base < floats; base += block) {
            size_t limit = base + block;
            if (limit > floats) limit = floats;
            
            float sum = 0.0f;
            for (size_t i = base; i < limit; i++) {
                sum += aligned_buffer[i];
                aligned_buffer[i] = aligned_buffer[i] * 0.999f;
            }
            
            /* Store result to prevent optimization */
            if (base < floats) {
                aligned_buffer[base] = sum;
            }
        }
        
        /* Reverse stride access */
        for (size_t stride = 1; stride <= 8; stride *= 2) {
            for (size_t i = 0; i < floats; i += stride) {
                aligned_buffer[i] = aligned_buffer[i] + 0.001f;
            }
        }
        
        COMPILER_BARRIER();
    }
    
    volatile float final = aligned_buffer[0];
    (void)final;
    
    free(aligned_buffer);
}
#endif

/* Generic benchmark that should work on any x86 */
static void benchmark_generic(void) {
    printf("Running generic x86 cache benchmark...\n");
    
    /* Conservative sizes that should work everywhere */
    size_t buffer_size = 1024 * 1024;  /* 1MB */
    int *buffer = malloc(buffer_size);
    
    if (!buffer) {
        printf("Memory allocation failed\n");
        return;
    }
    
    size_t elements = buffer_size / sizeof(int);
    
    /* Simple but effective cache thrashing */
    for (int iter = 0; iter < 100; iter++) {
        /* Linear access */
        for (size_t i = 0; i < elements; i++) {
            buffer[i] = buffer[i] + 1;
        }
        
        /* Every cache line (assuming 64 bytes) */
        for (size_t i = 0; i < elements; i += 16) {
            buffer[i] = buffer[i] * 3;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile int result = 0;
    for (size_t i = 0; i < elements; i += 128) {
        result ^= buffer[i];
    }
    (void)result;
    
    free(buffer);
}

int main(void) {
    printf("Starting cache detection coverage test...\n");
    
    /* Run all enabled benchmarks */
#ifdef TARGET_PENTIUM3
    benchmark_pentium3();
#endif
    
#ifdef TARGET_PENTIUM4
    benchmark_pentium4();
#endif
    
#ifdef TARGET_NOCONA
    benchmark_nocona();
#endif
    
#ifdef TARGET_K8
    benchmark_k8();
#endif
    
#ifdef TARGET_CORE2
    benchmark_core2();
#endif
    
    /* Always run generic benchmark */
    benchmark_generic();
    
    printf("Cache benchmark completed.\n");
    
    /* Force output to prevent optimization */
    volatile int dummy = 0;
    return dummy;
}
