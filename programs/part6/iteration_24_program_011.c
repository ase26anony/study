/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation - creates scheduling barriers */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b);
}

/* Helper with memory dependencies */
static void memory_helper(volatile int* in, volatile int* out, int count) {
    for (int i = 0; i < count; i++) {
        out[i] = in[i] * 2 + 1;
        /* Create anti-dependency */
        in[i] = out[i] >> 1;
    }
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t3 - e;
    int t5 = t4 * 7;
    int t6 = t5 & 0xFF;
    int t7 = t6 | a;
    int t8 = t7 << 3;
    int t9 = t8 - b;
    int t10 = t9 * c;
    int t11 = t10 ^ d;
    int t12 = t11 + e;
    int t13 = t12 * 2;
    int t14 = t13 & 0xFFFF;
    int t15 = t14 | t1;
    int t16 = t15 - t2;
    int t17 = t16 * 3;
    int t18 = t17 ^ t3;
    int t19 = t18 + t4;
    int t20 = t19 * 5;
    int t21 = t20 & 0xFFF;
    int t22 = t21 | t5;
    int t23 = t22 - t6;
    int t24 = t23 * 11;
    int t25 = t24 ^ t7;
    int t26 = t25 + t8;
    int t27 = t26 * 13;
    int t28 = t27 & 0xFFFFFF;
    int t29 = t28 | t9;
    int t30 = t29 - t10;
    
    return t30;
}

/* Function with SSE intrinsics - triggers target-specific scheduling */
static __m128 vector_computation(__m128 a, __m128 b, __m128 c) {
    __m128 t1 = _mm_add_ps(a, b);
    __m128 t2 = _mm_mul_ps(t1, c);
    __m128 t3 = _mm_sub_ps(t2, a);
    __m128 t4 = _mm_mul_ps(t3, _mm_set1_ps(2.0f));
    __m128 t5 = _mm_add_ps(t4, b);
    __m128 t6 = _mm_mul_ps(t5, _mm_set1_ps(1.5f));
    __m128 t7 = _mm_sub_ps(t6, c);
    __m128 t8 = _mm_add_ps(t7, t1);
    __m128 t9 = _mm_mul_ps(t8, _mm_set1_ps(0.5f));
    
    return t9;
}

/* Function with inline assembly barriers - forces state restoration */
static int asm_barrier_computation(int a, int b, int c, int d) {
    int result;
    
    /* First computation cluster */
    int t1 = a + b;
    
    /* Assembly barrier that looks schedulable but has memory clobber */
    asm volatile (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r"(result)
        : "r"(t1), "r"(c)
        : "%eax", "memory"
    );
    
    /* Dependent computation after barrier */
    int t2 = result * d;
    
    /* Another barrier with different constraints */
    asm volatile (
        "imul %1, %0\n\t"
        "add $42, %0\n\t"
        : "+r"(t2)
        : "r"(a)
        : "cc", "memory"
    );
    
    /* Final computation with another barrier */
    int t3;
    asm volatile (
        "mov %1, %%ecx\n\t"
        "xor %%edx, %%edx\n\t"
        "div %2\n\t"
        "mov %%eax, %0\n\t"
        : "=r"(t3)
        : "r"(t2), "r"(b + 1)
        : "%eax", "%edx", "%ecx", "cc", "memory"
    );
    
    return t3;
}

/* Complex loop with unpredictable branching - creates multiple contexts */
static int branching_computation(volatile int seed, int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile read makes branch unpredictable */
        int mod = seed % 7;
        
        if (mod == 0) {
            /* Path 1: Dense computation */
            total += dense_arithmetic(i, i+1, i+2, i+3, i+4);
        } 
        else if (mod == 1 || mod == 2) {
            /* Path 2: Pure helper calls */
            total += pure_helper(i, i*2, i*3);
            total += pure_helper(total, i, seed);
        }
        else if (mod == 3) {
            /* Path 3: Memory operations */
            volatile int buf1[4] = {i, i+1, i+2, i+3};
            volatile int buf2[4];
            memory_helper(buf1, buf2, 4);
            total += buf2[0] + buf2[3];
        }
        else {
            /* Path 4: Mixed operations */
            total += asm_barrier_computation(i, total, seed, iterations);
        }
        
        /* Loop-carried dependency with volatile */
        seed = total % 100;
    }
    
    return total;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Outer loop to create multiple scheduling regions */
    for (int outer = 0; outer < 100; outer++) {
        /* Mix different computation patterns to engage various scheduler features */
        
        /* 1. Vector computations (triggers target-specific scheduling) */
        if (outer % 4 == 0) {
            __m128 vec_a = _mm_set_ps(seed1 * 0.1f, seed2 * 0.2f, seed3 * 0.3f, outer * 0.4f);
            __m128 vec_b = _mm_set_ps(seed2 * 0.5f, seed3 * 0.6f, seed1 * 0.7f, outer * 0.8f);
            __m128 vec_c = _mm_set_ps(seed3 * 0.9f, seed1 * 1.0f, seed2 * 1.1f, outer * 1.2f);
            
            __m128 vec_result = vector_computation(vec_a, vec_b, vec_c);
            
            float result_arr[4];
            _mm_store_ps(result_arr, vec_result);
            total += (int)(result_arr[0] + result_arr[1] + result_arr[2] + result_arr[3]);
        }
        
        /* 2. Branching with unpredictable control flow */
        if (outer % 3 == 0) {
            total += branching_computation(seed1 + outer, 5 + (outer % 10));
        }
        
        /* 3. Dense arithmetic block (fills instruction queue) */
        if (outer % 5 == 0) {
            total += dense_arithmetic(
                seed1 + outer, 
                seed2 - outer, 
                seed3 * (outer + 1), 
                total % 256,
                (seed1 ^ seed2) + outer
            );
        }
        
        /* 4. Assembly barrier computations (forces state restoration) */
        if (outer % 7 == 0) {
            total += asm_barrier_computation(
                seed1 % 100,
                seed2 % 200,
                total % 300,
                outer % 400
            );
        }
        
        /* 5. Mixed pure computations with volatile dependencies */
        for (int inner = 0; inner < 3; inner++) {
            volatile int temp = seed3 + outer + inner;
            total += pure_helper(total, temp, seed1);
            total += pure_helper(temp, total, seed2);
            
            /* Small memory operation to create dependencies */
            volatile int mem_var = total;
            total = mem_var + 1;
        }
        
        /* Update volatile seeds to create changing dependencies */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7fffffff;
        seed3 = (seed3 * 214013 + 2531011) & 0x7fffffff;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total % 256;
}
