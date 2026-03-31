/*
 * test_target.c - Comprehensive test to trigger GCC driver's CPUID cache detection
 * Specifically targets the large switch-case block in driver-i386.cc (lines 127-244)
 * 
 * Compilation recommendations:
 *   gcc -O2 -march=native -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target
 *   gcc -O2 -m32 -march=i686 -mtune=generic -fprofile-arcs -ftest-coverage test_target.c -o test_target_32
 *   gcc -O3 -march=native -mtune=native -fprofile-arcs -ftest-coverage -fno-inline -funroll-loops test_target.c -o test_target_aggressive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)

/* ========== PATTERN 1: Multiple target attributes for different Intel architectures ========== */

/* Function with target attribute for Core 2 - may trigger cache descriptors: 0x66, 0x67, 0x68, etc. */
__attribute__((target("arch=core2")))
void core2_optimized_compute(int *array, size_t size) {
    /* Use prefetch hints that might encourage cache model queries */
    for (size_t i = 0; i < size; i += 16) {
        __builtin_prefetch(&array[i + 32], 0, 3); /* High temporal locality */
    }
    
    /* Computation that could benefit from Core 2 cache characteristics */
    int sum = 0;
    for (size_t i = 0; i < size; i++) {
        array[i] = array[i] * 3 + 1;
        sum += array[i];
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(sum) : : "memory");
}

/* Function with target attribute for Nehalem - may trigger cache descriptors: 0x0a, 0x0c, 0x0d, etc. */
__attribute__((target("arch=nehalem")))
void nehalem_optimized_compute(int *array, size_t size) {
    /* Different access pattern that might trigger different cache detection */
    for (size_t i = 0; i < size; i += 8) {
        __builtin_prefetch(&array[i + 16], 0, 1); /* Low temporal locality */
    }
    
    int sum = 0;
    for (size_t i = 0; i < size; i += 2) {
        array[i] = array[i] * 2 - 1;
        sum += array[i];
    }
    
    asm volatile("" : "+r"(sum) : : "memory");
}

/* Function with target attribute for Sandy Bridge - may trigger cache descriptors: 0x2c, 0x3a, 0x3b, etc. */
__attribute__((target("arch=sandybridge")))
void sandybridge_optimized_compute(int *array, size_t size) {
    /* AVX-friendly alignment */
    for (size_t i = 0; i < size; i += 32) {
        __builtin_prefetch(&array[i + 64], 0, 2); /* Medium temporal locality */
    }
    
    int sum = 0;
    for (size_t i = 0; i < size; i++) {
        array[i] = (array[i] << 1) | 1;
        sum ^= array[i];
    }
    
    asm volatile("" : "+r"(sum) : : "memory");
}

/* Function with target attribute for Ivy Bridge - may trigger cache descriptors: 0x3c, 0x3d, 0x3e, etc. */
__attribute__((target("arch=ivybridge")))
void ivybridge_optimized_compute(int *array, size_t size) {
    /* Different stride pattern */
    for (size_t i = 0; i < size; i += 64) {
        __builtin_prefetch(&array[i + 128], 1, 3); /* Write prefetch */
    }
    
    int sum = 0;
    for (size_t i = 0; i < size; i += 4) {
        array[i] = array[i] + array[i+1];
        sum += array[i];
    }
    
    asm volatile("" : "+r"(sum) : : "memory");
}

/* ========== PATTERN 2: ifunc resolver for runtime dispatch ========== */

/* Default implementation */
static void default_compute(int *array, size_t size) {
    int sum = 0;
    for (size_t i = 0; i < size; i++) {
        array[i] = array[i] * 5;
        sum += array[i];
    }
    asm volatile("" : "+r"(sum) : : "memory");
}

/* Resolver function - forces CPU detection during compilation */
static void (*resolve_compute(void)) (int *, size_t) {
    /* These builtins cause GCC to initialize CPU detection */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        return sandybridge_optimized_compute;
    } else if (__builtin_cpu_supports("avx")) {
        return sandybridge_optimized_compute;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return nehalem_optimized_compute;
    } else if (__builtin_cpu_supports("sse4.1")) {
        return nehalem_optimized_compute;
    } else if (__builtin_cpu_supports("ssse3")) {
        return core2_optimized_compute;
    } else if (__builtin_cpu_supports("sse3")) {
        return core2_optimized_compute;
    }
    
    return default_compute;
}

