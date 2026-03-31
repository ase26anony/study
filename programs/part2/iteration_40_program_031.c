/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile globals to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int* g_volatile_ptr = &g_volatile_counter;
volatile float g_float_array[256] = {0};

/* Noinline functions to create scheduling barriers */
__attribute__((noinline)) int noinline_add(int a, int b) {
    asm volatile ("" : : : "memory");
    return a + b;
}

__attribute__((noinline)) float noinline_mul(float a, float b) {
    asm volatile ("" : : : "memory", "xmm0", "xmm1");
    return a * b;
}

__attribute__((noinline)) void noinline_side_effect(int* p) {
    *p = (*p * 37) ^ 0xDEADBEEF;
    asm volatile ("" : : : "memory");
}

/* Function with complex scheduling requirements */
__attribute__((noinline, optimize("O3"))) 
int complex_scheduling_region(int seed) {
    int a = seed;
    int b = a * 2;
    int c = b + 0x12345678;
    volatile int* vp = g_volatile_ptr;
    
    /* Mixed integer operations with dependencies */
    a = b + c;
    b = a * seed;
    c = b >> 3;
    a = c ^ 0xABCDEF;
    b = a & 0xFF00FF;
    
    /* Volatile access creates scheduling barrier */
    *vp = *vp + 1;
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (c)
        : "r" (b)
        : "eax", "memory"
    );
    
    /* SIMD operations for target-specific scheduling */
    __m128i vec1 = _mm_set_epi32(a, b, c, seed);
    __m128i vec2 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    __m128i vec4 = _mm_mullo_epi32(vec3, vec2);
    
    /* Extract results */
    int results[4];
    _mm_storeu_si128((__m128i*)results, vec4);
    
    /* More arithmetic with results */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += results[i] * (i + 1);
    }
    
    /* Function call creates another scheduling point */
    sum = noinline_add(sum, *vp);
    
    return sum;
}

