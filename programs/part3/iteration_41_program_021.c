/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO: gcc -O2 -fearly-remat -flto -ffat-lto-objects -fno-omit-frame-pointer -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg_store;

/* Function to create rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local array for address calculations */
    int local_array[256];
    volatile int result = 0;
    
    /* Many local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < arg4; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile argument */
        int cand1 = arg1 + 10;  /* arg1 + constant */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2;   /* arg2 * constant */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 5];  /* &array[arg + const] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * 3) + (arg2 / 2);
        
        /* Immediate use of candidates in Block A */
        temp1 = cand1 * 2;
        temp2 = cand2 + cand1;
        temp3 = *cand3 + cand4;
        result += temp1 + temp2 + temp3;
        
        /* Control flow: conditional jump to Block B */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Dense sequence of independent arithmetic operations */
            temp1 = arg1 * arg2;
            temp2 = arg2 * arg3;
            temp3 = arg3 * arg4;
            temp4 = arg4 * arg1;
            temp5 = temp1 + temp2;
            temp6 = temp3 + temp4;
            temp7 = temp5 * temp6;
            temp8 = temp7 / (arg1 + 1);
            temp9 = temp8 << 2;
            temp10 = temp9 >> 1;
            temp11 = temp10 | 0xFF;
            temp12 = temp11 & 0x0F;
            temp13 = temp12 ^ 0x55;
            temp14 = ~temp13;
            temp15 = temp14 % 17;
            
            /* Long operations */
            ltemp1 = (long)arg1 * arg2;
            ltemp2 = (long)arg2 * arg3;
            ltemp3 = (long)arg3 * arg4;
            ltemp4 = ltemp1 + ltemp2;
            ltemp5 = ltemp3 * ltemp4;
            
            /* Float operations */
            ftemp1 = (float)arg1 * 1.5f;
            ftemp2 = (float)arg2 * 2.5f;
            ftemp3 = ftemp1 + ftemp2;
            ftemp4 = ftemp3 * 3.14f;
            ftemp5 = ftemp4 / 2.0f;
            
            /* Double operations */
            dtemp1 = (double)arg3 * 1.234;
            dtemp2 = (double)arg4 * 2.345;
            dtemp3 = dtemp1 + dtemp2;
            dtemp4 = dtemp3 * 3.456;
            dtemp5 = dtemp4 / 4.567;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1, v2, v3, v4, v5;
            v1 = (v4si){arg1, arg2, arg3, arg4};
            v2 = (v4si){arg2, arg3, arg4, arg1};
            v3 = v1 + v2;
            v4 = v1 * v2;
            v5 = v3 + v4;
            
            /* Use vector results */
            int vtemp[4];
            memcpy(vtemp, &v5, sizeof(v5));
            temp15 += vtemp[0] + vtemp[1] + vtemp[2] + vtemp[3];
            #endif
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This forces compiler to either rematerialize or replace */
            temp1 = cand1 + cand2;      /* Use cand1 and cand2 */
            temp2 = *cand3 - cand4;     /* Use cand3 and cand4 */
            temp3 = cand1 * cand4;      /* Use cand1 and cand4 */
            temp4 = cand2 + *cand3;     /* Use cand2 and cand3 */
            
            /* Mix with high-pressure temporaries */
            result += temp1 + temp2 + temp3 + temp4 + temp15;
            result += (int)ltemp5 + (int)ftemp5 + (int)dtemp5;
        }
        
        /* Alternate path to create more control flow complexity */
        if (iter % 2) {
            /* Use candidates in different context */
            temp1 = cand1 - cand2;
            temp2 = *cand3 * 2;
            result += temp1 + temp2;
        }
    }
    
    return result;
}

/* Main function with loop to ensure hot path */
int main(int argc, char **argv)
{
    volatile int iterations = 100;
    volatile int result = 0;
    
    /* Read iterations from argv if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    /* Create volatile arguments */
    volatile int arg1 = 42;
    volatile int arg2 = 17;
    volatile int arg3 = 8;
    volatile int arg4 = iterations;
    
    /* Call test function in loop */
    for (volatile int i = 0; i < 10; i++) {
        result += test_remat(arg1 + i, arg2 + i, arg3 + i, arg4);
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", result);
    
    /* Store to volatile to ensure all computations complete */
    vol_arg_store = result;
    
    return 0;
}