/* ifunc function that will trigger resolver */
void ifunc_compute(int *array, size_t size) 
    __attribute__((ifunc("resolve_compute")));

/* ========== PATTERN 3: Function multiversioning ========== */

/* Function with target_clones attribute - forces driver to consider multiple architectures */
__attribute__((target_clones("default,arch=core2,arch=nehalem,arch=sandybridge,arch=ivybridge")))
void multiversion_compute(int *array, size_t size) {
    /* Array sized to match specific cache sizes from the switch cases */
    static int cache_sized_buffer[2048 * 1024 / sizeof(int)]; /* 2MB - matches some L2 sizes */
    
    /* Mix of operations that might trigger different optimizations */
    int sum = 0;
    for (size_t i = 0; i < size && i < sizeof(cache_sized_buffer)/sizeof(cache_sized_buffer[0]); i++) {
        /* Access pattern with different strides to test various cache line sizes */
        array[i] = array[i] + cache_sized_buffer[i % 1024];
        sum += array[i];
        
        /* Alternate between 32-byte and 64-byte aligned accesses */
        if (i % 32 == 0) {
            __builtin_prefetch(&array[(i + 64) % size], 0, 0);
        }
    }
    
    asm volatile("" : "+r"(sum) : : "memory");
}

/* ========== PATTERN 4: Direct CPUID inline assembly ========== */

static void query_cpuid_cache_info(void) {
    uint32_t eax, ebx, ecx, edx;
    
    /* CPUID leaf 2 - Cache descriptors (may return the bytes in the switch) */
    asm volatile ("cpuid" 
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                  : "a"(2), "c"(0));
    
    /* CPUID leaf 4 - Deterministic cache parameters */
    for (int i = 0; ; i++) {
        asm volatile ("cpuid" 
                      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) 
                      : "a"(4), "c"(i));
        
        int cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break; /* No more caches */
        }
        
        /* The values here may cause GCC to sync its internal cache model */
        int cache_level = (eax >> 5) & 0x7;
        int line_size = (ebx & 0xFFF) + 1;
        int partitions = ((ebx >> 12) & 0x3FF) + 1;
        int associativity = ((ebx >> 22) & 0x3FF) + 1;
        int sets = ecx + 1;
        
        (void)cache_level;
        (void)line_size;
        (void)partitions;
        (void)associativity;
        (void)sets;
    }
}

/* ========== PATTERN 5: Constructor that runs before main ========== */

__attribute__((constructor))
static void init_cpu_detection(void) {
    /* Force CPU initialization early */
    __builtin_cpu_init();
    
    /* Query CPUID directly */
    query_cpuid_cache_info();
    
    /* Check for various CPU features - each may trigger cache detection */
    int has_sse2 = __builtin_cpu_supports("sse2");
    int has_sse3 = __builtin_cpu_supports("sse3");
    int has_ssse3 = __builtin_cpu_supports("ssse3");
    int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    
    (void)has_sse2;
    (void)has_sse3;
    (void)has_ssse3;
    (void)has_sse4_1;
    (void)has_sse4_2;
    (void)has_avx;
    (void)has_avx2;
}

/* ========== PATTERN 6: Cache-sensitive computation ========== */

