/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO: gcc -O2 -fearly-remat -flto -ffat-lto-objects -fno-omit-frame-pointer -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force values to be recomputable but not constant-folded */
static int volatile global_seed = 42;

/* Vector types for register pressure */
#ifdef __SSE2__
#include <xmmintrin.h>
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static int volatile __attribute__((noinline)) 
test_remat(int volatile arg1, int volatile arg2, int volatile arg3) 
{
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i + global_seed;
    }
    
    /* Volatile result to prevent optimization */
    int volatile result = 0;
    
    /* Loop to create multiple uses */
    int volatile loop_counter = 10;
    if (arg1 < 0) loop_counter = 1; /* Never taken but opaque to compiler */
    
    for (int iter = 0; iter < loop_counter; iter++) {
        /* --- BLOCK A: Create rematerialization candidates --- */
        /* Candidate 1: Simple arithmetic on volatile argument */
        int cand1 = arg1 + 5;  /* arg1 + 5 is cheap to recompute */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[arg2 + 3];  /* &array[arg2+3] is recomputable */
        
        /* Candidate 3: Another arithmetic expression */
        int cand3 = arg3 * 2 + 7;
        
        /* Immediate use of candidates in Block A */
        result += cand1;
        result += *cand2;
        result += cand3;
        
        /* --- Control flow to split live ranges --- */
        /* Volatile condition ensures both paths exist in CFG */
        int volatile take_path = global_seed > 0;  /* Always true but opaque */
        
        if (take_path) {
            /* --- BLOCK B: High register pressure region --- */
            /* Many distinct variables to consume registers */
            int t1 = result + arg1;
            int t2 = t1 * arg2;
            int t3 = t2 - arg3;
            int t4 = t3 ^ arg1;
            int t5 = t4 | arg2;
            int t6 = t5 & arg3;
            long t7 = t6 * 1234567L;
            long t8 = t7 - 987654L;
            long t9 = t8 ^ 0xABCDEF;
            float f1 = t9 * 0.5f;
            float f2 = f1 + 3.14f;
            float f3 = f2 * 2.0f;
            double d1 = f3 * 1.41421356;
            double d2 = d1 / 3.14159265;
            double d3 = d2 + 2.71828182;
            
            /* More variables to increase pressure */
            int t10 = t6 + t1;
            int t11 = t10 * t2;
            int t12 = t11 - t3;
            int t13 = t12 ^ t4;
            int t14 = t13 | t5;
            int t15 = t14 & t6;
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Vector operations if available */
#ifdef __SSE2__
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6, t10, t11};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            v4sf vf1 = {f1, f2, f3, (float)t1};
            v4sf vf2 = {2.0f, 3.0f, 4.0f, 5.0f};
            v4sf vf3 = vf1 * vf2;
            
            /* Use vector results */
            int *vp = (int*)&v5;
            result += vp[0] + vp[1] + vp[2] + vp[3];
#endif
            
            /* Use all temporaries to keep them live */
            result += t1 + t2 + t3 + t4 + t5 + t6;
            result += (int)(t7 ^ t8 ^ t9);
            result += (int)(f1 + f2 + f3);
            result += (int)(d1 + d2 + d3);
            result += t10 + t11 + t12 + t13 + t14 + t15;
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
        }
        
        /* --- BLOCK C: Use candidates again after high pressure --- */
        /* This forces compiler to either rematerialize or replace old candidates */
        result += cand1 * 2;
        result += *cand2 + 1;
        result += cand3 - 3;
        
        /* Additional use with different computation */
        int cand1_alt = arg1 + 5;  /* Same recomputation */
        int *cand2_alt = &local_array[arg2 + 3];
        int cand3_alt = arg3 * 2 + 7;
        
        result += cand1_alt + *cand2_alt + cand3_alt;
    }
    
    return result;
}

/* Main function with loop to increase optimization opportunities */
int main(int argc, char **argv) 
{
    int volatile iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    int volatile total = 0;
    
    /* Vary arguments slightly each iteration */
    for (int i = 0; i < iterations; i++) {
        int volatile arg1 = global_seed + i;
        int volatile arg2 = global_seed - i * 2;
        int volatile arg3 = global_seed + i * 3;
        
        total += test_remat(arg1, arg2, arg3);
        
        /* Modify global seed to change recomputable values */
        global_seed ^= i;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
