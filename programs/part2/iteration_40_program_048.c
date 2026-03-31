/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    asm volatile ("" : "+r" (result) : : "memory", "eax");
    return result;
}

__attribute__((noinline)) int helper2(int a, int b) {
    volatile int result = (a << 3) | (b >> 5);
    asm volatile ("nop; nop" : : : "memory", "ebx", "ecx");
    return result;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float result = a * b - a / (b + 1.0f);
    asm volatile ("" : "+x" (result) : : "memory");
    return result;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int* volatile_ptr = &seed;
    int arr_int[64];
    float arr_float[64];
    __m128i vec_data[8];
    int result = seed;
    
    /* Initialize arrays with mixed operations */
    for (int i = 0; i < 64; i++) {
        arr_int[i] = i * seed + (i ^ seed);
        arr_float[i] = (float)i * 0.1f + seed * 0.01f;
        if (i % 8 == 0) {
            vec_data[i/8] = _mm_set_epi32(i, i+1, i+2, i+3);
        }
    }
    
    /* Region 1: Integer operations with dependencies */
    int a = *volatile_ptr + 1;
    int b = a * 3 - seed;
    int c = b >> 2;
    int d = c & 0xFF;
    int e = d | (a << 8);
    int f = helper1(e, seed);
    int g = helper2(f, a);
    
    /* Memory barrier to split scheduling region */
    asm volatile ("" : : : "memory");
    
    /* Region 2: Mixed operations with SIMD */
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set_epi32(e, f, g, seed);
    __m128i v3 = _mm_add_epi32(v1, v2);
    __m128i v4 = _mm_slli_epi32(v3, 2);
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "pmulld %1, %0\n\t"
        "paddd %2, %0"
        : "+x" (v4)
        : "x" (v3), "x" (_mm_set1_epi32(7))
        : "xmm0", "xmm1"
    );
    
    /* Region 3: Floating point with volatile accesses */
    float fa = (float)a * 0.5f;
    float fb = fa + (float)b * 0.25f;
    *volatile_ptr = *volatile_ptr + 1;  /* Volatile access */
    float fc = helper3(fa, fb);
    float fd = fc * arr_float[seed % 64];
    
    /* Region 4: Pointer arithmetic and conditionals */
    int* ptr = arr_int;
    for (int i = 0; i < 16; i++) {
        ptr[i] = ptr[i] + (i % 4) * 3;
        if (ptr[i] & 1) {
            ptr[i] = helper1(ptr[i], i);
        } else {
            ptr[i] = helper2(ptr[i], i);
        }
        /* Memory clobber every 4 iterations */
        if (i % 4 == 3) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Region 5: Switch with multiple cases creating different paths */
    switch (seed % 8) {
        case 0:
            result = a + b + c + _mm_extract_epi32(v4, 0);
            asm volatile ("nop" : : : "eax", "memory");
            break;
        case 1:
            result = d * e - f + (int)fd;
            asm volatile ("nop; nop" : : : "ebx", "ecx", "memory");
            break;
        case 2:
            result = helper1(g, result) ^ helper2(result, seed);
            /* Fall through */
        case 3:
            result += arr_int[result % 64] * 3;
            asm volatile ("" : "+r" (result) : : "memory", "edx");
            break;
        case 4:
            result = __builtin_popcount(result) + __builtin_ctz(seed | 1);
            /* Vector operation in switch case */
            v4 = _mm_srli_epi32(v4, 1);
            break;
        case 5:
            result = (result << 3) | (result >> 29);
            /* Multiple independent instructions */
            int t1 = helper1(result, a);
            int t2 = helper2(result, b);
            int t3 = t1 * t2 - result;
            result = t3 & 0x7FFFFFFF;
            break;
        case 6:
            /* Complex chain */
            result = result * 1103515245 + 12345;
            result = (result & 0x7FFFFFFF) ^ seed;
            result = helper1(result, helper2(result, seed));
            break;
        case 7:
        default:
            result = ~result;
            asm volatile (
                "rorl $3, %0\n\t"
                "roll $7, %0"
                : "+r" (result)
                : 
                : "cc"
            );
            break;
    }
    
    /* Final mixing */
    result ^= _mm_extract_epi32(v4, 0);
    result ^= _mm_extract_epi32(v4, 1);
    result ^= _mm_extract_epi32(v4, 2);
    result ^= _mm_extract_epi32(v4, 3);
    result ^= (int)fd;
    
    return result;
}

