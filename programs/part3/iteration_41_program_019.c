/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_seed = 12345;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i + global_seed;
    }
    
    /* High register pressure temporaries - declared outside loop for longer live ranges */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long lt1, lt2, lt3, lt4, lt5;
    float ft1, ft2, ft3, ft4, ft5;
    double dt1, dt2, dt3, dt4, dt5;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg1; iter++) {
        /* BLOCK A: Create rematerialization candidates with simple recomputable values */
        /* Candidate 1: Constant derived from function argument */
        int cand1 = arg2 + 5;  /* arg2 + 5 is cheap to recompute */
        
        /* Candidate 2: Another constant expression */
        int cand2 = arg3 * 2;  /* arg3 * 2 is cheap to recompute */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg4];  /* &local_array[arg4] is recomputable */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg2 << 3) | (arg3 & 0xFF);
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += cand2;
        result += *cand3;
        result += cand4;
        
        /* Control flow to split live ranges */
        if (always_true) {  /* Always taken, but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Dense sequence of independent arithmetic operations */
            t1 = arg1 * arg2;
            t2 = arg2 + arg3;
            t3 = arg3 - arg4;
            t4 = arg4 ^ arg1;
            t5 = t1 + t2;
            t6 = t2 - t3;
            t7 = t3 * t4;
            t8 = t4 / (arg1 | 1);
            t9 = t5 ^ t6;
            t10 = t7 & t8;
            
            lt1 = (long)arg1 * arg2 * 1000L;
            lt2 = (long)arg2 * arg3 * 2000L;
            lt3 = (long)arg3 * arg4 * 3000L;
            lt4 = lt1 + lt2 + lt3;
            lt5 = lt4 - (lt1 >> 3);
            
            ft1 = (float)arg1 * 1.1f;
            ft2 = (float)arg2 * 2.2f;
            ft3 = (float)arg3 * 3.3f;
            ft4 = ft1 + ft2 + ft3;
            ft5 = ft4 * 0.5f;
            
            dt1 = (double)arg1 * 1.111;
            dt2 = (double)arg2 * 2.222;
            dt3 = (double)arg3 * 3.333;
            dt4 = dt1 + dt2 + dt3;
            dt5 = dt4 / 4.444;
            
            /* Vector operations to consume more registers if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {arg2, arg3, arg4, arg1};
            v4si v3 = v1 + v2;
            v4si v4 = v1 - v2;
            v4si v5 = v3 * v4;
            
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations to increase pressure */
            t1 = t1 + t10;
            t2 = t2 * t9;
            t3 = t3 - t8;
            t4 = t4 ^ t7;
            t5 = t5 | t6;
            
            lt1 = lt1 + lt5;
            lt2 = lt2 - lt4;
            lt3 = lt3 * lt5;
            
            ft1 = ft1 + ft5;
            ft2 = ft2 - ft4;
            ft3 = ft3 * ft5;
            
            dt1 = dt1 + dt5;
            dt2 = dt2 - dt4;
            dt3 = dt3 * dt5;
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
        }
        
        /* BLOCK C: Use candidates again after high-pressure region */
        /* This forces compiler to either rematerialize or replace old candidates */
        result += cand1 * 2;
        result += cand2 / 2;
        result += *cand3 + 1;
        result += cand4 ^ 0xAA;
        
        /* Use high-pressure temporaries to keep them live */
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        result += (int)(lt1 + lt2 + lt3 + lt4 + lt5);
        result += (int)(ft1 + ft2 + ft3 + ft4 + ft5);
        result += (int)(dt1 + dt2 + dt3 + dt4 + dt5);
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Call test_remat multiple times with different volatile arguments */
    for (int i = 0; i < iterations; i++) {
        global_seed = i * 7 + 11;  /* Change seed each iteration */
        
        /* Use different volatile arguments each time */
        total += test_remat(
            (global_seed >> 0) & 0xF,
            (global_seed >> 4) & 0xF,
            (global_seed >> 8) & 0xF,
            (global_seed >> 12) & 0xF
        );
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    printf("Result: %d\n", total);
    return 0;
}
