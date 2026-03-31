/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) static int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + barrier;
}

__attribute__((noinline)) static float helper2(float a, float b) {
    volatile float barrier = 1.0f;
    asm volatile("" : "+x"(a), "+x"(b) : : "memory");
    return a / b + barrier;
}

__attribute__((noinline)) static __m128i helper3(__m128i a, __m128i b) {
    volatile __m128i barrier = _mm_set1_epi32(0);
    asm volatile("" : "+x"(a), "+x"(b) : : "memory");
    return _mm_add_epi32(a, b);
}

/* Function with complex scheduling requirements */
__attribute__((noinline)) static int compute_region(int seed, int iterations) {
    int i, j, k;
    int result = seed;
    volatile int* volatile_ptr = &result;
    float fresult = seed * 1.5f;
    __m128i vec_result = _mm_set1_epi32(seed);
    
    /* Region 1: Mixed operations with volatile and inline asm */
    for (i = 0; i < iterations; i++) {
        /* Create instruction chains with dependencies */
        int a = result + i;
        int b = a * 2;
        int c = b >> 3;
        int d = c ^ 0x55AA55AA;
        
        /* Volatile access creates scheduling barrier */
        *volatile_ptr = *volatile_ptr + d;
        
        /* Inline assembly with clobbers */
        asm volatile (
            "addl %1, %0\n\t"
            "rorl $7, %0"
            : "+r"(result)
            : "r"(d)
            : "cc", "memory"
        );
        
        /* Floating point operations */
        fresult = helper2(fresult, 1.1f + i);
        
        /* SIMD operations */
        __m128i vec_tmp = _mm_set1_epi32(i);
        vec_result = helper3(vec_result, vec_tmp);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Region 2: Nested loops with data-dependent exits */
    for (i = 0; i < 10; i++) {
        int limit = (result & 0xF) + 5;
        for (j = 0; j < limit; j++) {
            /* Independent instructions for ready list */
            int x1 = i * j;
            int x2 = i + j;
            int x3 = i ^ j;
            int x4 = i - j;
            int x5 = i & j;
            
            /* Use all results to prevent elimination */
            result += x1 + x2 + x3 + x4 + x5;
            
            /* Conditional break with data dependency */
            if ((x1 + x2) > 100) {
                /* More operations in break path */
                result = helper1(result, 3);
                break;
            }
            
            /* Function call in loop */
            result = helper1(result, 2);
        }
        
        /* Switch statement with multiple cases */
        switch (i & 3) {
            case 0:
                result = __builtin_popcount(result);
                result = result * 3 + 1;
                break;
            case 1:
                result = __builtin_ctz(result | 1);
                result = result << 2;
                break;
            case 2:
                result = helper1(result, result);
                result = result ^ 0x12345678;
                break;
            case 3:
                /* Fall through with goto to create complex CFG */
                goto common_label;
                break;
        }
        
        if (i == 8) {
            common_label:
            result = result + 0x1000;
            /* Vector operation in uncommon path */
            vec_result = _mm_slli_epi32(vec_result, 2);
        }
    }
    
    /* Region 3: Many independent instructions */
    {
        int t1 = result + 1;
        int t2 = result * 2;
        int t3 = result >> 1;
        int t4 = result ^ 0xFF;
        int t5 = result & 0xAA;
        int t6 = result | 0x55;
        int t7 = __builtin_bswap32(result);
        int t8 = helper1(result, 5);
        float t9 = helper2(fresult, 2.0f);
        __m128i t10 = _mm_srli_epi32(vec_result, 1);
        
        /* Use all temporaries */
        result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + (int)t9;
        int vec_sum = _mm_extract_epi32(t10, 0);
        result += vec_sum;
    }
    
    return result;
}

/* Another scheduling region with different characteristics */
__attribute__((noinline)) static int compute_region2(int base) {
    int arr[64];
    volatile int* varr = arr;
    int sum = base;
    
    /* Initialize array */
    for (int i = 0; i < 64; i++) {
        arr[i] = i * base;
    }
    
    /* Complex pointer arithmetic and memory access pattern */
    for (int i = 0; i < 32; i++) {
        /* Multiple memory accesses with different latencies */
        int idx1 = (i * 3) & 63;
        int idx2 = (i * 5) & 63;
        int idx3 = (i * 7) & 63;
        
        int val1 = varr[idx1];  /* Volatile read */
        int val2 = arr[idx2];   /* Normal read */
        varr[idx3] = val1 + val2; /* Volatile write */
        
        /* Arithmetic chain */
        val1 = val1 * 2 + 1;
        val2 = val2 / 2 - 1;
        int val3 = val1 ^ val2;
        val3 = val3 << (i & 3);
        
        /* Inline asm with register constraints */
        asm volatile (
            "imull %%ecx, %%eax\n\t"
            "addl %%edx, %%eax"
            : "+a"(val3)
            : "c"(val1), "d"(val2)
            : "cc"
        );
        
        sum += val3;
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return sum;
}

/* Main function with multiple scheduling regions */
int main() {
    int final_result = 0;
    
    /* Profile-guided optimization hints */
    if (__builtin_expect(final_result == 0, 1)) {
        /* Region 1 */
        final_result = compute_region(42, 25);
        
        /* Region 2 - different optimization characteristics */
        #pragma GCC optimize("O2")
        final_result = compute_region2(final_result);
        
        /* Region 3 - reset optimization */
        #pragma GCC optimize("O3")
        final_result = compute_region(final_result, 15);
        
        /* Region 4 - with vector operations */
        {
            __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
            __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
            __m128i v3 = _mm_add_epi32(v1, v2);
            __m128i v4 = _mm_mullo_epi32(v3, v1);
            
            /* Extract and use vector results */
            int vsum = _mm_extract_epi32(v4, 0) +
                      _mm_extract_epi32(v4, 1) +
                      _mm_extract_epi32(v4, 2) +
                      _mm_extract_epi32(v4, 3);
            
            final_result += vsum;
            
            /* More vector operations */
            v1 = _mm_slli_epi32(v4, 2);
            v2 = _mm_srli_epi32(v4, 1);
            v3 = _mm_and_si128(v1, v2);
            v4 = _mm_or_si128(v3, _mm_set1_epi32(0xFF));
            
            vsum = _mm_extract_epi32(v4, 0) ^
                  _mm_extract_epi32(v4, 1) ^
                  _mm_extract_epi32(v4, 2) ^
                  _mm_extract_epi32(v4, 3);
            
            final_result ^= vsum;
        }
        
        /* Region 5 - final mixed operations */
        volatile int sync = 0;
        for (int i = 0; i < 100; i++) {
            int tmp = final_result;
            
            /* Long dependency chain */
            for (int j = 0; j < 5; j++) {
                tmp = helper1(tmp, j + 1);
                tmp = tmp + (tmp >> 3);
                tmp = tmp ^ (tmp << 5);
                tmp = tmp * 1103515245 + 12345;
            }
            
            final_result = tmp;
            sync = final_result; /* Volatile write */
            
            /* Conditional with unlikely path */
            if (__builtin_expect((i & 0x1F) == 0, 0)) {
                /* Unlikely path with different operations */
                float ftmp = helper2((float)final_result, 1.414f);
                final_result += (int)ftmp;
                asm volatile("mfence" : : : "memory");
            }
        }
    }
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    
    return output & 0x7FFFFFFF; /* Ensure positive return */
}
