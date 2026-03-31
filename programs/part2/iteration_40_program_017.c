/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) static int helper1(int a, int b) {
    volatile int barrier = a;
    asm volatile ("" : "+r" (barrier) : : "memory");
    return barrier + b * 3;
}

__attribute__((noinline)) static float helper2(float a, float b) {
    volatile float barrier = a;
    asm volatile ("" : "+x" (barrier) : : "memory");
    return barrier * b - 1.0f;
}

__attribute__((noinline)) static void helper3(volatile int* p, int n) {
    for (int i = 0; i < n; i++) {
        *p = *p + i;
        asm volatile ("nop" : : : "memory", "eax", "ebx");
    }
}

/* Complex function with multiple scheduling regions */
static int complex_scheduling_region(int seed) {
    volatile int v1 = seed;
    volatile float v2 = seed * 1.5f;
    volatile double v3 = seed * 2.5;
    int result = 0;
    
    /* Region 1: Mixed integer operations with dependencies */
    int a = v1 + 1;
    int b = a * v1;
    int c = b >> 3;
    int d = c ^ v1;
    int e = d & 0xFF;
    int f = e * 7;
    int g = f - a;
    int h = g / 2;
    result += h;
    
    /* Inline assembly with clobbers to force scheduling constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (result)
        : "r" (v1)
        : "eax", "memory"
    );
    
    /* Region 2: SIMD operations */
    __m128i vec1 = _mm_set_epi32(v1, v1 + 1, v1 + 2, v1 + 3);
    __m128i vec2 = _mm_set_epi32(v1 * 2, v1 * 3, v1 * 4, v1 * 5);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    __m128i vec4 = _mm_mullo_epi32(vec3, vec1);
    
    alignas(16) int vec_result[4];
    _mm_store_si128((__m128i*)vec_result, vec4);
    result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Region 3: Floating point with memory barriers */
    float f1 = v2;
    float f2 = f1 * 3.14f;
    float f3 = f2 / 1.618f;
    float f4 = f3 + f1;
    float f5 = f4 - f2;
    asm volatile ("" : : : "memory");
    result += (int)(f5 * 100);
    
    /* Call to noinline helper */
    result += helper1(result, v1);
    
    return result;
}

/* Another complex region with switch statement */
static int switch_based_scheduling(int mode) {
    int result = 0;
    
    switch (mode & 7) {
        case 0: {
            /* Long dependency chain */
            int a = mode;
            int b = a * 2;
            int c = b + a;
            int d = c ^ 0x55AA55AA;
            int e = d >> 4;
            int f = e * 3;
            int g = f & 0xFFF;
            int h = g - a;
            int i = h / 2;
            int j = i | 0x1000;
            result = j;
            asm volatile ("nop; nop; nop" : : : "memory", "eax", "ebx", "ecx");
            break;
        }
        case 1: {
            /* Independent operations */
            int x1 = mode + 1;
            int x2 = mode * 2;
            int x3 = mode ^ 0x12345678;
            int x4 = mode >> 3;
            int x5 = mode & 0xFF;
            asm volatile ("" : : : "memory");
            result = x1 + x2 + x3 + x4 + x5;
            break;
        }
        case 2: {
            /* Mixed float/int */
            float f1 = mode * 1.1f;
            float f2 = f1 * 2.2f;
            int i1 = (int)f1;
            int i2 = (int)f2;
            result = i1 * i2 + mode;
            break;
        }
        case 3: {
            /* Vector operations */
            __m128 v1 = _mm_set_ps(mode, mode+1, mode+2, mode+3);
            __m128 v2 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 v3 = _mm_mul_ps(v1, v2);
            alignas(16) float farr[4];
            _mm_store_ps(farr, v3);
            result = (int)(farr[0] + farr[1] + farr[2] + farr[3]);
            break;
        }
        default: {
            /* Complex chain with function call */
            volatile int tmp = mode;
            for (int i = 0; i < 3; i++) {
                tmp = tmp * 2 + i;
                asm volatile ("" : "+r" (tmp) : : "memory");
            }
            result = helper1(tmp, mode);
            helper3(&result, 2);
        }
    }
    
    return result;
}