/* Another complex region with nested loops */
int nested_loop_scheduler(int base) {
    volatile int counter = base;
    int matrix[8][8];
    int result = 0;
    
    /* Initialize matrix with complex pattern */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = (i * 17 + j * 13 + base) & 0xFF;
            /* Volatile access in inner loop */
            counter = counter + 1;
        }
        /* Memory barrier between rows */
        asm volatile ("" : : : "memory");
    }
    
    /* Process matrix with data-dependent exits */
    for (int i = 0; i < 8; i++) {
        int row_sum = 0;
        for (int j = 0; j < 8; j++) {
            row_sum += matrix[i][j];
            /* Early exit based on computation */
            if (row_sum > 1000) {
                row_sum = helper1(row_sum, j);
                break;
            }
            /* Mix operations */
            matrix[i][j] = matrix[i][j] ^ (i * j);
            matrix[i][j] = helper2(matrix[i][j], counter);
        }
        result ^= row_sum;
        
        /* SIMD operation in outer loop */
        if (i % 2 == 0) {
            __m128i v = _mm_set_epi32(matrix[i][0], matrix[i][1], 
                                     matrix[i][2], matrix[i][3]);
            v = _mm_add_epi32(v, _mm_set1_epi32(result));
            result += _mm_extract_epi32(v, 0);
        }
    }
    
    return result;
}

/* Function with computed goto for complex control flow */
int computed_goto_scheduler(int val) {
    static void* labels[] = {
        &&L0, &&L1, &&L2, &&L3, &&L4,
        &&L5, &&L6, &&L7, &&L8, &&L9
    };
    
    int result = val;
    int index = val % 10;
    
    goto *labels[index];
    
L0:
    result = helper1(result, val) << 1;
    asm volatile ("" : "+r" (result) : : "memory", "eax");
    /* Fall through */
L1:
    result ^= __builtin_popcount(val);
    /* Multiple independent instructions */
    int t1 = result * 3;
    int t2 = result / 2;
    int t3 = t1 ^ t2;
    result = helper2(t3, result);
    goto L9;
    
L2:
    result = result + (val & 0xF);
    /* Vector operations */
    __m128i v = _mm_set1_epi32(result);
    v = _mm_slli_epi32(v, 2);
    result = _mm_extract_epi32(v, 0);
    goto L9;
    
L3:
    result = ~result;
    asm volatile ("nop; nop; nop" : : : "memory", "ebx", "ecx");
    goto L9;
    
L4:
    /* Complex chain */
    result = result * 1103515245 + 12345;
    result = result & 0x7FFFFFFF;
    result = helper1(result, helper2(result, val));
    goto L9;
    
L5:
    result = (result << 5) | (result >> 27);
    /* Memory intensive */
    volatile int* vp = &val;
    for (int i = 0; i < 4; i++) {
        *vp = *vp + result;
        result ^= *vp;
    }
    goto L9;
    
L6:
    result = __builtin_ctz(val | 1) + __builtin_clz(val | 1);
    goto L9;
    
L7:
    result = result * 7 - 13;
    asm volatile (
        "imul %1, %0\n\t"
        "add %2, %0"
        : "+r" (result)
        : "r" (val), "r" (0x1234)
        : "cc"
    );
    goto L9;
    
L8:
    result = (result ^ val) * 0x5A5A5A5A;
    /* Multiple barriers */
    asm volatile ("" : : : "memory");
    asm volatile ("" : : : "memory");
    asm volatile ("" : : : "memory");
    goto L9;
    
L9:
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    int final_result = 0;
    
    /* Change optimization pragma mid-function */
    #pragma GCC optimize("O3")
    final_result += complex_scheduling_region(42);
    
    #pragma GCC optimize("O2")
    final_result += nested_loop_scheduler(final_result);
    
    #pragma GCC optimize("O3")
    final_result += computed_goto_scheduler(final_result);
    
    /* Additional complex region */
    volatile int seed = final_result;
    for (int i = 0; i < 100; i++) {
        seed = seed * 1664525 + 1013904223;
        int temp = seed & 0xFF;
        
        /* Mix of operations in loop */
        if (temp & 1) {
            final_result += helper1(final_result, temp);
        } else if (temp & 2) {
            final_result ^= helper2(final_result, temp);
        } else {
            final_result = (final_result << 3) | (final_result >> 29);
        }
        
        /* SIMD every 16 iterations */
        if (i % 16 == 0) {
            __m128i v = _mm_set1_epi32(final_result);
            v = _mm_add_epi32(v, _mm_set1_epi32(i));
            final_result += _mm_extract_epi32(v, 0);
        }
        
        /* Memory barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use __builtin_expect for profile-guided optimization hints */
    if (__builtin_expect((final_result & 1), 0)) {
        final_result = complex_scheduling_region(final_result);
    } else {
        final_result = nested_loop_scheduler(final_result);
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int output = final_result;
    asm volatile ("" : : "r" (output) : "memory");
    
    return final_result & 0x7FFFFFFF;  /* Ensure positive return */
}
