/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile seeds to prevent constant propagation */
static volatile int seed1, seed2, seed3;

/* Helper with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations to fill scheduler structures */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a * c + b;
    t5 = b * d - a;
    t6 = t3 & t4;
    t7 = t5 | t6;
    t8 = t2 + t7;
    t9 = t1 * t8;
    t10 = t9 / (c + 1);
    
    /* More operations with varying dependencies */
    t11 = t10 << 2;
    t12 = t11 >> 1;
    t13 = t12 + t3;
    t14 = t13 * t4;
    t15 = t14 - t5;
    t16 = t15 & t6;
    t17 = t16 | t7;
    t18 = t17 ^ t8;
    t19 = t18 + t9;
    t20 = t19 * t10;
    
    /* Memory operations to introduce load/store dependencies */
    volatile int mem1 = t20;
    volatile int mem2 = t11;
    int t21 = mem1 + mem2;
    
    /* Final mixing */
    return (t21 + t12 + t13 + t14) & 0xFFFF;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_sequence(int a, int b, int c) {
    int result = a;
    
    /* Sequence with assembly barriers that scheduler might try to move across */
    for (int i = 0; i < (seed2 & 0x3) + 2; ++i) {
        int temp;
        
        /* Dependent chain with barrier in middle */
        result = result * b + c;
        
        /* Assembly barrier - scheduler may attempt and fail to move instructions */
        asm volatile ("" : "=r"(temp) : "0"(result) : "memory");
        
        result = temp + i;
        
        /* Another barrier with clobber */
        asm volatile ("# barrier" : : : "memory");
        
        /* More computation */
        result = result ^ (b << i);
    }
    
    /* Unpredictable branch that might cause speculative scheduling */
    if (seed3 & 0x1) {
        asm volatile ("# branch taken" : : : "memory");
        result = result * 3;
    } else {
        asm volatile ("# branch not taken" : : : "memory");
        result = result / 2;
    }
    
    return result;
}

/* Function using vector intrinsics to trigger target-specific scheduling */
static int vector_intrinsics_mix(int a, int b) {
    /* Use SSE intrinsics - these often engage target-specific scheduling */
    __m128 vec1 = _mm_set_ps(a, b, a + b, a - b);
    __m128 vec2 = _mm_set_ps(b, a, b * 2, a * 3);
    
    /* Multiple vector operations */
    __m128 vec3 = _mm_add_ps(vec1, vec2);
    __m128 vec4 = _mm_mul_ps(vec1, vec2);
    __m128 vec5 = _mm_sub_ps(vec3, vec4);
    
    /* Mix with scalar operations */
    float temp[4];
    _mm_storeu_ps(temp, vec5);
    
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        result += (int)(temp[i] * 100);
    }
    
    /* Additional integer vector operations */
    __m128i ivec1 = _mm_set_epi32(a, b, result, a ^ b);
    __m128i ivec2 = _mm_set_epi32(b, a, b & a, b | a);
    __m128i ivec3 = _mm_add_epi32(ivec1, ivec2);
    
    /* Extract results */
    int res_arr[4];
    _mm_storeu_si128((__m128i*)res_arr, ivec3);
    
    return result + res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
}

/* Complex loop with varying trip counts and inline assembly */
static int loop_with_variable_schedule(int base, int iterations) {
    int sum = 0;
    volatile int mod = seed1 & 0x7;
    
    /* Loop with volatile bound - scheduler may create different contexts per iteration */
    for (int i = 0; i < (iterations + mod); ++i) {
        int val = base + i;
        
        /* Inline assembly with dependencies */
        int doubled;
        asm volatile ("add %1, %1, %1" : "=r"(doubled) : "r"(val));
        
        /* More operations with resource constraints */
        int tripled;
        asm volatile ("add %0, %1, %2" : "=r"(tripled) : "r"(doubled), "r"(val));
        
        /* Conditional with unpredictable outcome */
        if ((val ^ seed2) & 0x1) {
            asm volatile ("# conditional path" : : : "memory");
            sum += tripled * 2;
        } else {
            asm volatile ("# alternate path" : : : "memory");
            sum += doubled / 2;
        }
        
        /* Memory barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile ("# periodic barrier" : : : "memory");
        }
    }
    
    return sum;
}

/* Pure function that acts as scheduling barrier */
static int pure_helper(int x, int y) {
    return (x * y) + (x ^ y) - (x & y);
}

/* Function with calls to pure helpers in tight loop */
static int pure_function_sequence(int start, int count) {
    int result = start;
    volatile int v = seed3;
    
    for (int i = 0; i < (count + (v & 0x3)); ++i) {
        /* Call pure function - scheduler may try speculative motion around it */
        result = pure_helper(result, i);
        
        /* Volatile access prevents reordering */
        int tmp = v;
        
        /* More computation */
        result = result ^ tmp;
        result = result * 2 + 1;
    }
    
    return result;
}

/* Main driver that exercises all patterns */
int main(int argc, char **argv) {
    /* Initialize volatile seeds from argv to prevent constant propagation */
    seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Execute multiple iterations to increase chance of context creation/freeing */
    for (int iter = 0; iter < 100; ++iter) {
        /* Mix different scheduling patterns in each iteration */
        
        /* 1. Dense arithmetic to fill instruction queues */
        total += dense_arithmetic(seed1 + iter, seed2, seed3, iter);
        
        /* 2. Sequence with assembly barriers */
        total += asm_barrier_sequence(total, seed2, iter);
        
        /* 3. Vector intrinsics for target-specific scheduling */
        if (iter % 4 == 0) {
            total += vector_intrinsics_mix(total & 0xFF, iter & 0xFF);
        }
        
        /* 4. Loop with variable schedule */
        total += loop_with_variable_schedule(total, (iter & 0x7) + 3);
        
        /* 5. Pure function sequence */
        total += pure_function_sequence(total, (iter & 0x3) + 2);
        
        /* Modify seeds slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
