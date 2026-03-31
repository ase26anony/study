/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <stdio.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int* mem = (volatile int*)malloc(sizeof(int));
    *mem = a + b;
    int result = *mem;
    free((void*)mem);
    return result;
}

__attribute__((noinline)) int helper2(int a, int b) {
    asm volatile ("# Helper2 start" : : : "memory");
    int c = a * b;
    asm volatile ("# Helper2 middle" : : : "eax", "ebx", "ecx");
    c = c >> (b & 3);
    asm volatile ("# Helper2 end" : : : "memory");
    return c;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float* fptr = (volatile float*)&a;
    *fptr = *fptr + b;
    return *fptr * 2.0f;
}

/* Function with complex scheduling requirements */
__attribute__((noinline, optimize("O3"))) 
int complex_scheduling_region(int seed) {
    int a = seed, b = seed * 2, c = seed * 3;
    volatile int barrier = 0;
    
    /* Region 1: Mixed operations with volatile and inline asm */
    for (int i = 0; i < 10; i++) {
        a = a + b + i;
        asm volatile ("# Loop barrier %0" : "+r"(a) : : "memory", "eax");
        barrier = a;
        c = c ^ (barrier * i);
        
        /* Vector operations to trigger target-specific scheduling */
        __m128i v1 = _mm_set_epi32(a, b, c, i);
        __m128i v2 = _mm_set_epi32(i, a, b, c);
        __m128i v3 = _mm_add_epi32(v1, v2);
        int vresult[4];
        _mm_storeu_si128((__m128i*)vresult, v3);
        a += vresult[0];
    }
    
    return a + b + c;
}

/* Another scheduling region with different characteristics */
__attribute__((noinline, optimize("O2")))
int switch_based_scheduling(int x) {
    int result = 0;
    
    /* Complex switch to create multiple basic blocks */
    switch (x & 7) {
        case 0: {
            /* Many independent instructions */
            int t1 = x + 1;
            int t2 = x * 2;
            int t3 = x & 0xFF;
            int t4 = x | 0x55;
            int t5 = x ^ t1;
            int t6 = t2 << 2;
            int t7 = t3 >> 1;
            int t8 = t4 + t5;
            int t9 = t6 * t7;
            int t10 = __builtin_popcount(t8);
            result = t9 + t10;
            asm volatile ("# Case 0 done" : : : "memory");
            break;
        }
        case 1: {
            /* Memory intensive */
            volatile int* arr = (volatile int*)malloc(64 * sizeof(int));
            for (int i = 0; i < 64; i++) {
                arr[i] = x + i;
                result += arr[i];
                if (i & 1) {
                    result = helper1(result, i);
                }
            }
            free((void*)arr);
            break;
        }
        case 2: {
            /* Floating point mix */
            float f1 = x * 1.5f;
            float f2 = x * 2.5f;
            for (int i = 0; i < 8; i++) {
                f1 = helper3(f1, f2);
                f2 = helper3(f2, f1);
                result += (int)f1 + (int)f2;
            }
            break;
        }
        case 3: {
            /* Inline assembly with clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull %%eax, %%eax\n\t"
                "addl $1, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r"(result)
                : "r"(x)
                : "eax", "memory", "cc"
            );
            break;
        }
        default: {
            /* Long dependency chain */
            result = x;
            for (int i = 0; i < 20; i++) {
                result = helper2(result, i);
                if (result & 1) {
                    result = helper1(result, i);
                } else {
                    result = helper2(i, result);
                }
                asm volatile ("# Default loop" : "+r"(result) : : "memory");
            }
        }
    }
    
    return result;
}

