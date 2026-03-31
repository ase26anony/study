/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Helper function with pure computation - creates scheduling region */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a & b;
    int t5 = c | d;
    int t6 = t3 - t4;
    int t7 = t5 + t6;
    int t8 = t1 * t2;
    int t9 = t3 / (t4 ? t4 : 1);
    int t10 = t5 ^ t6;
    int t11 = t7 & t8;
    int t12 = t9 | t10;
    int t13 = t11 - t12;
    int t14 = t13 * t1;
    int t15 = t14 + t2;
    int t16 = t15 ^ t3;
    int t17 = t16 & t4;
    int t18 = t17 | t5;
    int t19 = t18 - t6;
    int t20 = t19 * t7;
    return t20;
}

/* Function with inline assembly barriers */
static int asm_barrier_test(int a, int b, int c) {
    int result;
    
    /* First computation */
    int temp1 = a * b;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Dependent computation */
    int temp2 = temp1 + c;
    
    /* Another barrier */
    asm volatile ("# This is a comment\n\t" : : : "memory");
    
    /* More computation with inline asm */
    asm volatile ("addl %1, %0" : "+r"(temp2) : "r"(a));
    
    result = temp2;
    
    /* Complex asm with multiple outputs */
    int out1, out2;
    asm volatile ("movl %2, %0\n\t"
                  "imull %3, %0\n\t"
                  "movl %2, %1\n\t"
                  "addl %3, %1"
                  : "=&r"(out1), "=&r"(out2)
                  : "r"(b), "r"(c));
    
    return result + out1 - out2;
}

/* Function using SSE intrinsics to trigger target-specific scheduling */
static float sse_test(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    
    /* Multiple vector operations */
    __m128 vec3 = _mm_add_ps(vec1, vec2);
    __m128 vec4 = _mm_mul_ps(vec1, vec2);
    __m128 vec5 = _mm_sub_ps(vec3, vec4);
    
    /* Shuffle operations */
    __m128 vec6 = _mm_shuffle_ps(vec5, vec5, _MM_SHUFFLE(0, 1, 2, 3));
    
    /* More operations */
    __m128 vec7 = _mm_add_ps(vec5, vec6);
    __m128 vec8 = _mm_mul_ps(vec7, _mm_set1_ps(2.0f));
    
    float result[4];
    _mm_storeu_ps(result, vec8);
    
    return result[0] + result[1] + result[2] + result[3];
}

/* Complex loop with unpredictable branching */
static int branching_test(int seed, int iterations) {
    volatile int vol_seed = seed; /* Prevent optimization */
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Unpredictable condition */
        if ((vol_seed + i) % 7 < 3) {
            /* Branch 1: More arithmetic */
            int a = i * 3;
            int b = vol_seed ^ i;
            asm volatile ("" : "+r"(a), "+r"(b) : : "memory");
            sum += a * b;
            
            /* Inline asm with dependencies */
            int temp;
            asm volatile ("imull %%ebx, %%eax\n\t"
                          "addl %%ecx, %%eax"
                          : "=a"(temp)
                          : "a"(a), "b"(b), "c"(sum)
                          : "cc");
            sum = temp;
        } else {
            /* Branch 2: Different operations */
            int x = i | vol_seed;
            int y = i & vol_seed;
            
            /* Memory operations */
            int* ptr = &sum;
            *ptr += x - y;
            
            /* Another asm barrier */
            asm volatile ("# Branch2 barrier\n\t" : : : "memory");
        }
        
        /* Loop-carried dependency */
        vol_seed = (vol_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return sum;
}

/* Mixed operation test with memory accesses */
static int memory_ops_test(int* arr, int size, int seed) {
    int sum = 0;
    volatile int vol_seed = seed;
    
    /* Create alias to force conservative scheduling */
    int* alias_arr = arr;
    
    for (int i = 0; i < size; i++) {
        /* Store */
        arr[i] = vol_seed + i;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Load with potential alias */
        int val = alias_arr[i];
        
        /* Computation */
        sum += val * (i + 1);
        
        /* More stores */
        if (i % 3 == 0) {
            arr[(i + 1) % size] = sum;
            asm volatile ("" : : : "memory");
        }
        
        /* Update volatile to prevent loop unrolling */
        vol_seed = (vol_seed + 1) & 0xFF;
    }
    
    return sum;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Multiple iterations to increase chance of context creation/freeing */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the iteration count per test */
        volatile int inner_iter = (seed1 + iter) % 50 + 10;
        
        /* Test 1: Pure computation with helper calls */
        int r1 = 0;
        for (int i = 0; i < inner_iter; i++) {
            r1 += helper_pure(seed1 + i, seed2 - i);
        }
        total += r1;
        
        /* Test 2: Dense arithmetic */
        int r2 = dense_arithmetic(seed1, seed2, seed3, iter);
        total ^= r2;
        
        /* Test 3: Assembly barriers */
        int r3 = asm_barrier_test(seed1, seed2, seed3);
        total += r3 * 3;
        
        /* Test 4: SSE operations */
        float r4 = sse_test(seed1 * 0.1f, seed2 * 0.2f, 
                           seed3 * 0.3f, iter * 0.4f);
        total += (int)r4;
        
        /* Test 5: Branching with unpredictable paths */
        int r5 = branching_test(seed1 + iter, 25);
        total -= r5;
        
        /* Test 6: Memory operations */
        int arr[32];
        for (int i = 0; i < 32; i++) arr[i] = i;
        int r6 = memory_ops_test(arr, 32, seed2 + iter);
        total += r6;
        
        /* Update seeds to change patterns */
        seed1 = (seed1 * 1664525 + 1013904223) & 0x7FFFFFFF;
        seed2 = (seed2 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed3 = (seed3 * 214013 + 2531011) & 0x7FFFFFFF;
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
