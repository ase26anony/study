/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions with different scheduling characteristics */

/* Function 1: Mixed integer operations with volatile barriers */
static int mixed_ops_with_barriers(volatile int a, volatile int b, volatile int c) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    /* Initial computation */
    t1 = a + b;
    
    /* Memory barrier to potentially cause scheduler backtracking */
    asm volatile ("" : : : "memory");
    
    t2 = c * t1;
    t3 = t2 ^ a;
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    t4 = b << 2;
    t5 = t3 - t4;
    t6 = t5 | c;
    
    /* Volatile read to prevent optimization */
    volatile int v = a;
    t7 = t6 + v;
    
    t8 = t7 * 3;
    t9 = t8 / (b ? b : 1);
    t10 = t9 ^ t1;
    
    return t10;
}

/* Function 2: Vector operations using SSE intrinsics */
static __m128 vector_ops(volatile float f1, volatile float f2, 
                         volatile float f3, volatile float f4) {
    __m128 v1, v2, v3, v4, result;
    
    /* Create vectors */
    v1 = _mm_set1_ps(f1);
    v2 = _mm_set1_ps(f2);
    v3 = _mm_set1_ps(f3);
    v4 = _mm_set1_ps(f4);
    
    /* Mixed vector operations */
    __m128 t1 = _mm_add_ps(v1, v2);
    __m128 t2 = _mm_mul_ps(v3, v4);
    __m128 t3 = _mm_sub_ps(t1, t2);
    __m128 t4 = _mm_add_ps(t3, v1);
    
    /* More operations to increase instruction density */
    result = _mm_mul_ps(t4, _mm_set1_ps(2.0f));
    result = _mm_add_ps(result, _mm_set1_ps(1.0f));
    
    return result;
}

/* Function 3: Dense independent arithmetic sequence */
static int dense_arithmetic(volatile int base) {
    /* Many independent operations to fill instruction queue */
    int a = base + 1;
    int b = base * 2;
    int c = base ^ 0x55AA55AA;
    int d = base << 3;
    int e = base >> 1;
    int f = a + b;
    int g = c * d;
    int h = e ^ f;
    int i = g - h;
    int j = i | a;
    int k = j & b;
    int l = k + c;
    int m = l * d;
    int n = m ^ e;
    int o = n | f;
    int p = o & g;
    int q = p + h;
    int r = q * i;
    int s = r ^ j;
    int t = s | k;
    int u = t & l;
    int v = u + m;
    int w = v * n;
    int x = w ^ o;
    int y = x | p;
    int z = y & q;
    
    return z;
}

/* Function 4: Loop with unpredictable branching */
static int branching_loop(volatile int limit) {
    int sum = 0;
    
    /* Loop with volatile condition to prevent optimization */
    for (int i = 0; i < (limit & 0xF); ++i) {
        /* Unpredictable branch */
        if (rand() & 1) {
            sum += i * 2;
            
            /* Inline assembly with dependencies */
            int temp;
            asm volatile ("addl %1, %0" 
                         : "=r"(temp) 
                         : "r"(i), "0"(sum)
                         : "cc");
            sum = temp;
        } else {
            sum -= i;
            
            /* Another assembly operation */
            asm volatile ("imull %1, %0" 
                         : "+r"(sum) 
                         : "r"(i)
                         : "cc");
        }
        
        /* Memory operation to introduce dependencies */
        volatile int mem_var = i;
        sum ^= mem_var;
    }
    
    return sum;
}

/* Function 5: Complex scheduling pattern with function calls */
static int complex_pattern(volatile int x, volatile int y) {
    int result = 0;
    
    /* Multiple phases of computation */
    for (int phase = 0; phase < 3; ++phase) {
        /* Different computation based on phase */
        switch (phase) {
            case 0:
                /* Integer operations */
                result = x * y + phase;
                break;
            case 1:
                /* More complex computation */
                result = (result << 2) | (x & y);
                
                /* Artificial scheduling barrier */
                asm volatile ("" : : : "memory");
                
                result ^= (x * phase);
                break;
            case 2:
                /* Final computation with memory operations */
                volatile int temp = y;
                result += temp * result;
                
                /* Force memory dependency */
                asm volatile ("movl %1, %0" 
                             : "=r"(result) 
                             : "m"(temp)
                             : "memory");
                break;
        }
        
        /* Conditional based on volatile to prevent optimization */
        if (x & (1 << phase)) {
            result += 1;
        }
    }
    
    return result;
}

/* Main function that orchestrates all patterns */
int main(int argc, char *argv[]) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile float fseed = argc > 4 ? atof(argv[4]) : 3.14159f;
    
    int total = 0;
    
    /* Multiple iterations to increase chance of scheduler context creation */
    for (int iter = 0; iter < 100; ++iter) {
        /* Call different functions to create varied scheduling contexts */
        
        /* 1. Mixed operations with barriers */
        total += mixed_ops_with_barriers(seed1 + iter, seed2, seed3);
        
        /* 2. Vector operations (trigger target-specific scheduling) */
        __m128 vec_result = vector_ops(fseed + iter, fseed * 2, 
                                      fseed / 2, fseed * iter);
        float vec_float[4];
        _mm_storeu_ps(vec_float, vec_result);
        total += (int)vec_float[0];
        
        /* 3. Dense arithmetic sequence */
        total += dense_arithmetic(seed3 + iter);
        
        /* 4. Branching loop */
        total += branching_loop(seed1 ^ iter);
        
        /* 5. Complex pattern */
        total += complex_pattern(seed2 + iter, seed3 - iter);
        
        /* Modify seeds to create different scheduling patterns each iteration */
        seed1 ^= total;
        seed2 += iter;
        seed3 = (seed3 * 1103515245 + 12345) & 0x7fffffff;
        fseed += 0.1f;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
