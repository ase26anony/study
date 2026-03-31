/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Noinline functions to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    return result;
}

__attribute__((noinline)) float helper2(float a, float b) {
    volatile float result = a * b - a / (b + 1.0f);
    return result;
}

__attribute__((noinline)) void helper3(volatile int* ptr) {
    *ptr = (*ptr << 3) | (*ptr >> 5);
}

/* Function with mixed operations to create complex scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int barrier = seed;
    int result = 0;
    
    /* Region 1: Mixed integer operations with volatile accesses */
    {
        volatile int* p = &barrier;
        int a = seed + 1;
        int b = seed * 2;
        int c = seed / 3;
        
        /* Chain of dependent operations */
        a = b + c;
        b = a * seed;
        c = b >> 2;
        a = c ^ 0xABCD;
        
        /* Volatile access creates scheduling barrier */
        *p = *p + a;
        
        /* Inline assembly with clobbers */
        asm volatile ("addl %%ebx, %%eax\n\t"
                      "rorl $3, %%eax"
                      : "=a"(a)
                      : "a"(a), "b"(b)
                      : "cc");
        
        result += a + *p;
    }
    
    /* Region 2: SIMD/vector operations */
    {
        __m128i vec1 = _mm_set_epi32(seed, seed+1, seed+2, seed+3);
        __m128i vec2 = _mm_set_epi32(seed+4, seed+5, seed+6, seed+7);
        __m128i vec3 = _mm_add_epi32(vec1, vec2);
        __m128i vec4 = _mm_mullo_epi32(vec3, vec1);
        
        int temp[4];
        _mm_storeu_si128((__m128i*)temp, vec4);
        
        /* Mix with integer operations */
        for (int i = 0; i < 4; i++) {
            temp[i] = (temp[i] << i) | (temp[i] >> (32 - i));
        }
        
        /* Architecture-specific builtins */
        result += __builtin_popcount(temp[0]) + 
                  __builtin_ctz(temp[1] | 1) + 
                  __builtin_clz(temp[2] | 1);
    }
    
    /* Region 3: Nested control flow with many instructions */
    switch (seed % 5) {
        case 0: {
            /* Many independent instructions for parallel scheduling */
            int x1 = result + 1;
            int x2 = result * 2;
            int x3 = result & 0xFF;
            int x4 = result | 0x1234;
            int x5 = result ^ 0x5678;
            int x6 = result << 2;
            int x7 = result >> 3;
            int x8 = ~result;
            
            /* Create dependencies */
            x1 = x2 + x3;
            x4 = x5 * x6;
            x7 = x8 - x1;
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            
            result = x1 + x4 + x7;
            break;
        }
        case 1: {
            /* Loop with data-dependent exit */
            int i = 0;
            int acc = result;
            while (1) {
                acc = (acc * 1103515245 + 12345) & 0x7FFFFFFF;
                i++;
                if (i > (seed & 0xF)) break;
                
                /* Function call in loop */
                acc += helper1(acc, i);
                
                /* Another memory barrier */
                asm volatile ("mfence" : : : "memory");
            }
            result = acc;
            break;
        }
        case 2: {
            /* Mixed float/int operations */
            float f1 = (float)result;
            float f2 = f1 * 1.5f;
            float f3 = f2 / 3.14159f;
            
            /* Function call with float */
            f3 = helper2(f1, f2);
            
            /* Convert back to int with inline assembly */
            int iresult;
            asm volatile ("cvttss2si %1, %0"
                         : "=r"(iresult)
                         : "x"(f3));
            
            result = iresult;
            break;
        }
        case 3: {
            /* Complex pointer arithmetic */
            int arr[16];
            for (int i = 0; i < 16; i++) {
                arr[i] = result + i * 7;
            }
            
            /* Pointer chasing */
            int* ptr = arr;
            for (int i = 0; i < 8; i++) {
                ptr = arr + (*ptr & 0xF);
                result ^= *ptr;
            }
            
            /* Volatile access through function */
            helper3(&result);
            break;
        }
        case 4: {
            /* Multiple scheduling barriers */
            asm volatile ("# barrier 1" : : : "memory");
            result = result * 3 + 1;
            asm volatile ("# barrier 2" : : : "memory");
            result = (result << 4) | (result >> 28);
            asm volatile ("# barrier 3" : : : "memory");
            result = result ^ 0xDEADBEEF;
            asm volatile ("# barrier 4" : : : "memory");
            break;
        }
    }
    
    return result;
}

