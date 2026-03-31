/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a + b;
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

__attribute__((noinline)) int helper2(int a, int b) {
    volatile int result = a * b;
    asm volatile("" : "+r"(result) : : "memory", "eax");
    return result;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float result = a / b;
    asm volatile("" : "+x"(result) : : "memory", "xmm0");
    return result;
}

/* Function with mixed operations to create complex scheduling regions */
__attribute__((noinline, optimize("O3"))) 
int complex_scheduling_region(int seed, int iterations) {
    volatile int* volatile_ptr = &seed;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    float fa = seed * 1.0f, fb = seed * 2.0f, fc = seed * 3.0f;
    __m128i vec1, vec2, vec3;
    int result = 0;
    
    /* Force scheduling state with volatile accesses */
    *volatile_ptr = *volatile_ptr + 1;
    asm volatile("mfence" ::: "memory");
    
    /* Create instruction chains with dependencies */
    for (int i = 0; i < iterations; i++) {
        /* Integer operation chain */
        a = b + c;
        d = a * (i + 1);
        a = d >> 2;
        b = a ^ c;
        c = b | d;
        
        /* Mixed with volatile */
        *volatile_ptr = *volatile_ptr + a;
        
        /* SIMD operations for target-specific scheduling */
        vec1 = _mm_set_epi32(a, b, c, d);
        vec2 = _mm_set_epi32(d, c, b, a);
        vec3 = _mm_add_epi32(vec1, vec2);
        
        int arr[4];
        _mm_storeu_si128((__m128i*)arr, vec3);
        
        /* Floating point chain */
        fa = fb * fc;
        fb = fc + fa;
        fc = fa / (fb + 1.0f);
        
        /* Function calls create scheduling barriers */
        if (i % 3 == 0) {
            a = helper1(a, b);
            asm volatile("" ::: "memory");
        } else if (i % 3 == 1) {
            b = helper2(b, c);
            asm volatile("" ::: "memory", "ebx");
        } else {
            fa = helper3(fa, fb);
            asm volatile("" ::: "memory", "xmm1");
        }
        
        /* Switch with multiple cases for different scheduling paths */
        switch (i % 5) {
            case 0:
                result += a + __builtin_popcount(b);
                asm volatile("nop" ::: "memory");
                break;
            case 1:
                result += b + __builtin_ctz(c);
                asm volatile("nop" ::: "memory", "eax");
                break;
            case 2:
                result += c + (int)fa;
                /* Inline assembly with explicit clobbers */
                asm volatile("addl %%eax, %%ebx" : "+b"(result) : "a"(d) : "cc");
                break;
            case 3:
                result += d + arr[i % 4];
                /* Memory barrier */
                asm volatile("lock; addl $0, 0(%%esp)" ::: "memory", "cc");
                break;
            case 4:
                result += arr[0] + arr[1] + arr[2] + arr[3];
                /* Complex asm with multiple constraints */
                asm volatile("pmulld %1, %0" : "+x"(vec3) : "xm"(vec1) : "cc");
                _mm_storeu_si128((__m128i*)arr, vec3);
                break;
        }
        
        /* Nested conditional for additional complexity */
        if (result > 1000) {
            for (int j = 0; j < 3; j++) {
                result -= j * __builtin_expect(arr[j % 4], 0);
                asm volatile("" ::: "memory");
            }
        } else {
            for (int j = 0; j < 2; j++) {
                result += j * __builtin_expect(arr[(j + 1) % 4], 1);
                asm volatile("" ::: "memory", "ecx");
            }
        }
    }
    
    return result;
}

/* Another region with different patterns */
__attribute__((noinline, optimize("O2")))
int second_scheduling_region(int base) {
    volatile int v1 = base, v2 = base * 2;
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    
    /* Unrolled loop with independent operations */
    #pragma GCC unroll 4
    for (int i = 0; i < 16; i++) {
        r1 += v1 * i;
        r2 += v2 / (i + 1);
        r3 ^= r1 | r2;
        r4 = r3 << (i % 8);
        
        /* Memory operations with different addressing modes */
        volatile int* p = &v1;
        *p = *p + r4;
        p = &v2;
        *p = *p - r3;
        
        /* SIMD again */
        __m128i v = _mm_set1_epi32(i);
        __m128i w = _mm_set1_epi32(r1);
        __m128i x = _mm_mullo_epi32(v, w);
        
        int tmp[4];
        _mm_storeu_si128((__m128i*)tmp, x);
        
        r1 += tmp[0];
        r2 += tmp[1];
        r3 += tmp[2];
        r4 += tmp[3];
    }
    
    /* Computed goto for control flow complexity */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
    goto *labels[base % 4];
    
L0:
    r1 = helper1(r1, r2);
    asm volatile("" ::: "memory");
    goto LEND;
L1:
    r2 = helper2(r2, r3);
    asm volatile("" ::: "memory", "edx");
    goto LEND;
L2:
    r3 = helper1(r3, r4);
    asm volatile("" ::: "memory", "eax", "ebx");
    goto LEND;
L3:
    r4 = helper2(r4, r1);
    asm volatile("" ::: "memory", "ecx", "edx");
    goto LEND;
LEND:
    
    return r1 + r2 + r3 + r4;
}

/* Third region with pointer arithmetic and mixed types */
__attribute__((always_inline)) 
static inline int inline_helper(int* arr, float* farr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + i;
        farr[i] = farr[i] * 1.5f - i;
        sum += arr[i] + (int)farr[i];
        
        /* Inline asm with register constraints */
        asm volatile("imull %%ecx, %%eax" 
                     : "+a"(arr[i]) 
                     : "c"(i + 1) 
                     : "cc");
    }
    return sum;
}

int third_scheduling_region(int seed) {
    int arr[16];
    float farr[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
        farr[i] = seed * 0.5f + i;
    }
    
    int result = 0;
    
    /* Multiple passes with different optimizations */
    #pragma GCC optimize("O3")
    {
        result += inline_helper(arr, farr, 8);
    }
    
    #pragma GCC optimize("O2")
    {
        result += inline_helper(arr + 8, farr + 8, 8);
    }
    
    /* Complex expression with many temporaries */
    result = result * 3 - arr[0] + arr[1] * 2 - arr[2] / 3 + 
             (int)(farr[0] * 2.0f) - (int)(farr[1] / 1.5f) +
             __builtin_popcount(arr[3]) + __builtin_ctz(arr[4]);
    
    return result;
}

int main() {
    int final_result = 0;
    
    /* Execute multiple scheduling regions with different characteristics */
    for (int region = 0; region < 5; region++) {
        switch (region) {
            case 0:
                final_result ^= complex_scheduling_region(region * 100, 8);
                break;
            case 1:
                final_result += second_scheduling_region(region * 50);
                break;
            case 2:
                final_result |= third_scheduling_region(region * 25);
                break;
            case 3:
                /* Mix regions */
                final_result += complex_scheduling_region(final_result, 4);
                final_result ^= second_scheduling_region(final_result);
                break;
            case 4:
                /* Deep nesting */
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        if (__builtin_expect((i + j) % 2 == 0, 1)) {
                            final_result += complex_scheduling_region(i * j, 2);
                        } else {
                            final_result -= second_scheduling_region(i + j);
                        }
                        asm volatile("" ::: "memory");
                    }
                }
                break;
        }
        
        /* Memory barrier between regions */
        asm volatile("mfence" ::: "memory");
    }
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    
    /* Use result to affect control flow */
    if (__builtin_expect(output > 0, 1)) {
        return output % 255;
    } else {
        return (-output) % 255;
    }
}