/* Function with nested loops creating instruction queues */
static int nested_loop_scheduling(int limit) {
    int sum = 0;
    volatile int counter = 0;
    
    for (int i = 0; i < limit; i++) {
        /* Outer loop operations */
        int a = i * 2;
        int b = a ^ 0xABCD;
        int c = b >> 1;
        
        for (int j = 0; j < (i & 3) + 1; j++) {
            /* Inner loop with data-dependent exit */
            int x = j + c;
            int y = x * x;
            int z = y % 257;
            
            /* Memory barrier to split scheduling regions */
            asm volatile ("" : : : "memory");
            
            sum += z;
            
            /* Volatile access */
            counter = counter + 1;
            
            /* Early exit based on computation */
            if ((z & 1) && (j > 0)) {
                asm volatile ("nop" : : : "memory", "eax");
                break;
            }
        }
        
        /* Mix in some floating point */
        float f1 = i * 0.5f;
        float f2 = helper2(f1, sum * 0.01f);
        sum += (int)f2;
    }
    
    return sum;
}

/* Main function with multiple scheduling regions */
int main() {
    int final_result = 0;
    volatile int seed = 42;
    
    /* Array of different types to force various operations */
    int int_array[256];
    float float_array[256];
    volatile int volatile_array[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
        volatile_array[i] = i * 2;
    }
    
    /* Region 1: Complex scheduling with SIMD */
    for (int i = 0; i < 100; i++) {
        final_result += complex_scheduling_region(seed + i);
        
        /* Interleave with SIMD operations */
        __m128i v1 = _mm_loadu_si128((__m128i*)&int_array[i % 240]);
        __m128i v2 = _mm_set1_epi32(i);
        __m128i v3 = _mm_add_epi32(v1, v2);
        _mm_storeu_si128((__m128i*)&int_array[i % 240], v3);
        
        /* Architecture-specific builtins */
        final_result += __builtin_popcount(final_result);
        final_result += __builtin_ctz(final_result | 1);
    }
    
    /* Region 2: Switch-based scheduling */
    for (int i = 0; i < 50; i++) {
        final_result ^= switch_based_scheduling(final_result + i);
        
        /* Memory operations with different latencies */
        volatile_array[i % 128] = final_result;
        asm volatile ("" : : : "memory");
        final_result += volatile_array[(i + 64) % 128];
    }
    
    /* Region 3: Nested loops */
    final_result += nested_loop_scheduling(final_result % 50 + 10);
    
    /* Region 4: More complex patterns */
    int mode = 0;
    while (mode < 8) {
        /* Computed goto-like structure using switch */
        switch (mode) {
            case 0:
            case 2:
            case 4:
            case 6: {
                /* Even modes: integer intensive */
                int chain = final_result;
                for (int k = 0; k < 8; k++) {
                    chain = chain * 1103515245 + 12345;
                    chain = (chain >> 16) & 0x7FFF;
                    asm volatile ("" : "+r" (chain) : : "memory");
                }
                final_result = chain;
                break;
            }
            default: {
                /* Odd modes: mixed operations */
                float f = final_result * 0.01f;
                for (int k = 0; k < 4; k++) {
                    f = helper2(f, k * 0.5f);
                    asm volatile ("" : "+x" (f) : : "memory");
                }
                final_result += (int)f;
            }
        }
        
        /* Function call with volatile pointer */
        helper3(&final_result, 1);
        
        mode++;
    }
    
    /* Final computation with many independent operations */
    int t1 = final_result + 1;
    int t2 = final_result * 2;
    int t3 = final_result ^ 0xDEADBEEF;
    int t4 = final_result >> 4;
    int t5 = final_result & 0xFFFF;
    int t6 = t1 + t2;
    int t7 = t3 * t4;
    int t8 = t5 - t6;
    int t9 = t7 ^ t8;
    
    /* Force scheduling barrier */
    asm volatile ("mfence" : : : "memory");
    
    final_result = t9 + helper1(t9, seed);
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    
    return output;
}