/* Second complex function to create interprocedural scheduling */
__attribute__((always_inline)) 
static inline int inline_helper(int x) {
    /* Inline function with many operations */
    int a = x * x;
    int b = a + x;
    int c = b >> 1;
    int d = c & 0xFFFF;
    int e = d | 0x10000;
    int f = e ^ x;
    
    /* Profile-guided optimization hint */
    if (__builtin_expect((f & 1), 0)) {
        f = f * 3 + 1;
    } else {
        f = f / 2;
    }
    
    return f;
}

/* Main function with multiple scheduling regions */
int main() {
    volatile int checksum = 0;
    
    /* Array of different types */
    int int_array[256];
    float float_array[256];
    volatile int* volatile_ptr = int_array;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 0.5f;
    }
    
    /* Region A: Multiple independent instruction chains */
    for (int i = 0; i < 100; i++) {
        int a = int_array[i];
        int b = int_array[i+1];
        int c = int_array[i+2];
        int d = int_array[i+3];
        
        /* Independent operations */
        int r1 = a + b;
        int r2 = c * d;
        int r3 = a ^ c;
        int r4 = b | d;
        int r5 = a << 2;
        int r6 = c >> 1;
        
        /* Create some dependencies */
        r1 = r2 + r3;
        r4 = r5 * r6;
        r2 = r1 ^ r4;
        
        checksum += r2;
        
        /* Memory operation with volatile */
        *volatile_ptr = checksum;
        volatile_ptr++;
    }
    
    /* Region B: Nested loops with complex exit conditions */
    for (int outer = 0; outer < 10; outer++) {
        int inner = 0;
        int acc = checksum;
        
        while (1) {
            /* Many operations in loop body */
            acc = inline_helper(acc);
            acc += helper1(acc, inner);
            
            /* Mixed float/int */
            float fval = float_array[inner & 0xFF];
            fval = helper2(fval, fval * 2.0f);
            acc += (int)fval;
            
            /* SIMD operation */
            __m128i v1 = _mm_set1_epi32(acc);
            __m128i v2 = _mm_set1_epi32(inner);
            __m128i v3 = _mm_add_epi32(v1, v2);
            int temp[4];
            _mm_storeu_si128((__m128i*)temp, v3);
            acc += temp[0] + temp[1];
            
            inner++;
            
            /* Complex exit condition */
            if (inner > (outer * 5) || (acc & 0xFFF) == 0)
                break;
        }
        
        checksum ^= acc;
    }
    
    /* Region C: Switch with computed goto-like flow */
    for (int i = 0; i < 20; i++) {
        int val = complex_scheduling_region(checksum + i);
        
        switch (val & 7) {
            case 0: {
                /* Many instructions in case */
                int t1 = val * 2;
                int t2 = val + 0x1234;
                int t3 = t1 ^ t2;
                int t4 = t3 << 3;
                int t5 = t4 >> 1;
                int t6 = t5 | 0xAA;
                int t7 = t6 & 0x55;
                checksum += t7;
                break;
            }
            case 1:
            case 2: {
                /* Fall-through case with operations */
                checksum = (checksum * 1103515245 + 12345);
                /* Intentional fall-through */
            }
            case 3: {
                checksum ^= val;
                break;
            }
            default: {
                checksum = checksum * 3 + 1;
                /* Inline assembly with register constraints */
                asm volatile ("imull %%ecx, %%eax\n\t"
                             "addl $0x123, %%eax"
                             : "+a"(checksum)
                             : "c"(val)
                             : "cc");
            }
        }
    }
    
    /* Region D: Final mixing with all operation types */
    {
        /* Integer chain */
        int final = checksum;
        for (int i = 0; i < 8; i++) {
            final = (final << 5) | (final >> 27);
            final = final ^ int_array[i];
            final = final + (final * 2);
        }
        
        /* SIMD final reduction */
        __m128i vsum = _mm_setzero_si128();
        for (int i = 0; i < 64; i += 4) {
            __m128i v = _mm_loadu_si128((__m128i*)&int_array[i]);
            vsum = _mm_add_epi32(vsum, v);
        }
        
        int sum_arr[4];
        _mm_storeu_si128((__m128i*)sum_arr, vsum);
        final += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];
        
        /* Volatile write */
        volatile int* final_ptr = &int_array[0];
        *final_ptr = final;
        
        checksum = final;
    }
    
    return checksum & 0x7FFFFFFF; /* Ensure positive return */
}