/* Function with nested loops and computed goto */
__attribute__((noinline))
int computed_goto_schedule(int init) {
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    int val = init;
    int i = 0;
    
    /* Outer loop */
    while (i < 100) {
        /* Computed goto creates complex control flow */
        goto *labels[val % 5];
        
        L0: {
            /* Vector operations */
            __m128i v = _mm_set1_epi32(val);
            for (int j = 0; j < 4; j++) {
                v = _mm_add_epi32(v, _mm_set1_epi32(j));
                int temp[4];
                _mm_storeu_si128((__m128i*)temp, v);
                val += temp[0] + temp[1] + temp[2] + temp[3];
            }
            i += 4;
            continue;
        }
        
        L1: {
            /* Memory barrier and volatile */
            volatile int counter = 0;
            for (int j = 0; j < 8; j++) {
                counter = val + j;
                asm volatile ("# L1 loop" : : "r"(counter) : "memory");
                val = helper1(val, counter);
            }
            i += 8;
            continue;
        }
        
        L2: {
            /* Mixed integer/float */
            float fval = (float)val;
            for (int j = 0; j < 6; j++) {
                fval = helper3(fval, (float)j);
                val += (int)fval;
                if (j & 1) {
                    val = helper2(val, j);
                }
            }
            i += 6;
            continue;
        }
        
        L3: {
            /* Many independent operations */
            int t1 = val + 1;
            int t2 = val * 2;
            int t3 = val & 0xFF;
            int t4 = val | 0xAA;
            int t5 = t1 ^ t2;
            int t6 = t3 * t4;
            int t7 = __builtin_ctz(t5 | 1);
            int t8 = __builtin_popcount(t6);
            val = t5 + t6 + t7 + t8;
            i++;
            continue;
        }
        
        L4: {
            /* Function call chain */
            val = helper1(val, i);
            val = helper2(val, i + 1);
            val = helper1(val, i + 2);
            val = helper2(val, i + 3);
            i += 4;
            continue;
        }
    }
    
    return val;
}

/* Main function with multiple scheduling regions */
int main() {
    int result = 0;
    
    /* Region 1: Complex scheduling with vector ops */
    result += complex_scheduling_region(42);
    
    /* Region 2: Switch-based scheduling */
    for (int i = 0; i < 10; i++) {
        result += switch_based_scheduling(result + i);
        /* Insert memory barrier */
        asm volatile ("# Main loop barrier" : : : "memory");
    }
    
    /* Region 3: Computed goto with nested scheduling */
    result += computed_goto_schedule(result);
    
    /* Region 4: Manual unrolling with mixed operations */
    {
        int a = result, b = result * 2, c = result * 3;
        
        /* Unrolled loop with scheduling barriers */
        #pragma GCC unroll 4
        for (int i = 0; i < 16; i++) {
            a = a + b + i;
            asm volatile ("# Unrolled barrier" : "+r"(a) : : "memory");
            c = c ^ (a * i);
            
            /* Every 4 iterations, do something different */
            if ((i & 3) == 0) {
                __m128i v = _mm_set_epi32(a, b, c, i);
                int temp[4];
                _mm_storeu_si128((__m128i*)temp, v);
                b += temp[0] + temp[1];
            }
            
            /* Volatile access every other iteration */
            if (i & 1) {
                volatile int* ptr = &a;
                *ptr = *ptr + 1;
            }
        }
        result = a + b + c;
    }
    
    /* Region 5: Profile-guided style with __builtin_expect */
    {
        int unlikely_sum = 0;
        for (int i = 0; i < 100; i++) {
            if (__builtin_expect((i & 15) == 0, 0)) {
                /* Unlikely path - complex operations */
                unlikely_sum += helper1(i, result);
                unlikely_sum += helper2(unlikely_sum, i);
                __m128i v = _mm_set1_epi32(unlikely_sum);
                int temp[4];
                _mm_storeu_si128((__m128i*)temp, v);
                unlikely_sum += temp[0];
            } else {
                /* Likely path - simple operations */
                unlikely_sum += i;
            }
        }
        result ^= unlikely_sum;
    }
    
    /* Prevent dead code elimination */
    volatile int output = result;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
