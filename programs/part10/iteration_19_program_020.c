/* test_cache_descriptors.c - Comprehensive test for GCC i386 driver cache detection */
/* Compile with different -D flags and -march options to cover specific cases */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent aggressive optimization */
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Cache thrashing benchmark function */
static void cache_thrash(size_t size_kb, int iterations) {
    size_t elements = (size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    volatile int sink = 0;
    
    if (!buffer) return;
    
    /* Initialize with pseudo-random pattern */
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < elements; i++) {
        seed = seed * 1103515245 + 12345;
        buffer[i] = (int)(seed & 0x7FFFFFFF);
    }
    
    /* Cache thrashing benchmark */
    for (int iter = 0; iter < iterations; iter++) {
        seed = iter;
        for (size_t i = 0; i < elements; i++) {
            /* Simple LCG for access pattern */
            seed = seed * 1664525 + 1013904223;
            size_t idx = seed % elements;
            buffer[idx] = buffer[idx] * 3 + 1;
        }
        MEMORY_BARRIER();
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (size_t i = 0; i < elements; i++) {
        sink ^= buffer[i];
    }
    
    free(buffer);
    (void)sink; /* Use sink to prevent optimization */
}

/* Architecture-specific test functions with target attributes */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
void test_pentium3_cache(void) {
    printf("Testing Pentium III cache configuration...\n");
    /* Exercise different cache levels */
    cache_thrash(8, 1000);   /* L1 size for case 0x0a */
    cache_thrash(16, 1000);  /* L1 size for cases 0x0c, 0x0d */
    cache_thrash(256, 500);  /* L2 size for case 0x21 */
    cache_thrash(1024, 200); /* L2 size for case 0x24 */
}
#endif

#ifdef TEST_PENTIUM4
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x2c, 0x39-0x3e, 0x41-0x45 */
__attribute__((target("arch=pentium4")))
void test_pentium4_cache(void) {
    printf("Testing Pentium 4 cache configuration...\n");
    cache_thrash(8, 1000);    /* L1 for 0x0a */
    cache_thrash(16, 1000);   /* L1 for 0x0c, 0x0d */
    cache_thrash(32, 800);    /* L1 for 0x2c */
    cache_thrash(128, 400);   /* L2 for 0x39, 0x3b, 0x41 */
    cache_thrash(256, 300);   /* L2 for 0x3c, 0x42 */
    cache_thrash(512, 200);   /* L2 for 0x3e, 0x43 */
    cache_thrash(1024, 100);  /* L2 for 0x44 */
    cache_thrash(2048, 50);   /* L2 for 0x45 */
}
#endif

#ifdef TEST_NOCONA
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68, 0x78-0x87 */
__attribute__((target("arch=nocona")))
void test_nocona_cache(void) {
    printf("Testing Nocona (Xeon DP) cache configuration...\n");
    /* Nocona has 16KB L1 (case 0x60) and 1MB/2MB L2 */
    cache_thrash(16, 1000);   /* L1 for 0x60 */
    cache_thrash(1024, 100);  /* L2 for 0x78, 0x7c, 0x84, 0x87 */
    cache_thrash(2048, 50);   /* L2 for 0x7d, 0x85 */
    cache_thrash(4096, 25);   /* L2 for 0x49 (non-Xeon-MP) */
}
#endif

#ifdef TEST_K8
/* Targets cases: 0x40 series, 0x78-0x87 (AMD K8) */
__attribute__((target("arch=k8")))
void test_k8_cache(void) {
    printf("Testing AMD K8 (Athlon64) cache configuration...\n");
    /* K8 has 64KB L1 and 512KB-1MB L2 */
    cache_thrash(64, 800);    /* L1 */
    cache_thrash(512, 200);   /* L2 for 0x7b, 0x7f, 0x80, 0x83, 0x86 */
    cache_thrash(1024, 100);  /* L2 for 0x7c, 0x84, 0x87 */
}
#endif

#ifdef TEST_CORE2
/* Targets cases: 0x49 (if not Xeon MP), 0x66-0x68, 0x78-0x87 */
__attribute__((target("arch=core2")))
void test_core2_cache(void) {
    printf("Testing Core 2 cache configuration...\n");
    cache_thrash(32, 800);    /* L1 for 0x68 */
    cache_thrash(64, 600);    /* L1 */
    cache_thrash(2048, 50);   /* L2 for 0x7d, 0x85 */
    cache_thrash(4096, 25);   /* L2 for 0x49 (non-Xeon-MP) */
    cache_thrash(6144, 15);   /* L2 for 0x4e */
}
#endif

#ifdef TEST_NEHALEM
/* Targets cases: 0x49, 0x4e, 0x60, 0x78-0x87 */
__attribute__((target("arch=nehalem")))
void test_nehalem_cache(void) {
    printf("Testing Nehalem cache configuration...\n");
    cache_thrash(32, 800);    /* L1 */
    cache_thrash(256, 300);   /* L2 */
    cache_thrash(8192, 10);   /* L3 - triggers large cache detection */
}
#endif

/* Multi-versioned function that will be compiled for multiple targets */
#if defined(USE_MULTIVERSION) && __GNUC__ >= 4
__attribute__((target_clones("pentium3, pentium4, nocona, k8, core2, nehalem")))
void multiversion_cache_test(void) {
    printf("Multi-version cache test running...\n");
    cache_thrash(1024, 100);
    cache_thrash(2048, 50);
    cache_thrash(4096, 25);
}
#endif

/* Main test routine */
int main(void) {
    printf("Cache Descriptor Test Program\n");
    printf("=============================\n");
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Run architecture-specific tests based on compile-time defines */
#ifdef TEST_PENTIUM3
    test_pentium3_cache();
#endif
    
#ifdef TEST_PENTIUM4
    test_pentium4_cache();
#endif
    
#ifdef TEST_NOCONA
    test_nocona_cache();
#endif
    
#ifdef TEST_K8
    test_k8_cache();
#endif
    
#ifdef TEST_CORE2
    test_core2_cache();
#endif
    
#ifdef TEST_NEHALEM
    test_nehalem_cache();
#endif
    
#if defined(USE_MULTIVERSION) && __GNUC__ >= 4
    multiversion_cache_test();
#endif
    
    /* Generic cache test that should work on any architecture */
    printf("\nRunning generic cache test...\n");
    
    /* Test various cache sizes to potentially trigger different cases */
    const size_t test_sizes[] = {
        8,    /* case 0x0a */
        16,   /* cases 0x0c, 0x0d, 0x0e, 0x60, 0x67 */
        24,   /* case 0x0e */
        32,   /* cases 0x2c, 0x68 */
        64,   /* generic L1 */
        128,  /* cases 0x39, 0x3b, 0x41, 0x79 */
        192,  /* case 0x3a */
        256,  /* cases 0x21, 0x3c, 0x42, 0x7a, 0x82 */
        384,  /* case 0x3d */
        512,  /* cases 0x3e, 0x43, 0x7b, 0x7f, 0x80, 0x83, 0x86 */
        1024, /* cases 0x24, 0x44, 0x78, 0x7c, 0x84, 0x87 */
        2048, /* cases 0x45, 0x7d, 0x85 */
        3072, /* case 0x48 */
        4096, /* case 0x49 */
        6144  /* case 0x4e */
    };
    
    for (size_t i = 0; i < sizeof(test_sizes)/sizeof(test_sizes[0]); i++) {
        cache_thrash(test_sizes[i], 1000 / (1 + (test_sizes[i] / 256)));
    }
    
    printf("Cache test completed.\n");
    return 0;
}
