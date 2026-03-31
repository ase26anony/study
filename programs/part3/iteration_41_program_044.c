/* test-early-remat.c
 * Program designed to trigger filter_old_remats logic in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -o test test-early-remat.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_counter = 0;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Function with complex control flow and register pressure */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    /* Local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg4; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* These are cheap recomputable expressions */
        int cand1 = arg1 + 5;              /* Simple constant addition */
        int cand2 = arg2 * 2;              /* Simple multiplication */
        int cand3 = arg3 & 0xFF;           /* Mask operation */
        /* Address calculation with constant offset - strong remat candidate */
        int *cand4 = &local_array[arg1 % 50 + 10];
        
        /* Immediate use of candidates in BLOCK A */
        temp1 = cand1 * cand2;
        temp2 = cand3 + *cand4;
        result += temp1 + temp2;
        
        /* Control flow: conditional jump to BLOCK B */
        /* Use volatile to prevent optimization */
        if (always_true) {
            /* BLOCK B: High register pressure region */
            /* This should cause reconsideration of rematerialization decisions */
            
            /* Many independent arithmetic operations */
            temp1 = arg1 * 3 + iter;
            temp2 = arg2 / 2 - iter;
            temp3 = arg3 << 2;
            temp4 = arg1 ^ arg2;
            temp5 = arg2 | arg3;
            temp6 = arg1 & arg2;
            temp7 = arg3 - arg1;
            temp8 = arg1 + arg2 + arg3;
            temp9 = arg2 * arg3;
            temp10 = arg1 % 7;
            temp11 = temp1 * temp2;
            temp12 = temp3 + temp4;
            temp13 = temp5 ^ temp6;
            temp14 = temp7 << 1;
            temp15 = temp8 >> 2;
            
            /* Floating point operations */
            ftemp1 = (float)arg1 * 1.5f;
            ftemp2 = (float)arg2 / 3.0f;
            ftemp3 = ftemp1 + ftemp2;
            ftemp4 = ftemp1 * ftemp2;
            ftemp5 = ftemp3 - ftemp4;
            
            /* Double precision operations */
            dtemp1 = (double)arg3 * 2.5;
            dtemp2 = (double)arg1 / 1.7;
            dtemp3 = dtemp1 + dtemp2;
            dtemp4 = dtemp1 * dtemp2;
            
            /* Long integer operations */
            ltemp1 = (long)arg1 * 1000L;
            ltemp2 = (long)arg2 * 2000L;
            ltemp3 = ltemp1 + ltemp2;
            ltemp4 = ltemp1 - ltemp2;
            ltemp5 = ltemp3 * ltemp4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            v4si vec1 = {arg1, arg2, arg3, iter};
            v4si vec2 = {5, 10, 15, 20};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {(float)arg1, (float)arg2, (float)arg3, (float)iter};
            v4sf fvec2 = {1.5f, 2.5f, 3.5f, 4.5f};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent elimination */
            result += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
            result += temp9 + temp10 + temp11 + temp12 + temp13 + temp14 + temp15;
            result += (int)ftemp1 + (int)ftemp2 + (int)ftemp3;
            result += (int)dtemp1 + (int)dtemp2;
            result += (int)(ltemp5 % 1000);
            
            #ifdef __SSE2__
            /* Extract and use vector elements */
            int vec_result = vec3[0] + vec3[1] + vec3[2] + vec3[3];
            vec_result += vec4[0] + vec4[1] + vec4[2] + vec4[3];
            result += vec_result;
            #endif
        }
        
        /* BLOCK C: Use candidates again after high-pressure region */
        /* This requires the compiler to either rematerialize or replace */
        temp1 = cand1 + cand2;
        temp2 = cand3 - *cand4;
        result += temp1 * temp2;
        
        /* Additional use with different computation */
        temp3 = cand1 * cand3;
        temp4 = cand2 + *cand4;
        result += temp3 / (temp4 ? temp4 : 1);
        
        /* Force another memory clobber */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Second test function with different pattern */
static volatile int test_remat2(volatile int base) {
    int array[64];
    volatile int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 64; i++) {
        array[i] = i + base;
    }
    
    /* Multiple rematerialization candidates */
    for (volatile int i = 0; i < 32; i++) {
        /* Candidates with different expression types */
        int cand1 = base + i * 4;          /* Linear function */
        int cand2 = array[base % 32 + i];  /* Array access with computation */
        int cand3 = (base << 3) | 0x0F;    /* Bit operations */
        int *cand4 = &array[i + 16];       /* Address calculation */
        
        /* First use */
        sum += cand1 + cand2 + cand3 + *cand4;
        
        /* High pressure block */
        if (always_true) {
            int t1 = base * i;
            int t2 = base + i;
            int t3 = base - i;
            int t4 = base ^ i;
            int t5 = base | i;
            int t6 = base & i;
            int t7 = t1 * t2;
            int t8 = t3 + t4;
            int t9 = t5 ^ t6;
            int t10 = t7 - t8;
            int t11 = t9 * t10;
            int t12 = t1 + t2 + t3 + t4 + t5 + t6;
            
            float f1 = (float)base * 0.5f;
            float f2 = (float)i * 1.5f;
            float f3 = f1 + f2;
            float f4 = f1 * f2;
            
            double d1 = (double)base * 0.25;
            double d2 = (double)i * 0.75;
            double d3 = d1 + d2;
            
            asm volatile("" ::: "memory");
            
            sum += t7 + t8 + t9 + t10 + t11 + t12;
            sum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
            sum += (int)d1 + (int)d2 + (int)d3;
        }
        
        /* Second use of candidates */
        sum += cand1 * cand2 - cand3 + *cand4;
        
        /* Third use with different computation */
        sum += (cand1 << 2) | (cand2 & 0xFF);
    }
    
    return sum;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total_result = 0;
    
    /* Call test functions multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments to prevent constant propagation */
        volatile int arg1 = i % 100 + 1;
        volatile int arg2 = i % 50 + 2;
        volatile int arg3 = i % 25 + 3;
        volatile int arg4 = 10 + (i % 5);
        
        total_result += test_remat(arg1, arg2, arg3, arg4);
        total_result += test_remat2(arg1 + arg2);
        
        /* Modify global to prevent optimization */
        global_counter++;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (global counter: %d)\n", total_result, global_counter);
    
    return total_result != 0 ? 0 : 1;
}
