/* early-remat-test.c
 * Designed to trigger uncovered lines 930-937 in early-remat.cc
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_registers(int a, int b, short c, float d, double e, 
                   int *f, int *g, char h, long i, float j, 
                   double k, int *l) {
    /* Complex enough to use many registers */
    global_sink = a + b + c + (int)d + (int)e + *f + *g + h + (int)i + (int)j + (int)k + *l;
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024], arr2[1024], arr3[1024], arr4[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        arr4[i] = i * 3;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
        sarr1[i] = i & 0x7FFF;
        carr1[i] = i & 0xFF;
    }
    
    int result = 0;
    
    /* High register pressure loop with many live variables */
    for (int i = 0; i < 1024; i++) {
        /* Declare many scalar variables with different types */
        int scalar1 = i * 2;          /* Candidate for remat: i*2 is cheap */
        int scalar2 = i * 3;          /* Another cheap computation */
        short scalar3 = i & 0x7F;     /* Different mode: SImode vs HImode */
        float scalar4 = i * 1.25f;    /* SFmode */
        double scalar5 = i * 2.71828; /* DFmode */
        long scalar6 = i * 5L;        /* DImode on 64-bit */
        char scalar7 = i & 0x3F;      /* QImode */
        
        /* Compute addresses - these are cheap to rematerialize */
        int *ptr1 = &arr1[i];         /* Address computation */
        int *ptr2 = &arr2[i & 0x3F];  /* More complex address */
        int *ptr3 = &arr3[scalar1 & 0x1FF]; /* Using computed index */
        float *ptr4 = &farr1[i];
        double *ptr5 = &darr1[i];
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            scalar1 += 1;
            scalar4 += 1.0f;
        }
        
        if (i % 13 == 0) {
            scalar2 *= 2;
            scalar5 *= 1.1;
        }
        
        /* Call function that clobbers caller-saved registers
         * This forces the compiler to save/restore or rematerialize
         * values used after the call */
        use_registers(scalar1, scalar2, scalar3, scalar4, scalar5,
                      ptr1, ptr2, scalar7, scalar6, scalar4 * 2.0f,
                      scalar5 * 0.5, ptr3);
        
        /* Use computed values after call - they need to be live across call */
        arr1[i] = scalar1 + *ptr1;
        arr2[i & 0x3F] = scalar2 + *ptr2;
        arr3[scalar1 & 0x1FF] = scalar1 * scalar2;
        farr1[i] = scalar4 + *ptr4;
        darr1[i] = scalar5 + *ptr5;
        
        /* More computations using the same values */
        if (i % 3 == 0) {
            /* Recompute addresses - might trigger remat */
            int *ptr6 = &arr4[i];  /* Another address computation */
            *ptr6 = scalar1 - scalar2;
            
            /* Use mixed modes */
            sarr1[i] = (short)(scalar1 + scalar2);
            carr1[i] = (char)(scalar1 & 0xFF);
        }
        
        /* Accumulate result to prevent dead code elimination */
        result += arr1[i] + arr2[i & 0x3F] + arr3[scalar1 & 0x1FF];
    }
    
    /* Final computation to use all arrays */
    for (int i = 0; i < 1024; i++) {
        result += arr4[i] + (int)farr1[i] + (int)darr1[i] + sarr1[i] + carr1[i];
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