/* Function with nested control flow */
__attribute__((optimize("O2")))
int nested_control_flow(int base) {
    int result = base;
    
    /* Switch with multiple cases */
    switch (base & 0x7) {
        case 0: {
            /* Case with many independent instructions */
            int t1 = result + 1;
            int t2 = result * 2;
            int t3 = result & 0xFF;
            int t4 = result | 0xAA;
            int t5 = result ^ t1;
            int t6 = t2 + t3;
            int t7 = t4 - t5;
            int t8 = t6 * t7;
            result = t8 >> 2;
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            
            /* More operations */
            float f1 = (float)result;
            float f2 = f1 * 3.14159f;
            float f3 = f2 / 2.0f;
            result = (int)(f3 * 100.0f);
            break;
        }
        case 1: {
            /* Loop with data-dependent exit */
            int i = 0;
            while (i < (base & 0xF)) {
                result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
                i++;
                
                /* Inline assembly in loop */
                asm volatile (
                    "addl $1, %0\n\t"
                    : "+r" (result)
                    : 
                    : "cc"
                );
            }
            break;
        }
        case 2: {
            /* Mixed float/int operations */
            for (int j = 0; j < 8; j++) {
                float f = (float)result;
                f = noinline_mul(f, 1.5f);
                result = (int)f;
                result ^= (j << 3);
            }
            break;
        }
        default: {
            /* Default case with vector operations */
            __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 v2 = _mm_set_ps((float)result, (float)(result+1), 
                                  (float)(result+2), (float)(result+3));
            __m128 v3 = _mm_add_ps(v1, v2);
            __m128 v4 = _mm_mul_ps(v3, v1);
            
            float farr[4];
            _mm_storeu_ps(farr, v4);
            
            for (int k = 0; k < 4; k++) {
                result += (int)farr[k];
            }
            break;
        }
    }
    
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    int checksum = 0;
    
    /* Initialize some data */
    int int_array[256];
    for (int i = 0; i < 256; i++) {
        int_array[i] = i * 3;
        g_float_array[i] = (float)i * 0.5f;
    }
    
    /* Region 1: Complex scheduling with SIMD */
    checksum ^= complex_scheduling_region(0x1234);
    
    /* Region 2: Nested loops with volatile accesses */
    for (int outer = 0; outer < 4; outer++) {
        int temp = checksum;
        
        /* Inner loop with many operations */
        for (int inner = 0; inner < 16; inner++) {
            /* Independent instructions for parallel scheduling */
            int a = temp + inner;
            int b = temp * inner;
            int c = temp & inner;
            int d = temp | inner;
            int e = temp ^ inner;
            
            /* Volatile access */
            *g_volatile_ptr = *g_volatile_ptr + a;
            
            /* Function call */
            noinline_side_effect(&temp);
            
            /* More arithmetic */
            a = b + c;
            b = d * e;
            c = a >> (inner & 3);
            d = b ^ c;
            e = d & 0xFFFF;
            
            temp = e;
        }
        
        checksum += temp;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Region 3: Switch with computed goto-like behavior */
    for (int i = 0; i < 8; i++) {
        checksum = nested_control_flow(checksum + i);
    }
    
    /* Region 4: Mixed operations in tight loop */
    {
        __m128i simd_acc = _mm_setzero_si128();
        
        for (int i = 0; i < 64; i++) {
            /* Load 4 integers */
            __m128i data = _mm_loadu_si128((__m128i*)&int_array[i*4]);
            
            /* Various SIMD operations */
            __m128i mul = _mm_mullo_epi32(data, _mm_set1_epi32(i));
            __m128i add = _mm_add_epi32(mul, simd_acc);
            __m128i shift = _mm_slli_epi32(add, 1);
            __m128i xor_op = _mm_xor_si128(shift, _mm_set1_epi32(0xAAAAAAAA));
            
            simd_acc = xor_op;
            
            /* Scalar operations mixed in */
            checksum += int_array[i] * i;
            
            /* Conditional with unlikely path */
            if (__builtin_expect((i & 0x7) == 0, 0)) {
                /* Unlikely path with different operations */
                float f = g_float_array[i];
                f = f * f + 1.0f;
                checksum += (int)f;
                
                /* Assembly with specific register constraints */
                asm volatile (
                    "movl %1, %%ecx\n\t"
                    "roll $3, %%ecx\n\t"
                    "movl %%ecx, %0\n\t"
                    : "=r" (checksum)
                    : "r" (checksum)
                    : "ecx", "cc"
                );
            }
        }
        
        /* Extract SIMD result */
        int simd_results[4];
        _mm_storeu_si128((__m128i*)simd_results, simd_acc);
        checksum += simd_results[0] + simd_results[1] + 
                   simd_results[2] + simd_results[3];
    }
    
    /* Region 5: Deeply nested control flow */
    {
        int x = checksum;
        int y = 0;
        
        /* Complex nested if-else chain */
        if (x > 1000) {
            if (x < 10000) {
                for (int i = 0; i < (x % 10); i++) {
                    y = (y * 13 + 7) & 0xFFF;
                    y ^= x;
                    
                    /* Inline assembly with memory clobber */
                    asm volatile (
                        "movl %1, %%eax\n\t"
                        "addl %%eax, %0\n\t"
                        : "+r" (y)
                        : "r" (i)
                        : "eax", "memory"
                    );
                }
            } else {
                y = x * x;
                y = y % 7919;
            }
        } else {
            switch (x & 3) {
                case 0: y = x + 1; break;
                case 1: y = x * 2; break;
                case 2: y = x ^ 0x55; break;
                case 3: y = x & 0xAA; break;
            }
            
            /* More operations after switch */
            y = y << 2;
            y = y | 1;
            y = noinline_add(y, x);
        }
        
        checksum = y;
    }
    
    /* Final computation to prevent dead code elimination */
    checksum = (checksum * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return checksum;
}
