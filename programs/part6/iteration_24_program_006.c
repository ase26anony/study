/* test_sched_context.c - Comprehensive test for Haifa scheduler context management */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */
#include <emmintrin.h>  /* For SSE2 intrinsics */

/* Helper function with pure computation - creates scheduling opportunities */
static int helper_pure(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b) + (b & c);
}

/* Helper with memory operations - creates load/store dependencies */
static int helper_memops(volatile int* arr, int idx1, int idx2, int idx3) {
    int t1 = arr[idx1];
    int t2 = arr[idx2];
    arr[idx3] = t1 + t2;
    return arr[idx3] * 2;
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = e << 3;
    int t5 = t3 & t4;
    int t6 = t2 - t1;
    int t7 = t5 | t6;
    int t8 = t4 + t7;
    int t9 = t8 * t3;
    int t10 = t9 / (t1 + 1);
    int t11 = t10 ^ t7;
    int t12 = t11 << 2;
    int t13 = t12 - t5;
    int t14 = t13 & t8;
    int t15 = t14 | t9;
    int t16 = t15 * t10;
    int t17 = t16 + t11;
    int t18 = t17 - t12;
    int t19 = t18 ^ t13;
    int t20 = t19 & t14;
    return t20;
}

/* Function with SSE/MMX intrinsics - triggers target-specific scheduling */
static float sse_operations(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec3 = _mm_add_ps(vec1, vec2);
    __m128 vec4 = _mm_mul_ps(vec1, vec2);
    __m128 vec5 = _mm_sub_ps(vec3, vec4);
    
    /* Mix with integer SSE operations */
    __m128i ivec1 = _mm_set1_epi32((int)a);
    __m128i ivec2 = _mm_set1_epi32((int)b);
    __m128i ivec3 = _mm_add_epi32(ivec1, ivec2);
    
    float result[4];
    _mm_store_ps(result, vec5);
    return result[0] + result[1] + result[2] + result[3];
}

/* Function with inline assembly barriers - causes scheduler backtracking */
static int asm_barrier_ops(int a, int b, int c) {
    int r1, r2, r3;
    
    /* First computation with barrier */
    asm volatile ("addl %1, %0" : "=r"(r1) : "r"(a), "0"(b));
    
    /* Memory clobber barrier - scheduler may try to move across this */
    asm volatile ("" : : : "memory");
    
    /* Dependent computation */
    asm volatile ("imull %1, %0" : "=r"(r2) : "r"(c), "0"(r1));
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Final computation */
    asm volatile ("xorl %1, %0" : "=r"(r3) : "r"(r2), "0"(a));
    
    return r3;
}

/* Function with unpredictable branching - creates control flow for speculative scheduling */
static int branching_pattern(volatile int cond, int x, int y, int z) {
    int result = 0;
    
    for (int i = 0; i < (cond & 0x3F); ++i) {
        if (i % 3 == 0) {
            result += x * y;
        } else if (i % 3 == 1) {
            result += y ^ z;
        } else {
            result += x | z;
        }
        
        /* Volatile read to prevent optimization */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    return result;
}

/* Complex function mixing multiple patterns */
static int mixed_scheduling_pattern(volatile int seed, int iter) {
    int total = 0;
    volatile int arr[64];
    
    /* Initialize array with volatile writes */
    for (int i = 0; i < 64; ++i) {
        arr[i] = (seed + i) ^ 0x12345678;
    }
    
    /* Loop with varying trip count - encourages context saves */
    for (int i = 0; i < (iter & 0x1F) + 5; ++i) {
        /* Mix different computation patterns */
        total += helper_pure(arr[i], arr[i+1], arr[i+2]);
        
        if (i % 4 == 0) {
            total += helper_memops(arr, i, i+1, i+2);
        }
        
        /* Dense arithmetic sequence */
        total += dense_arithmetic(arr[i], arr[i+1], arr[i+2], arr[i+3], arr[i+4]);
        
        /* SSE operations every 8 iterations */
        if (i % 8 == 0) {
            float fval = sse_operations(
                (float)arr[i] / 1000.0f,
                (float)arr[i+1] / 1000.0f,
                (float)arr[i+2] / 1000.0f,
                (float)arr[i+3] / 1000.0f
            );
            total += (int)fval;
        }
        
        /* Assembly barriers */
        total += asm_barrier_ops(arr[i], arr[i+1], arr[i+2]);
        
        /* Unpredictable branching */
        total += branching_pattern(seed + i, arr[i], arr[i+1], arr[i+2]);
    }
    
    return total;
}

/* Main driver with multiple phases of scheduling */
int main(int argc, char** argv) {
    volatile int seed1, seed2, seed3;
    int total = 0;
    
    /* Initialize seeds from argv to prevent constant propagation */
    seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    
    /* Multiple iterations to increase chance of context allocation/freeing */
    for (int phase = 0; phase < 100; ++phase) {
        volatile int iter = (seed1 + phase) & 0xFF;
        
        /* Call different patterns to exercise various scheduler behaviors */
        total += mixed_scheduling_pattern(seed1 + phase, iter);
        
        /* Alternate between different computation intensities */
        if (phase % 3 == 0) {
            /* Heavy computation phase */
            for (int j = 0; j < 10; ++j) {
                total += dense_arithmetic(
                    seed1 + j, seed2 + j, seed3 + j,
                    phase + j, iter + j
                );
            }
        } else if (phase % 3 == 1) {
            /* Memory-intensive phase */
            volatile int mem_arr[32];
            for (int j = 0; j < 32; ++j) {
                mem_arr[j] = (seed2 + phase + j) ^ 0xABCDEF;
            }
            for (int j = 0; j < 16; ++j) {
                total += helper_memops(mem_arr, j, j+1, j+2);
            }
        } else {
            /* Vector/SIMD phase */
            for (int j = 0; j < 5; ++j) {
                float fval = sse_operations(
                    (float)(seed3 + j) / 100.0f,
                    (float)(phase + j) / 100.0f,
                    (float)(iter + j) / 100.0f,
                    (float)(total + j) / 100.0f
                );
                total += (int)(fval * 100.0f);
            }
        }
        
        /* Insert occasional barriers */
        if (phase % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total);
    
    return total & 0xFF;
}
