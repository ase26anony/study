/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early_remat pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline))
void helper_func(int a, int b, int c, float d, double e, 
                 int *p1, int *p2, short *p3, float *p4, double *p5) {
    /* Perform operations that use all arguments to prevent optimization */
    global_volatile = a + b + c;
    *p1 = (*p1) * 2;
    *p2 = (*p2) + 1;
    *p3 = (short)(*p3 + 1);
    *p4 = *p4 + d;
    *p5 = *p5 + e;
    
    /* Additional computation to increase register pressure in caller */
    global_volatile += (int)(d * 100.0f);
    global_volatile += (int)(e * 1000.0);
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    float arr4[1024];
    double arr5[1024];
    int arr6[1024];
    int arr7[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i % 256);
        arr4[i] = (float)i * 0.5f;
        arr5[i] = (double)i * 0.25;
        arr6[i] = i * 2;
        arr7[i] = i * 3;
    }
    
    /* Many scalar variables to increase register pressure */
    int scalar1 = 1;
    int scalar2 = 2;
    int scalar3 = 3;
    float scalar4 = 4.0f;
    double scalar5 = 5.0;
    short scalar6 = 6;
    int scalar7 = 7;
    int scalar8 = 8;
    float scalar9 = 9.0f;
    double scalar10 = 10.0;
    int scalar11 = 11;
    int scalar12 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Conditional to prevent hoisting of computations */
        if (i % 7 == 0) {
            scalar1 += 1;
            scalar4 += 0.5f;
        }
        
        /* Compute multiple address expressions - potential remat candidates */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        short *ptr3 = &arr3[(i * 3) & 0x3FF];    /* More complex index */
        float *ptr4 = &arr4[i % 512];            /* Modulo operation */
        double *ptr5 = &arr5[i];                 /* Simple index */
        
        /* Additional pointer computations with different modes */
        int *ptr6 = &arr6[i] + scalar1;          /* Base + index + scalar */
        int *ptr7 = &arr7[i] + scalar2;          /* Another variant */
        
        /* Scalar computations that are cheap to recompute */
        int offset1 = i * 4;                     /* Multiplication - cheap */
        int offset2 = i << 2;                    /* Shift alternative */
        float foffset = (float)i * 2.0f;         /* Float computation */
        double doffset = (double)i * 1.5;        /* Double computation */
        
        /* Use all scalars in computations to keep them live */
        scalar2 = scalar1 + offset1;
        scalar3 = scalar2 * 2;
        scalar6 = (short)(scalar3 % 256);
        scalar7 = scalar8 + i;
        scalar9 = scalar4 + foffset;
        scalar10 = scalar5 + doffset;
        scalar11 = scalar12 * scalar1;
        
        /* Call helper function - clobbers caller-saved registers */
        helper_func(scalar1, scalar2, scalar3, scalar4, scalar5,
                   ptr1, ptr2, ptr3, ptr4, ptr5);
        
        /* Use results after call - forces save/restore or rematerialization */
        *ptr6 = scalar7 + *ptr1;
        *ptr7 = scalar8 + *ptr2;
        
        /* More computations using the pointers and scalars */
        arr1[i] = *ptr1 + scalar11;
        arr2[i & 0x3F] = *ptr2 + scalar12;
        
        /* Update scalars in loop to prevent optimization */
        scalar1 = (scalar1 + 1) & 0xFF;
        scalar2 = (scalar2 * 3) & 0xFF;
        scalar3 = (scalar3 + i) & 0xFF;
        scalar4 = scalar4 + 0.1f;
        scalar5 = scalar5 + 0.01;
        scalar8 = (scalar8 * 2) & 0xFF;
        scalar12 = (scalar12 + 1) & 0xFF;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)arr4[i] + (int)arr5[i];
        checksum += arr6[i] + arr7[i];
    }
    
    /* Also include scalars in checksum */
    checksum += scalar1 + scalar2 + scalar3 + (int)scalar4 + (int)scalar5;
    checksum += scalar6 + scalar7 + scalar8 + (int)scalar9 + (int)scalar10;
    checksum += scalar11 + scalar12 + global_volatile;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
