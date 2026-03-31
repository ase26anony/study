/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o coverage coverage.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + barrier;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* vptr = &arr[idx];
    int val = *vptr;
    asm volatile("" : : "r"(val) : "memory");
    return val;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float f = 0.0f;
    asm volatile("" : "+x"(a), "+x"(b) : : "memory");
    return a * b + f;
}

/* Force different scheduling regions */
#define SCHEDULE_BARRIER() asm volatile("" : : : "memory")

/* Complex scheduling region 1: Mixed operations with dependencies */
int region1(int seed) {
    int a = seed + 1;
    volatile int* vp = &a;
    int b = *vp * 2;
    
    /* Chain of dependent integer ops */
    int c = b << 3;
    int d = c ^ 0xABCD;
    int e = d + a;
    int f = e * 7;
    int g = f >> 2;
    int h = g & 0xFF;
    
    /* Inline assembly with clobbers */
    asm volatile (
        "addl $100, %0\n\t"
        "rorl $3, %0"
        : "+r"(h) : : "cc", "eax"
    );
    
    /* Memory barrier */
    SCHEDULE_BARRIER();
    
    /* SIMD operations */
    __m128i vec1 = _mm_set_epi32(h, g, f, e);
    __m128i vec2 = _mm_set_epi32(d, c, b, a);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    
    int result;
    _mm_storeu_si128((__m128i*)&result, vec3);
    
    /* Function call creates scheduling boundary */
    result = helper1(result, seed);
    
    return result;
}

/* Complex scheduling region 2: Nested control flow */
int region2(int base) {
    int sum = 0;
    volatile int counter = base;
    
    /* Nested loops with data-dependent exit */
    for (int i = 0; i < 8; i++) {
        int inner = counter;
        for (int j = 0; j < 4; j++) {
            /* Mixed operations preventing combining */
            inner = inner * 3 + j;
            inner = inner ^ (inner >> 1);
            inner = helper2(&inner, 0);
            
            if (inner & 1) {
                /* Early exit creates complex CFG */
                sum += inner;
                if (inner > 1000) break;
            }
        }
        
        /* Architecture-specific builtins */
        sum += __builtin_popcount(inner);
        sum += __builtin_ctz(i | 1);
        
        /* More inline assembly with constraints */
        asm volatile (
            "imull %%ecx, %%eax\n\t"
            "addl %%edx, %%eax"
            : "+a"(sum)
            : "c"(inner), "d"(i)
            : "cc"
        );
        
        counter = sum;
    }
    
    return sum;
}

/* Complex scheduling region 3: Switch with multiple cases */
int region3(int selector) {
    int result = 0;
    
    switch (selector & 7) {
        case 0: {
            /* Vector operations */
            __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 v2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
            __m128 v3 = _mm_mul_ps(v1, v2);
            float farr[4];
            _mm_storeu_ps(farr, v3);
            result = (int)farr[0];
            break;
        }
        case 1: {
            /* Integer chain */
            int x = selector;
            x = x * 3 + 1;
            x = x ^ (x << 4);
            x = x >> 2;
            result = helper1(x, selector);
            break;
        }
        case 2: {
            /* Mixed float/int */
            float f = (float)selector;
            f = helper3(f, f * 2.0f);
            result = (int)f;
            break;
        }
        case 3: {
            /* Memory intensive */
            volatile int arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = selector + i;
                result += arr[i];
            }
            break;
        }
        case 4: {
            /* More SIMD */
            __m128i v = _mm_set_epi32(selector, selector+1, selector+2, selector+3);
            v = _mm_slli_epi32(v, 2);
            int tmp[4];
            _mm_storeu_si128((__m128i*)tmp, v);
            result = tmp[0] + tmp[1] + tmp[2] + tmp[3];
            break;
        }
        default: {
            /* Complex arithmetic */
            result = selector;
            for (int i = 0; i < 5; i++) {
                result = result * 1103515245 + 12345;
                result = (result >> 16) & 0x7FFF;
            }
            break;
        }
    }
    
    /* Common tail with more operations */
    result = result * 3 - 1;
    result = result ^ (result >> 8);
    
    return result;
}

/* Complex scheduling region 4: Parallel independent operations */
int region4(int a, int b, int c, int d) {
    /* Many independent instructions */
    int r1 = a + b;
    int r2 = c * d;
    int r3 = a ^ b ^ c;
    int r4 = d << 3;
    int r5 = helper1(a, c);
    int r6 = helper2(&b, 0);
    
    SCHEDULE_BARRIER();
    
    /* More parallelism */
    float f1 = (float)r1 * 1.5f;
    float f2 = (float)r2 * 2.5f;
    float f3 = helper3(f1, f2);
    
    int r7 = (int)f3;
    int r8 = __builtin_popcount(r3);
    int r9 = r4 & 0xFF;
    
    /* Assembly with multiple outputs */
    asm volatile (
        "leal (%1, %2, 4), %0\n\t"
        "addl %3, %0"
        : "=r"(r9)
        : "r"(r7), "r"(r8), "r"(r9)
        : "cc"
    );
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
}

/* Main function with multiple scheduling regions */
int main() {
    volatile int seed = 42;
    int result = seed;
    
    /* Force multiple scheduling contexts */
    #pragma GCC optimize("O2")
    result = region1(result);
    
    #pragma GCC optimize("O3")
    result = region2(result);
    
    #pragma GCC optimize("Os")
    result = region3(result);
    
    #pragma GCC optimize("O3")
    result = region4(result, result+1, result+2, result+3);
    
    /* Final complex region */
    int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = result + i;
    }
    
    /* Mix of operations in loop */
    for (int i = 0; i < 16; i++) {
        if (i & 1) {
            arr[i] = helper1(arr[i], i);
        } else {
            arr[i] = helper2(arr, i);
        }
        
        /* SIMD in loop */
        if (i % 4 == 0) {
            __m128i v = _mm_loadu_si128((__m128i*)&arr[i]);
            v = _mm_add_epi32(v, _mm_set1_epi32(1));
            _mm_storeu_si128((__m128i*)&arr[i], v);
        }
        
        /* Memory barrier every few iterations */
        if (i % 3 == 0) {
            SCHEDULE_BARRIER();
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum ^= arr[i];
        checksum = checksum * 31 + i;
    }
    
    /* Use builtin_expect for PGO hints */
    if (__builtin_expect(checksum > 1000000, 0)) {
        checksum = checksum >> 4;
    }
    
    return checksum;
}