/* Matrix multiplication tuned for different cache sizes */
static void cache_sensitive_matrix_multiply(int size) {
    /* Allocate matrices with sizes that match cache parameters from switch */
    int *A = malloc(size * size * sizeof(int));
    int *B = malloc(size * size * sizeof(int));
    int *C = malloc(size * size * sizeof(int));
    
    if (!A || !B || !C) {
        free(A); free(B); free(C);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < size * size; i++) {
        A[i] = i % 100;
        B[i] = (i + 1) % 100;
        C[i] = 0;
    }
    
    /* Blocked matrix multiplication - block size chosen based on common cache line sizes */
    int block_size = 32; /* Try 32 first (matches some L1 cache lines) */
    
    /* Also try 64 in another loop (matches other cache lines in switch) */
    for (int bs = 32; bs <= 64; bs *= 2) {
        block_size = bs;
        
        for (int i = 0; i < size; i += block_size) {
            for (int j = 0; j < size; j += block_size) {
                for (int k = 0; k < size; k += block_size) {
                    /* Mini block matrix multiply */
                    for (int ii = i; ii < i + block_size && ii < size; ii++) {
                        for (int jj = j; jj < j + block_size && jj < size; jj++) {
                            int sum = C[ii * size + jj];
                            for (int kk = k; kk < k + block_size && kk < size; kk++) {
                                sum += A[ii * size + kk] * B[kk * size + jj];
                            }
                            C[ii * size + jj] = sum;
                        }
                    }
                }
            }
        }
        
        /* Use result to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < size * size; i += 128) {
            checksum ^= C[i];
        }
        asm volatile("" : "+r"(checksum) : : "memory");
    }
    
    free(A); free(B); free(C);
}

/* ========== Main function with extensive CPU checks ========== */

int main(void) {
    printf("Starting cache detection test...\n");
    
    /* PATTERN B: Extensive use of __builtin_cpu_is and __builtin_cpu_supports */
    __builtin_cpu_init();
    
    /* Check for specific Intel CPUs - each may have different cache descriptors */
    if (__builtin_cpu_is("intel")) {
        printf("CPU vendor: Intel\n");
        
        /* Check various Intel microarchitectures */
        const char *archs[] = {
            "core2", "nehalem", "sandybridge", "ivybridge", 
            "haswell", "broadwell", "skylake", "kabylake"
        };
        
        for (size_t i = 0; i < sizeof(archs)/sizeof(archs[0]); i++) {
            if (__builtin_cpu_is(archs[i])) {
                printf("Detected microarchitecture: %s\n", archs[i]);
            }
        }
    }
    
    /* Check cache line size at runtime */
    long cache_line = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    printf("L1 cache line size: %ld bytes\n", cache_line);
    
    /* Create test data with sizes matching cache parameters from switch cases */
    const size_t test_sizes[] = {
        8 * 1024 / sizeof(int),    /* 8KB - matches 0x0a, 0x66 */
        16 * 1024 / sizeof(int),   /* 16KB - matches 0x0c, 0x0d, 0x67 */
        24 * 1024 / sizeof(int),   /* 24KB - matches 0x0e */
        32 * 1024 / sizeof(int),   /* 32KB - matches 0x2c, 0x68 */
        64 * 1024 / sizeof(int),   /* 64KB */
        128 * 1024 / sizeof(int),  /* 128KB - matches 0x39, 0x3b, 0x41, 0x79 */
        256 * 1024 / sizeof(int),  /* 256KB - matches 0x21, 0x3c, 0x42, 0x7a, 0x82 */
        512 * 1024 / sizeof(int),  /* 512KB - matches 0x3e, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
        1024 * 1024 / sizeof(int), /* 1MB - matches 0x24, 0x78, 0x7c, 0x84 */
        2048 * 1024 / sizeof(int), /* 2MB - matches 0x45, 0x7d, 0x85 */
        4096 * 1024 / sizeof(int), /* 4MB - matches 0x49 */
        6144 * 1024 / sizeof(int), /* 6MB - matches 0x4e */
        8192 * 1024 / sizeof(int)  /* 8MB */
    };
    
    /* Run computations with different target attributes */
    for (size_t s = 0; s < sizeof(test_sizes)/sizeof(test_sizes[0]); s++) {
        size_t size = test_sizes[s] > 10000 ? 10000 : test_sizes[s];
        int *array = malloc(size * sizeof(int));
        
        if (!array) continue;
        
        /* Initialize array */
        for (size_t i = 0; i < size; i++) {
            array[i] = (i * 3) % 100;
        }
        
        /* Call all variants to trigger different code paths */
        core2_optimized_compute(array, size);
        nehalem_optimized_compute(array, size);
        sandybridge_optimized_compute(array, size);
        ivybridge_optimized_compute(array, size);
        
        /* Call ifunc version */
        ifunc_compute(array, size);
        
        /* Call multiversion version */
        multiversion_compute(array, size);
        
        free(array);
    }
    
    /* Run cache-sensitive matrix multiplication */
    cache_sensitive_matrix_multiply(128);
    
    /* Final CPUID query */
    query_cpuid_cache_info();
    
    printf("Test completed.\n");
    return 0;
}

#else /* Non-x86 fallback */

int main(void) {
    printf("This test is for x86 architectures only.\n");
    return 0;
}

#endif /* __i386__ || __x86_64__ */
