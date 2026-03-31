/* test_cache_coverage.c - Comprehensive test to cover CPUID leaf 2 cache descriptor cases */
/* Compile with different -D flags and -march options to target specific CPUs */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function template */
static void cache_thrash_benchmark(int *buffer, size_t size, int iterations) {
    volatile int sink = 0;
    size_t i, j;
    
    /* Simple linear congruential generator for pseudo-random access */
    uint32_t state = 0xDEADBEEF;
    
    for (j = 0; j < iterations; j++) {
        /* Sequential access pattern */
        for (i = 0; i < size; i++) {
            buffer[i] = i;
        }
        MEMORY_BARRIER();
        
        /* Pseudo-random access pattern to defeat prefetching */
        state = 1664525 * state + 1013904223;
        for (i = 0; i < size; i++) {
            size_t idx = (state + i) % size;
            buffer[idx] += buffer[(idx + 1) % size];
            state = 1664525 * state + 1013904223;
        }
        MEMORY_BARRIER();
        
        /* Another pattern: stride access */
        for (i = 0; i < size; i += 64) {  /* 64-byte cache line typical */
            buffer[i] *= 3;
        }
        MEMORY_BARRIER();
    }
    
    /* Final computation to prevent elimination */
    for (i = 0; i < size; i++) {
        sink ^= buffer[i];
    }
    
    /* Use sink to prevent dead code elimination */
    if (sink == 0x12345678) {
        printf("Impossible!\n");
    }
}

/* ============================================================
   Architecture-specific function versions using target attributes
   ============================================================ */

/* Case 0x0a: 8KB L1, 2-way, 32-byte line - Pentium III, some Celeron */
#ifdef TEST_CASE_0x0A
__attribute__((target("arch=pentium3")))
void bench_pentium3(void) {
    size_t size = 256 * 1024;  /* 256KB - larger than L1, fits in L2 */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 100);
        free(buffer);
    }
}
#endif

/* Case 0x0c: 16KB L1, 4-way, 32-byte line - Pentium III, some Pentium 4 */
#ifdef TEST_CASE_0x0C
__attribute__((target("arch=pentium3")))
void bench_pentium3_16kb(void) {
    size_t size = 512 * 1024;  /* 512KB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 100);
        free(buffer);
    }
}
#endif

/* Case 0x0d: 16KB L1, 4-way, 64-byte line - Some Pentium 4 */
#ifdef TEST_CASE_0x0D
__attribute__((target("arch=pentium4")))
void bench_pentium4_16kb(void) {
    size_t size = 1024 * 1024;  /* 1MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 50);
        free(buffer);
    }
}
#endif

/* Case 0x0e: 24KB L1, 6-way, 64-byte line - Some Pentium 4 */
#ifdef TEST_CASE_0x0E
__attribute__((target("arch=pentium4")))
void bench_pentium4_24kb(void) {
    size_t size = 2 * 1024 * 1024;  /* 2MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 30);
        free(buffer);
    }
}
#endif

/* Case 0x21: 256KB L2, 8-way, 64-byte line - Pentium 4, some Xeon */
#ifdef TEST_CASE_0x21
__attribute__((target("arch=pentium4")))
void bench_pentium4_256kb_l2(void) {
    size_t size = 4 * 1024 * 1024;  /* 4MB - larger than L2 */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 20);
        free(buffer);
    }
}
#endif

/* Case 0x24: 1MB L2, 16-way, 64-byte line - Some Xeon */
#ifdef TEST_CASE_0x24
__attribute__((target("arch=nocona")))
void bench_nocona_1mb_l2(void) {
    size_t size = 8 * 1024 * 1024;  /* 8MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

/* Case 0x2c: 32KB L1, 8-way, 64-byte line - Intel Core, some Xeon */
#ifdef TEST_CASE_0x2C
__attribute__((target("arch=core2")))
void bench_core2_32kb_l1(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 5);
        free(buffer);
    }
}
#endif

