/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -o test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, short c, float d, double e, 
                 int *p1, int *p2, float *p3, double *p4, char *p5) {
    /* Use all parameters to prevent dead argument elimination */
    global_volatile = a + b + c + (int)d + (int)e;
    if (p1) global_volatile += *p1;
    if (p2) global_volatile += *p2;
    if (p3) global_volatile += (int)*p3;
    if (p4) global_volatile += (int)*p4;
    if (p5) global_volatile += *p5;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    float arr4[1024];
    double arr5[1024];
    char arr6[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = (float)(i * 1.5f);
        arr5[i] = (double)(i * 2.5);
        arr6[i] = (char)(i & 0x7F);
    }
    
    /* Many scalar variables to increase register pressure */
    int scalar1 = 12345;
    int scalar2 = 67890;
    short scalar3 = 1234;
    float scalar4 = 56.78f;
    double scalar5 = 90.12;
    int scalar6 = 34567;
    int scalar7 = 89101;
    short scalar8 = 2345;
    float scalar9 = 67.89f;
    double scalar10 = 12.34;
    int scalar11 = 45678;
    int scalar12 = 90123;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        short *ptr3 = &arr3[i % 512];            /* Different type, different index */
        float *ptr4 = &arr4[(i * 3) % 1024];     /* Non-trivial index computation */
        double *ptr5 = &arr5[(i + 7) % 1024];    /* Offset index */
        char *ptr6 = &arr6[i ^ 0x55];            /* XOR index computation */
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication - cheap to recompute */
        int offset2 = (i << 2) + 16;             /* Shift and add - cheap to recompute */
        float scale_factor = (float)i * 0.25f;   /* Float multiplication */
        double dbl_offset = (double)i * 0.125;   /* Double multiplication */
        
        /* Conditional to prevent hoisting of computations */
        if (i % 7 == 0) {
            ptr1 = &arr1[(i + 1) % 1024];
            offset1 = (i + 1) * sizeof(int);
        }
        if (i % 13 == 0) {
            ptr2 = &arr2[(i + 2) % 1024];
            offset2 = ((i + 2) << 2) + 16;
        }
        
        /* Function call that clobbers caller-saved registers */
        helper_func(scalar1 + i, scalar2 - i, scalar3 + (short)i,
                   scalar4 + (float)i, scalar5 + (double)i,
                   ptr1, ptr2, ptr4, ptr5, ptr6);
        
        /* Use computed values after the call (must be kept live or rematerialized) */
        arr1[i] = *ptr1 + offset1;
        arr2[i & 0x3F] = *ptr2 + offset2;
        arr3[i % 512] = (short)(*ptr3 + (int)scale_factor);
        arr4[(i * 3) % 1024] = *ptr4 + scale_factor;
        arr5[(i + 7) % 1024] = *ptr5 + dbl_offset;
        arr6[i ^ 0x55] = (char)(*ptr6 + (i & 0x0F));
        
        /* Update scalars to prevent them from becoming loop invariants */
        scalar1 += arr1[i] & 0x1F;
        scalar2 -= arr2[i & 0x3F] & 0x0F;
        scalar3 += arr3[i % 512] & 0x07;
        scalar4 += (float)(arr4[(i * 3) % 1024] * 0.01f);
        scalar5 += arr5[(i + 7) % 1024] * 0.001;
        scalar6 = scalar6 * 1103515245 + 12345;
        scalar7 = scalar7 * 1664525 + 1013904223;
        scalar8 = (short)(scalar8 * 214013 + 2531011);
        scalar9 = scalar9 * 1.1f + 0.5f;
        scalar10 = scalar10 * 1.01 + 0.1;
        scalar11 = scalar11 ^ (scalar11 << 13);
        scalar12 = scalar12 ^ (scalar12 >> 17);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += arr3[i];
        checksum += (unsigned long long)arr4[i];
        checksum += (unsigned long long)arr5[i];
        checksum += arr6[i];
    }
    
    checksum += scalar1 + scalar2 + scalar3 + (int)scalar4 + 
                (int)scalar5 + scalar6 + scalar7 + scalar8 + 
                (int)scalar9 + (int)scalar10 + scalar11 + scalar12;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
