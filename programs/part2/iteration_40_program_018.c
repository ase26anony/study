/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
#define NOINLINE __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Volatile variables to create scheduling barriers */
static volatile int g_volatile_counter = 0;
static volatile int* g_volatile_ptr = 0;

/* Helper functions that won't be inlined */
NOINLINE int helper1(int a, int b) {
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + (a ^ b);
}

NOINLINE float helper2(float x, float y) {
    asm volatile ("" : "+x"(x), "+x"(y) : : "memory");
    return x * y - x / y;
}

NOINLINE void helper3(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + i;
    }
    asm volatile ("" : : "r"(arr), "r"(n) : "memory");
}

/* Function with mixed operations to create complex scheduling regions */
ALWAYS_INLINE int complex_region_1(int seed) {
    int a = seed;
    int b = a + 12345;
    volatile int* p = &g_volatile_counter;
    
    /* Chain of dependent integer operations */
    a = b * 3;
    b = a >> 4;
    a = b ^ 0xABCD;
    b = a + *p;  /* Volatile access creates barrier */
    
    /* Inline assembly with clobbers */
    asm volatile (
        "addl $5, %0\n\t"
        "rorl $3, %0\n\t"
        : "+r"(b)
        : 
        : "cc", "memory"
    );
    
    /* SIMD operations (x86 specific) */
    __m128i v1 = _mm_set_epi32(a, b, seed, 0);
    __m128i v2 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v3 = _mm_add_epi32(v1, v2);
    
    int result;
    _mm_storeu_si128((__m128i*)&result, v3);
    
    return result + helper1(a, b);
}

ALWAYS_INLINE float complex_region_2(float seed) {
    float x = seed;
    float y = x * 2.0f;
    volatile float* fp = (volatile float*)&g_volatile_counter;
    
    /* Mixed floating point operations */
    x = y / 1.5f;
    y = x * x - *fp;
    x = y + helper2(x, y);
    
    /* More inline assembly */
    asm volatile (
        "fadds %1, %0\n\t"
        "fmuls %0, %0\n\t"
        : "+t"(x)
        : "m"(*fp)
        : "memory"
    );
    
    return x;
}

/* Function with switch statement creating multiple basic blocks */
NOINLINE int switch_region(int val) {
    int result = 0;
    
    switch (val & 0x7) {
        case 0: {
            /* Vector operations */
            __m128i v = _mm_set1_epi32(val);
            v = _mm_slli_epi32(v, 2);
            v = _mm_add_epi32(v, _mm_set1_epi32(1));
            result = _mm_extract_epi32(v, 0);
            break;
        }
        case 1: {
            /* Integer chain */
            result = val;
            for (int i = 0; i < 8; i++) {
                result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
                asm volatile ("" : "+r"(result) : : "memory");
            }
            break;
        }
        case 2: {
            /* Memory operations */
            int arr[16];
            for (int i = 0; i < 16; i++) {
                arr[i] = val + i;
                g_volatile_ptr = &arr[i];
            }
            helper3(arr, 16);
            result = arr[15];
            break;
        }
        case 3: {
            /* Builtin functions */
            result = __builtin_popcount(val) + 
                    __builtin_ctz(val | 1) + 
                    __builtin_clz(val | 1);
            break;
        }
        case 4: {
            /* More assembly with specific registers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull $137, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r"(result)
                : "r"(val)
                : "%eax", "cc", "memory"
            );
            break;
        }
        default: {
            result = val * 3 - 7;
            /* Force scheduling barrier */
            asm volatile ("" : : : "memory");
            result ^= g_volatile_counter;
        }
    }
    
    return result;
}

/* Main function with nested control flow */
int main() {
    int checksum = 0;
    int array[256];
    float farray[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array[i] = i;
        farray[i] = i * 0.5f;
    }
    
    /* Region 1: Nested loops with data-dependent exits */
    for (int i = 0; i < 100; i++) {
        int j = i;
        while (j > 0) {
            checksum += complex_region_1(j);
            j = (j * 31) & 0xFF;
            if (checksum & 0x100) break;
        }
        
        /* Volatile access in loop */
        g_volatile_counter = i;
    }
    
    /* Region 2: Mixed operations in tight loop */
    for (int i = 0; i < 50; i++) {
        /* Independent instructions that can execute in parallel */
        int a = array[i] + 1;
        int b = array[i + 50] * 2;
        int c = array[i + 100] & 0xFF;
        float d = farray[i] * 3.14f;
        
        /* Force them to be used together */
        checksum += a + b + c + (int)d;
        
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* More operations */
        checksum ^= switch_region(checksum);
    }
    
    /* Region 3: Complex control flow with goto */
    int x = 0;
    int limit = 1000;
    
compute_region:
    {
        /* Many independent instructions */
        int t1 = x * 3;
        int t2 = x + 5;
        int t3 = x ^ 0xAA;
        int t4 = x << 2;
        int t5 = x >> 1;
        float f1 = x * 0.25f;
        float f2 = complex_region_2(f1);
        
        /* Use all results */
        checksum += t1 + t2 + t3 + t4 + t5 + (int)f2;
        
        /* Memory operations with different access patterns */
        array[x & 0xFF] = checksum;
        farray[x & 0xFF] = f2;
        
        x++;
        if (x < limit) {
            /* Vary the control flow */
            if (x & 1) goto compute_region;
            else if (x & 2) {
                checksum += switch_region(x);
                goto compute_region;
            }
        }
    }
    
    /* Region 4: SIMD intensive operations */
    for (int i = 0; i < 64; i += 4) {
        __m128i v1 = _mm_loadu_si128((__m128i*)&array[i]);
        __m128i v2 = _mm_loadu_si128((__m128i*)&array[i + 64]);
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi32(v3, _mm_set1_epi32(3));
        _mm_storeu_si128((__m128i*)&array[i], v4);
        
        /* Interleave with scalar operations */
        checksum += _mm_extract_epi32(v4, 0);
        asm volatile ("" : : : "memory");
    }
    
    /* Region 5: Function calls with varying arguments */
    for (int i = 0; i < 32; i++) {
        checksum += helper1(checksum, i);
        checksum += (int)helper2(checksum * 0.01f, i * 0.1f);
        
        /* Every 8 iterations, do something different */
        if ((i & 7) == 0) {
            helper3(array, 256);
            checksum ^= g_volatile_counter;
        }
    }
    
    /* Final mixing */
    checksum = (checksum * 1103515245 + 12345) & 0x7FFFFFFF;
    
    /* Use profile-guided optimization hint */
    if (__builtin_expect(checksum > 1000000, 0)) {
        checksum = switch_region(checksum);
    }
    
    return checksum & 0xFF;
}