/* Case 0x39: 128KB L2, 4-way, 64-byte line - Some Pentium III */
#ifdef TEST_CASE_0x39
__attribute__((target("arch=pentium3")))
void bench_pentium3_128kb_l2(void) {
    size_t size = 2 * 1024 * 1024;  /* 2MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 30);
        free(buffer);
    }
}
#endif

/* Case 0x3a: 192KB L2, 6-way, 64-byte line - Some Pentium 4 */
#ifdef TEST_CASE_0x3A
__attribute__((target("arch=pentium4")))
void bench_pentium4_192kb_l2(void) {
    size_t size = 4 * 1024 * 1024;  /* 4MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 20);
        free(buffer);
    }
}
#endif

/* Case 0x3b: 128KB L2, 2-way, 64-byte line - Celeron, some AMD */
#ifdef TEST_CASE_0x3B
__attribute__((target("arch=k6")))
void bench_k6_128kb_l2(void) {
    size_t size = 2 * 1024 * 1024;  /* 2MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 30);
        free(buffer);
    }
}
#endif

/* Case 0x3c: 256KB L2, 4-way, 64-byte line - Pentium M, some AMD */
#ifdef TEST_CASE_0x3C
__attribute__((target("arch=pentium-m")))
void bench_pentium_m_256kb_l2(void) {
    size_t size = 4 * 1024 * 1024;  /* 4MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 20);
        free(buffer);
    }
}
#endif

/* Case 0x3d: 384KB L2, 6-way, 64-byte line - Some Pentium 4 */
#ifdef TEST_CASE_0x3D
__attribute__((target("arch=pentium4")))
void bench_pentium4_384kb_l2(void) {
    size_t size = 8 * 1024 * 1024;  /* 8MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 15);
        free(buffer);
    }
}
#endif

/* Case 0x3e: 512KB L2, 4-way, 64-byte line - Pentium D, some Xeon */
#ifdef TEST_CASE_0x3E
__attribute__((target("arch=prescott")))
void bench_prescott_512kb_l2(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

/* Case 0x41: 128KB L2, 4-way, 32-byte line - Some early CPUs */
#ifdef TEST_CASE_0x41
__attribute__((target("arch=pentium2")))
void bench_pentium2_128kb_l2(void) {
    size_t size = 2 * 1024 * 1024;  /* 2MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 30);
        free(buffer);
    }
}
#endif

/* Case 0x42: 256KB L2, 4-way, 32-byte line - Pentium II, some AMD */
#ifdef TEST_CASE_0x42
__attribute__((target("arch=pentium2")))
void bench_pentium2_256kb_l2(void) {
    size_t size = 4 * 1024 * 1024;  /* 4MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 20);
        free(buffer);
    }
}
#endif

/* Case 0x43: 512KB L2, 4-way, 32-byte line - Pentium III, some AMD */
#ifdef TEST_CASE_0x43
__attribute__((target("arch=pentium3")))
void bench_pentium3_512kb_l2(void) {
    size_t size = 8 * 1024 * 1024;  /* 8MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 15);
        free(buffer);
    }
}
#endif

/* Case 0x44: 1MB L2, 4-way, 32-byte line - Some Xeon */
#ifdef TEST_CASE_0x44
__attribute__((target("arch=nocona")))
void bench_nocona_1mb_l2_32b(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

/* Case 0x45: 2MB L2, 4-way, 32-byte line - Some Xeon MP */
#ifdef TEST_CASE_0x45
__attribute__((target("arch=nocona")))
void bench_nocona_2mb_l2(void) {
    size_t size = 32 * 1024 * 1024;  /* 32MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 5);
        free(buffer);
    }
}
#endif

/* Case 0x48: 3MB L2, 12-way, 64-byte line - Some Xeon */
#ifdef TEST_CASE_0x48
__attribute__((target("arch=core2")))
void bench_core2_3mb_l2(void) {
    size_t size = 32 * 1024 * 1024;  /* 32MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 5);
        free(buffer);
    }
}
#endif

/* Case 0x49: 4MB L2, 16-way, 64-byte line - Xeon DP (not MP) */
/* Targeting nocona which is Xeon DP, not Xeon MP */
#ifdef TEST_CASE_0x49
__attribute__((target("arch=nocona")))
void bench_nocona_4mb_l2(void) {
    size_t size = 64 * 1024 * 1024;  /* 64MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 3);
        free(buffer);
    }
}
#endif

/* Case 0x4e: 6MB L2, 24-way, 64-byte line - Some Xeon */
#ifdef TEST_CASE_0x4E
__attribute__((target("arch=core2")))
void bench_core2_6mb_l2(void) {
    size_t size = 64 * 1024 * 1024;  /* 64MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 3);
        free(buffer);
    }
}
#endif

/* Case 0x60: 16KB L1, 8-way, 64-byte line - Some Xeon */
#ifdef TEST_CASE_0x60
__attribute__((target("arch=nocona")))
void bench_nocona_16kb_l1(void) {
    size_t size = 128 * 1024;  /* 128KB - fits in L2 */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 100);
        free(buffer);
    }
}
#endif

/* Case 0x66: 8KB L1, 4-way, 64-byte line - Some VIA, AMD */
#ifdef TEST_CASE_0x66
__attribute__((target("arch=k8")))
void bench_k8_8kb_l1(void) {
    size_t size = 256 * 1024;  /* 256KB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 100);
        free(buffer);
    }
}
#endif

/* Case 0x67: 16KB L1, 4-way, 64-byte line - Some AMD */
#ifdef TEST_CASE_0x67
__attribute__((target("arch=k8")))
void bench_k8_16kb_l1(void) {
    size_t size = 512 * 1024;  /* 512KB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 80);
        free(buffer);
    }
}
#endif

/* Case 0x68: 32KB L1, 4-way, 64-byte line - Some AMD */
#ifdef TEST_CASE_0x68
__attribute__((target("arch=k10")))
void bench_k10_32kb_l1(void) {
    size_t size = 1024 * 1024;  /* 1MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 60);
        free(buffer);
    }
}
#endif

/* Cases 0x78-0x87: Various L2 cache configurations */
#ifdef TEST_CASE_0x78
__attribute__((target("arch=k8")))
void bench_k8_1mb_l2(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x79
__attribute__((target("arch=k8")))
void bench_k8_128kb_l2_8way(void) {
    size_t size = 4 * 1024 * 1024;  /* 4MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 20);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x7A
__attribute__((target("arch=k8")))
void bench_k8_256kb_l2_8way(void) {
    size_t size = 8 * 1024 * 1024;  /* 8MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 15);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x7B
__attribute__((target("arch=k8")))
void bench_k8_512kb_l2_8way(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x7C
__attribute__((target("arch=k8")))
void bench_k8_1mb_l2_8way(void) {
    size_t size = 32 * 1024 * 1024;  /* 32MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 8);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x7D
__attribute__((target("arch=k10")))
void bench_k10_2mb_l2_8way(void) {
    size_t size = 64 * 1024 * 1024;  /* 64MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 5);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x7F
__attribute__((target("arch=k8")))
void bench_k8_512kb_l2_2way(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x80
__attribute__((target("arch=k8")))
void bench_k8_512kb_l2_8way_alt(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x82
__attribute__((target("arch=k8")))
void bench_k8_256kb_l2_8way_32b(void) {
    size_t size = 8 * 1024 * 1024;  /* 8MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 15);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x83
__attribute__((target("arch=k8")))
void bench_k8_512kb_l2_8way_32b(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x84
__attribute__((target("arch=k8")))
void bench_k8_1mb_l2_8way_32b(void) {
    size_t size = 32 * 1024 * 1024;  /* 32MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 8);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x85
__attribute__((target("arch=k10")))
void bench_k10_2mb_l2_8way_32b(void) {
    size_t size = 64 * 1024 * 1024;  /* 64MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 5);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x86
__attribute__((target("arch=k8")))
void bench_k8_512kb_l2_4way(void) {
    size_t size = 16 * 1024 * 1024;  /* 16MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

#ifdef TEST_CASE_0x87
__attribute__((target("arch=k8")))
void bench_k8_1mb_l2_8way_alt(void) {
    size_t size = 32 * 1024 * 1024;  /* 32MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 8);
        free(buffer);
    }
}
#endif

/* ============================================================
   Main function with multi-versioning support
   ============================================================ */

/* Function with multiple target clones for comprehensive coverage */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("pentium3, pentium4, nocona, core2, k8, k10")))
static void multi_version_bench(void) {
    size_t size = 8 * 1024 * 1024;  /* 8MB */
    int *buffer = malloc(size * sizeof(int));
    if (buffer) {
        cache_thrash_benchmark(buffer, size, 10);
        free(buffer);
    }
}
#endif

int main(int argc, char **argv) {
    printf("Cache descriptor coverage test\n");
    printf("==============================\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Call architecture-specific benchmarks based on compile-time defines */
    
#ifdef TEST_CASE_0x0A
    bench_pentium3();
#endif
    
#ifdef TEST_CASE_0x0C
    bench_pentium3_16kb();
#endif
    
#ifdef TEST_CASE_0x0D
    bench_pentium4_16kb();
#endif
    
#ifdef TEST_CASE_0x0E
    bench_pentium4_24kb();
#endif
    
#ifdef TEST_CASE_0x21
    bench_pentium4_256kb_l2();
#endif
    
#ifdef TEST_CASE_0x24
    bench_nocona_1mb_l2();
#endif
    
#ifdef TEST_CASE_0x2C
    bench_core2_32kb_l1();
#endif
    
#ifdef TEST_CASE_0x39
    bench_pentium3_128kb_l2();
#endif
    
#ifdef TEST_CASE_0x3A
    bench_pentium4_192kb_l2();
#endif
    
#ifdef TEST_CASE_0x3B
    bench_k6_128kb_l2();
#endif
    
#ifdef TEST_CASE_0x3C
    bench_pentium_m_256kb_l2();
#endif
    
#ifdef TEST_CASE_0x3D
    bench_pentium4_384kb_l2();
#endif
    
#ifdef TEST_CASE_0x3E
    bench_prescott_512kb_l2();
#endif
    
#ifdef TEST_CASE_0x41
    bench_pentium2_128kb_l2();
#endif
    
#ifdef TEST_CASE_0x42
    bench_pentium2_256kb_l2();
#endif
    
#ifdef TEST_CASE_0x43
    bench_pentium3_512kb_l2();
#endif
    
#ifdef TEST_CASE_0x44
    bench_nocona_1mb_l2_32b();
#endif
    
#ifdef TEST_CASE_0x45
    bench_nocona_2mb_l2();
#endif
    
#ifdef TEST_CASE_0x48
    bench_core2_3mb_l2();
#endif
    
#ifdef TEST_CASE_0x49
    bench_nocona_4mb_l2();
#endif
    
#ifdef TEST_CASE_0x4E
    bench_core2_6mb_l2();
#endif
    
#ifdef TEST_CASE_0x60
    bench_nocona_16kb_l1();
#endif
    
#ifdef TEST_CASE_0x66
    bench_k8_8kb_l1();
#endif
    
#ifdef TEST_CASE_0x67
    bench_k8_16kb_l1();
#endif
    
#ifdef TEST_CASE_0x68
    bench_k10_32kb_l1();
#endif
    
#ifdef TEST_CASE_0x78
    bench_k8_1mb_l2();
#endif
    
#ifdef TEST_CASE_0x79
    bench_k8_128kb_l2_8way();
#endif
    
#ifdef TEST_CASE_0x7A
    bench_k8_256kb_l2_8way();
#endif
    
#ifdef TEST_CASE_0x7B
    bench_k8_512kb_l2_8way();
#endif
    
#ifdef TEST_CASE_0x7C
    bench_k8_1mb_l2_8way();
#endif
    
#ifdef TEST_CASE_0x7D
    bench_k10_2mb_l2_8way();
#endif
    
#ifdef TEST_CASE_0x7F
    bench_k8_512kb_l2_2way();
#endif
    
#ifdef TEST_CASE_0x80
    bench_k8_512kb_l2_8way_alt();
#endif
    
#ifdef TEST_CASE_0x82
    bench_k8_256kb_l2_8way_32b();
#endif
    
#ifdef TEST_CASE_0x83
    bench_k8_512kb_l2_8way_32b();
#endif
    
#ifdef TEST_CASE_0x84
    bench_k8_1mb_l2_8way_32b();
#endif
    
#ifdef TEST_CASE_0x85
    bench_k10_2mb_l2_8way_32b();
#endif
    
#ifdef TEST_CASE_0x86
    bench_k8_512kb_l2_4way();
#endif
    
#ifdef TEST_CASE_0x87
    bench_k8_1mb_l2_8way_alt();
#endif
    
#ifdef USE_MULTIVERSIONING
    multi_version_bench();
#endif
    
    /* Generic benchmark that should work on any architecture */
    {
        size_t generic_size = 4 * 1024 * 1024;  /* 4MB */
        int *generic_buffer = malloc(generic_size * sizeof(int));
        if (generic_buffer) {
            printf("Running generic benchmark...\n");
            cache_thrash_benchmark(generic_buffer, generic_size, 20);
            free(generic_buffer);
        }
    }
    
    printf("Test completed.\n");
    return 0;
}
