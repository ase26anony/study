/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int y = x;
    return y ^ 0x55;
}

/* Complex structure with mixed types */
struct nested {
    int a;
    long b;
    volatile int c;
    float f;
    double d;
};

/* Multi-dimensional array */
static volatile int multi_array[8][8][8];

/* Force register pressure with many live variables */
__attribute__((noinline,noipa))
int test_reloads(int seed) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("r12") = seed;
    register int r1 asm ("r13") = seed + 1;
    int v1 = barrier(seed);
    int v2 = barrier(seed * 2);
    int v3 = barrier(seed * 3);
    int v4 = barrier(seed * 4);
    int v5 = barrier(seed * 5);
    int v6 = barrier(seed * 6);
    int v7 = barrier(seed * 7);
    int v8 = barrier(seed * 8);
    int v9 = barrier(seed * 9);
    int v10 = barrier(seed * 10);
    long l1 = (long)v1 * v2;
    long l2 = (long)v3 * v4;
    long l3 = (long)v5 * v6;
    long l4 = (long)v7 * v8;
    long l5 = (long)v9 * v10;
    float f1 = (float)v1 / 3.0f;
    float f2 = (float)v2 / 7.0f;
    double d1 = (double)v3 / 11.0;
    double d2 = (double)v4 / 13.0;
    
    /* Complex addressing mode requiring potential secondary reload */
    volatile int idx1 = v1 & 7;
    volatile int idx2 = v2 & 7;
    volatile int idx3 = v3 & 7;
    
    /* Access with complex addressing - may need SIB on x86 */
    int val1 = multi_array[idx1][idx2][idx3];
    int val2 = multi_array[idx3][idx1][idx2];
    
    /* Force register clobbering with inline asm */
    asm volatile (
        "# Complex asm block\n"
        "mov %[r0], %[tmp]\n\t"
        "add %[r1], %[tmp]\n\t"
        : [tmp] "=r" (v1)
        : [r0] "r" (r0), [r1] "r" (r1)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* More arithmetic to create dependency chain */
    v2 = v1 + v2 * 3;
    v3 = v2 + v3 * 5;
    v4 = v3 + v4 * 7;
    v5 = v4 + v5 * 11;
    v6 = v5 + v6 * 13;
    v7 = v6 + v7 * 17;
    v8 = v7 + v8 * 19;
    v9 = v8 + v9 * 23;
    v10 = v9 + v10 * 29;
    
    /* Mixed-type operations forcing moves between register classes */
    union {
        int i;
        float f;
    } pun;
    pun.i = v1;
    f1 = pun.f + f1;
    pun.f = f1;
    v1 = pun.i ^ v1;
    
    /* Atomic operations with complex addresses */
    struct nested ns[4];
    volatile int* volatile_ptr = &ns[idx1].c;
    
    __atomic_store_n(volatile_ptr, v2, __ATOMIC_RELAXED);
    int loaded = __atomic_load_n(volatile_ptr, __ATOMIC_RELAXED);
    
    /* Another inline asm with memory constraint and complex address */
    int* complex_addr = &ns[idx2].a + idx3 * 2;
    asm volatile (
        "# Memory constraint with complex address\n"
        "add %[in], %[out]\n\t"
        : [out] "=m" (*complex_addr)
        : [in] "r" (loaded), "[out]" (*complex_addr)
        : "cc"
    );
    
    /* More register pressure */
    l1 = l1 + (long)v1 * v2;
    l2 = l2 + (long)v3 * v4;
    l3 = l3 + (long)v5 * v6;
    l4 = l4 + (long)v7 * v8;
    l5 = l5 + (long)v9 * v10;
    
    d1 = d1 + (double)l1 / 100.0;
    d2 = d2 + (double)l2 / 200.0;
    
    /* Use all variables in final computation */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= (int)l1 ^ (int)l2 ^ (int)l3 ^ (int)l4 ^ (int)l5;
    result ^= (int)f1 ^ (int)f2;
    result ^= (int)d1 ^ (int)d2;
    result ^= val1 ^ val2 ^ loaded ^ *complex_addr;
    
    return barrier(result);
}

/* Vector operations for targets with vector registers */
#ifdef __SSE2__
#include <xmmintrin.h>
__attribute__((noinline))
int vector_reloads(int seed) {
    /* Force moves between vector and scalar registers */
    __m128i v1 = _mm_set_epi32(seed, seed+1, seed+2, seed+3);
    __m128i v2 = _mm_set_epi32(seed+4, seed+5, seed+6, seed+7);
    
    /* Vector operations */
    __m128i v3 = _mm_add_epi32(v1, v2);
    __m128i v4 = _mm_mullo_epi16(v1, v2);
    
    /* Extract to scalar - may require reloads */
    int arr[4];
    _mm_storeu_si128((__m128i*)arr, v3);
    
    /* Mix with scalar variables */
    int s1 = barrier(arr[0]);
    int s2 = barrier(arr[1]);
    int s3 = barrier(arr[2]);
    int s4 = barrier(arr[3]);
    
    /* Complex addressing with vector results */
    volatile int idx = s1 & 3;
    multi_array[idx][s2 & 7][s3 & 7] = s4;
    
    return s1 + s2 + s3 + s4;
}
#endif

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_array[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    int result = test_reloads(seed);
    
    #ifdef __SSE2__
    result ^= vector_reloads(seed);
    #endif
    
    /* Force another round with different seed */
    result ^= test_reloads(seed * 3 + 1);
    
    printf("Result: %d\n", result);
    return result & 255;
}
