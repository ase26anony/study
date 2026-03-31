/* Test program to exercise haifa-sched.cc free_sched_context logic */
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
    t10 = t4 / (b + 1);
    t11 = t9 - t10;
    t12 = t6 << 2;
    t13 = t7 >> 1;
    t14 = t11 * t12;
    t15 = t13 + t14;
    t16 = t8 ^ t15;
    t17 = t10 & t16;
    t18 = t14 | t17;
    t19 = t15 * t18;
    t20 = t16 + t19;
    
    /* Memory operations to introduce load/store dependencies */
    volatile int mem1 = t20;
    int mem2 = mem1;
    volatile int mem3 = mem2 + t19;
    
    return t20 + mem3;
}

/* Helper with inline assembly barriers to force state restoration */
static int asm_barrier_sequence(int a, int b, int c) {
    int result = 0;
    
    /* Initial computation */
    int t1 = a * b;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Dependent computation */
    int t2 = t1 + c;
    
    /* Another barrier with register constraints */
    int t3;
    asm volatile ("addl %1, %0" : "=r"(t3) : "r"(t2), "0"(a));
    
    /* Complex inline assembly with multiple outputs */
    int t4, t5;
    asm volatile ("imull %2, %1\n\t"
                  "addl %3, %0" 
                  : "=r"(t4), "=r"(t5) 
                  : "r"(t3), "r"(b), "0"(t2), "1"(a));
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    return t4 + t5;
}

/* Helper with vector intrinsics to trigger target-specific scheduling */
static int vector_intrinsics_mix(int a, int b, int c, int d) {
    /* Create vector operations that use MMX/SSE scheduling */
    __m128i vec1 = _mm_set_epi32(a, b, c, d);
    __m128i vec2 = _mm_set1_epi32(seed1);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    
    /* Floating point vector ops */
    __m128 fvec1 = _mm_set_ps(a, b, c, d);
    __m128 fvec2 = _mm_set1_ps(seed2 * 0.01f);
    __m128 fvec3 = _mm_add_ps(fvec1, fvec2);
    
    /* Mix vector and scalar operations */
    int arr[4];
    _mm_storeu_si128((__m128i*)arr, vec3);
    
    float farr[4];
    _mm_storeu_ps(farr, fvec3);
    
    /* Complex dependency chain */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr[i] + (int)farr[i];
    }
    
    /* Additional scalar ops to create scheduling complexity */
    sum = sum * a - b + c * d;
    
    return sum;
}

/* Helper with unpredictable branching for speculative scheduling */
static int branching_sequence(int a, int b, int limit) {
    int result = 0;
    
    /* Loop with volatile condition to prevent optimization */
    for (int i = 0; i < (volatile int)limit; ++i) {
        /* Unpredictable branch */
        if (rand() % 100 > 50) {
            /* Branch target 1: arithmetic heavy */
            int t1 = a * i;
            int t2 = b + i;
            result += t1 - t2;
            
            /* Inline asm in branch */
            asm volatile ("addl %1, %0" : "+r"(result) : "r"(seed3));
        } else {
            /* Branch target 2: different pattern */
            int t1 = a + i;
            int t2 = b * i;
            result += t1 ^ t2;
            
            /* Memory operation in other branch */
            volatile int mem = t2;
            result -= mem;
        }
        
        /* Small pure function call acting as scheduling barrier */
        result = result * 2 + 1;
    }
    
    return result;
}

/* Helper with mixed operation types for complex scheduling */
static int mixed_operations(int a, int b, int c) {
    /* Start with integer ops */
    int i1 = a * b + c;
    int i2 = (a << 3) | (b & 0xFF);
    
    /* Floating point ops */
    float f1 = (float)i1 * 0.5f;
    float f2 = (float)i2 * 1.5f;
    
    /* Memory access pattern */
    volatile int mem[4] = {a, b, c, i1};
    int load1 = mem[0];
    int load2 = mem[2];
    
    /* More integer ops with memory results */
    int i3 = load1 * load2;
    mem[3] = i3;
    
    /* Vector-style operations using generic C */
    int vec_op1 = (i1 + i2) * (i3 - a);
    int vec_op2 = (b << 2) + (c >> 1);
    
    /* Final computation with many dependencies */
    int result = i1 + i2 + i3 + vec_op1 + vec_op2 + (int)f1 + (int)f2;
    
    /* Ensure all operations are used */
    return result + mem[1] + mem[3];
}

int main(int argc, char **argv) {
    /* Initialize volatile seeds from command line to prevent constant folding */
    seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Initialize random seed for unpredictable branches */
    srand(seed1);
    
    int total = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the parameters to create different scheduling patterns */
        int a = seed1 + iter;
        int b = seed2 - iter;
        int c = seed3 * iter;
        int d = iter % 100;
        
        /* Call different helpers to exercise various scheduling scenarios */
        total += dense_arithmetic(a, b, c, d);
        total += asm_barrier_sequence(a, b, c);
        total += vector_intrinsics_mix(a, b, c, d);
        total += branching_sequence(a, b, 10 + (iter % 5));
        total += mixed_operations(a, b, c);
        
        /* Occasionally add more complex patterns */
        if (iter % 7 == 0) {
            /* Nested loop with volatile counter */
            volatile int inner_limit = 3;
            for (int j = 0; j < inner_limit; j++) {
                total += dense_arithmetic(a + j, b - j, c * j, d);
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
