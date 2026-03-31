/* early-remat-test.c
 * Designed to trigger uncovered lines 930-937 in early-remat.cc
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int c, float d, double e, 
                 short f, int* ptr1, int* ptr2, float* fptr, double* dptr) {
    /* Complex enough to prevent inlining, but simple enough to keep the test focused */
    global_volatile = a + b + c + (int)d + (int)e + f;
    if (ptr1) *ptr1 = a;
    if (ptr2) *ptr2 = b;
    if (fptr) *fptr = d;
    if (dptr) *dptr = e;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    int arr3[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
        sarr1[i] = i & 0x7FFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int scalar1 = 1;
    int scalar2 = 2;
    int scalar3 = 3;
    int scalar4 = 4;
    int scalar5 = 5;
    int scalar6 = 6;
    int scalar7 = 7;
    int scalar8 = 8;
    int scalar9 = 9;
    int scalar10 = 10;
    float fscalar1 = 11.0f;
    float fscalar2 = 12.0f;
    double dscalar1 = 13.0;
    double dscalar2 = 14.0;
    short sscalar1 = 15;
    short sscalar2 = 16;
    
    /* High register pressure loop with mixed computations */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int* ptr1 = &arr1[i];                    /* Base + index */
        int* ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        int* ptr3 = &arr3[i * 2];                /* Base + scaled index */
        float* fptr1 = &farr1[i];                /* Float pointer */
        float* fptr2 = &farr2[i & 0x1F];         /* Float pointer with mask */
        double* dptr1 = &darr1[i];               /* Double pointer */
        short* sptr1 = &sarr1[i];                /* Short pointer */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * 4;                     /* Scaled index - remat candidate */
        int offset2 = i * 8;                     /* Another scaled index */
        float foffset = i * 1.25f;               /* Float computation */
        double doffset = i * 2.71828;            /* Double computation */
        short soffset = i & 0xFF;                /* Masked computation */
        
        /* Conditional to prevent hoisting of computations */
        if (i % 7 == 0) {
            ptr1 = &arr2[i];                     /* Change pointer on some iterations */
            offset1 = i * 3;                     /* Different computation */
        }
        
        if (i % 13 == 0) {
            ptr2 = &arr3[i];                     /* Another conditional change */
            offset2 = i * 5;
        }
        
        /* Use all scalars to keep them live */
        scalar1 += i;
        scalar2 += offset1;
        scalar3 += offset2;
        scalar4 += *ptr1;
        scalar5 += *ptr2;
        scalar6 += *ptr3;
        scalar7 += (int)foffset;
        scalar8 += (int)doffset;
        scalar9 += soffset;
        scalar10 += *sptr1;
        
        fscalar1 += foffset;
        fscalar2 += *fptr1;
        dscalar1 += doffset;
        dscalar2 += *dptr1;
        sscalar1 += soffset;
        sscalar2 += *sptr1;
        
        /* Function call that clobbers caller-saved registers */
        /* Many arguments to increase register pressure across call */
        helper_func(scalar1, scalar2, scalar3, fscalar1, dscalar1,
                   sscalar1, ptr1, ptr2, fptr1, dptr1);
        
        /* Use values after call (forces save/restore or rematerialization) */
        arr1[i] = scalar1 + scalar2;
        arr2[i] = scalar3 + scalar4;
        arr3[i] = scalar5 + scalar6;
        farr1[i] = fscalar1 + fscalar2;
        farr2[i] = fscalar1 - fscalar2;
        darr1[i] = dscalar1 + dscalar2;
        sarr1[i] = sscalar1 + sscalar2;
        
        /* More conditional code to prevent optimization */
        if (i % 17 == 0) {
            /* Recompute some values (encourages rematerialization) */
            int* alt_ptr = &arr1[i * 2];         /* Another address computation */
            arr1[i] += *alt_ptr;
        }
        
        if (i % 19 == 0) {
            /* Use the computed offsets again */
            arr2[i] += offset1 + offset2;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + 
                   (unsigned)farr1[i] + (unsigned)farr2[i] + 
                   (unsigned long long)darr1[i] + sarr1[i];
    }
    
    checksum += scalar1 + scalar2 + scalar3 + scalar4 + scalar5 +
               scalar6 + scalar7 + scalar8 + scalar9 + scalar10 +
               (unsigned)fscalar1 + (unsigned)fscalar2 +
               (unsigned long long)dscalar1 + (unsigned long long)dscalar2 +
               sscalar1 + sscalar2;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
