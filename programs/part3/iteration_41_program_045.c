/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions that can't be constant folded */
static volatile int global_seed = 42;

/* High register pressure block with memory clobber */
#define HIGH_REGISTER_PRESSURE_BLOCK(result_var) do { \
    asm volatile("" ::: "memory"); \
    int t1 = (result_var) * 3; \
    int t2 = t1 + 7; \
    int t3 = t2 - 11; \
    int t4 = t3 * 2; \
    int t5 = t4 / 3; \
    long t6 = t5 * 5L; \
    long t7 = t6 + 13L; \
    long t8 = t7 - 17L; \
    long t9 = t8 * 19L; \
    float t10 = (float)t9 * 1.5f; \
    float t11 = t10 + 2.3f; \
    float t12 = t11 - 4.7f; \
    double t13 = (double)t12 * 3.14159; \
    double t14 = t13 + 2.71828; \
    double t15 = t14 - 1.41421; \
    result_var += (int)t15 + t1 + t5 + (int)t9; \
    asm volatile("" ::: "memory"); \
} while(0)

/* Vector operations if available */
#ifdef __SSE2__
#define VECTOR_PRESSURE_BLOCK(result_var) do { \
    typedef int v4si __attribute__((vector_size(16))); \
    v4si v1 = {result_var, result_var+1, result_var+2, result_var+3}; \
    v4si v2 = {5, 6, 7, 8}; \
    v4si v3 = {9, 10, 11, 12}; \
    v4si v4 = v1 + v2; \
    v4si v5 = v3 - v2; \
    v4si v6 = v4 * v5; \
    result_var += v6[0] + v6[1] + v6[2] + v6[3]; \
} while(0)
#else
#define VECTOR_PRESSURE_BLOCK(result_var) do { \
    int v1 = result_var + 100; \
    int v2 = v1 * 2; \
    int v3 = v2 - 50; \
    int v4 = v3 / 3; \
    result_var += v4; \
} while(0)
#endif

/* Test function with rematerialization candidates */
static volatile int __attribute__((noinline)) 
test_remat(volatile int arg1, volatile int arg2, volatile int arg3) {
    volatile int result = 0;
    int local_array[100];
    
    /* Initialize local array */
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of candidates */
    volatile int loop_cond = 1;
    for (int iter = 0; iter < 10 && loop_cond; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* These are cheap recomputable expressions */
        int cand1 = arg1 + 10;           /* Simple arithmetic */
        int cand2 = arg2 * 2;            /* Another recomputable */
        int *cand3 = &local_array[arg3]; /* Address calculation */
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += cand2;
        result += *cand3;
        
        /* Control flow to split live ranges */
        volatile int branch_cond = global_seed > 0; /* Always true but opaque */
        
        if (branch_cond) {
            /* BLOCK B: High register pressure region */
            /* This should cause register pressure and reconsider rematerialization */
            
            /* First memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Many independent operations consuming registers */
            int r1 = result * 2;
            int r2 = r1 + arg1;
            int r3 = r2 - arg2;
            int r4 = r3 * 3;
            int r5 = r4 / 2;
            long r6 = r5 * 7L;
            long r7 = r6 + 11L;
            long r8 = r7 - 13L;
            long r9 = r8 * 17L;
            float r10 = (float)r9 * 0.5f;
            float r11 = r10 + 1.5f;
            float r12 = r11 - 2.5f;
            double r13 = (double)r12 * 1.618;
            double r14 = r13 + 3.141;
            double r15 = r14 - 1.414;
            
            /* More operations to increase pressure */
            int s1 = r1 + r5;
            int s2 = s1 * 2;
            int s3 = s2 - (int)r9;
            int s4 = s3 + (int)r15;
            
            /* Vector operations for additional pressure */
            VECTOR_PRESSURE_BLOCK(s4);
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent elimination */
            result += r1 + r5 + s1 + s2 + s3 + s4;
            result += (int)r6 + (int)r7 + (int)r8;
            result += (int)r10 + (int)r11 + (int)r12;
            result += (int)r13 + (int)r14 + (int)r15;
            
            /* BLOCK C: Use candidates again after high pressure */
            /* This should trigger filter_old_remats */
            result += cand1 * 2;
            result += cand2 / 2;
            result += *cand3 + 5;
            
            /* Additional high pressure block */
            HIGH_REGISTER_PRESSURE_BLOCK(result);
            
            /* Final use of candidates */
            result += cand1 + cand2 + *cand3;
        } else {
            /* Unreachable but needed for control flow */
            result += 999;
        }
        
        /* Modify arguments to prevent loop invariant removal */
        arg1 += iter;
        arg2 -= iter;
        arg3 = (arg3 + 1) % 50;
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1;
    }
    
    volatile int final_result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Use volatile arguments to prevent constant propagation */
        volatile int arg1 = global_seed + i;
        volatile int arg2 = global_seed * 2 - i;
        volatile int arg3 = (global_seed + i * 3) % 50;
        
        final_result += test_remat(arg1, arg2, arg3);
        
        /* Modify global seed to change recomputable expressions */
        global_seed += 1;
    }
    
    printf("Result: %d\n", final_result);
    
    /* Use result to prevent dead code elimination */
    if (final_result == 0x12345678) {
        printf("Impossible!\n");
    }
    
    return 0;
}
