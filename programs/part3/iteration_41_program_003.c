/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -fdump-rtl-early-remat test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions by using volatile arguments */
static int __attribute__((noinline)) 
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    int local_array[256];
    
    /* Initialize local array with volatile pattern */
    for (int i = 0; i < 256; i++) {
        local_array[i] = i ^ arg1;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    volatile int loop_cond = 1;
    int loop_count = arg4 > 0 ? arg4 : 100;
    
    for (int iter = 0; iter < loop_count; iter++) {
        /* BLOCK A: Create rematerialization candidates with simple recomputable values */
        int cand1 = arg1 + 10;           /* Simple constant expression */
        int cand2 = arg2 * 2;            /* Cheap arithmetic */
        int cand3 = arg3 & 0xFF;         /* Mask operation */
        
        /* Address calculations - strong rematerialization candidates */
        int *cand4 = &local_array[arg1 + 5];  /* Stack address with constant offset */
        int *cand5 = &local_array[arg2 * 3];  /* Another address calculation */
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1 + cand2;
        result += *cand4 + *cand5;
        result += cand3;
        
        /* Control flow to split live ranges */
        volatile int branch_cond = 1;  /* Always true but opaque to compiler */
        
        if (branch_cond) {
            /* BLOCK B: High register pressure region */
            /* Many independent variables to consume registers */
            int t1 = result ^ arg1;
            int t2 = result + arg2;
            int t3 = result * arg3;
            int t4 = t1 + t2;
            int t5 = t2 - t3;
            int t6 = t3 * t4;
            int t7 = t4 / (arg1 + 1);
            int t8 = t5 ^ t6;
            int t9 = t6 + t7;
            int t10 = t7 - t8;
            int t11 = t8 * t9;
            int t12 = t9 + t10;
            int t13 = t10 ^ t11;
            int t14 = t11 - t12;
            int t15 = t12 * t13;
            
            /* Floating point variables for additional pressure */
            double f1 = t1 * 0.5;
            double f2 = t2 * 0.25;
            double f3 = t3 * 0.125;
            double f4 = f1 + f2;
            double f5 = f2 - f3;
            double f6 = f3 * f4;
            float f7 = t4 * 0.1f;
            float f8 = t5 * 0.2f;
            float f9 = f7 + f8;
            float f10 = f8 - f7;
            
            /* Long variables */
            long l1 = t6 * 1000L;
            long l2 = t7 * 2000L;
            long l3 = l1 + l2;
            long l4 = l2 - l1;
            long l5 = l3 * l4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6, t7, t8};
            v4si v3 = {t9, t10, t11, t12};
            v4si v4 = v1 + v2;
            v4si v5 = v2 - v3;
            v4si v6 = v4 * v5;
            
            /* Use vector results */
            int vsum = v6[0] + v6[1] + v6[2] + v6[3];
            result += vsum;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent optimization */
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            result += t11 + t12 + t13 + t14 + t15;
            result += (int)(f1 + f2 + f3 + f4 + f5 + f6);
            result += (int)(f7 + f8 + f9 + f10);
            result += (int)(l1 + l2 + l3 + l4 + l5);
        }
        
        /* BLOCK C: Use candidate values again after high-pressure region */
        /* This forces compiler to either rematerialize or replace old candidates */
        result += cand1 * 2;      /* Different use pattern */
        result += cand2 - 5;
        result += cand3 | 0x80;
        result += *cand4 >> 2;
        result += *cand5 << 1;
        
        /* Additional computation to ensure values are live across blocks */
        volatile int dummy = 0;
        if (dummy) {
            /* Never executed but creates additional control flow */
            result += cand1 + cand2 + cand3;
        }
    }
    
    return result;
}

/* Wrapper to create more optimization context */
static int __attribute__((noinline))
test_wrapper(volatile int a, volatile int b, volatile int c, volatile int d) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += test_remat(a + i, b - i, c ^ i, d);
    }
    return sum;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int arg1 = 42;
    volatile int arg2 = 123;
    volatile int arg3 = 255;
    volatile int arg4 = iterations;
    
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += test_wrapper(arg1 + i, arg2 - i, arg3 ^ i, arg4);
    }
    
    printf("Result: %d\n", total);
    
    /* Also test with LTO-friendly compilation */
    #ifdef __LTO_ENABLED__
    total += test_remat(arg1, arg2, arg3, arg4);
    printf("LTO result: %d\n", total);
    #endif
    
    return 0;
}
