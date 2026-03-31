/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int c, float d, double e, 
                 short f, int* ptr1, int* ptr2, long long g, 
                 unsigned char h, float* fptr, double* dptr) {
    /* Force register pressure by using all arguments */
    global_volatile = a + b + c + (int)d + (int)e + f + *ptr1 + *ptr2 + (int)g + h;
    if (fptr) *fptr = d * 2.0f;
    if (dptr) *dptr = e * 3.0;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    unsigned char carr1[1024];
    
    /* Initialize arrays with pattern to prevent optimization */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        farr1[i] = i * 0.5f;
        farr2[i] = (1023 - i) * 0.25f;
        darr1[i] = i * 0.125;
        sarr1[i] = i & 0x7FFF;
        carr1[i] = i & 0xFF;
    }
    
    /* Declare many scalar variables to increase register pressure */
    int scalar1 = 1;
    int scalar2 = 2;
    int scalar3 = 3;
    float fscalar1 = 1.5f;
    float fscalar2 = 2.5f;
    double dscalar1 = 3.14159;
    double dscalar2 = 2.71828;
    short sscalar1 = 100;
    short sscalar2 = 200;
    unsigned char cscalar1 = 50;
    unsigned char cscalar2 = 150;
    long long llscalar1 = 999999999LL;
    long long llscalar2 = 888888888LL;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Create expensive-to-spill computations (address calculations) */
        int* ptr1 = &arr1[i];                    /* Rematerialization candidate */
        int* ptr2 = &arr2[i & 0x3F];             /* Another candidate with mask */
        float* fptr1 = &farr1[i];                /* Float pointer candidate */
        float* fptr2 = &farr2[(i * 3) & 0x3FF];  /* More complex address */
        double* dptr1 = &darr1[i];               /* Double pointer candidate */
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * 4;                     /* Simple arithmetic - good remat candidate */
        int offset2 = (i + 1) * 4;               /* Another candidate */
        float foffset = i * 0.125f;              /* Float computation */
        double doffset = i * 0.0625;             /* Double computation */
        
        /* Conditional to prevent hoisting of computations */
        if (i % 7 == 0) {
            scalar1 = i * 2;
            fscalar1 = i * 0.75f;
        } else if (i % 13 == 0) {
            scalar2 = i * 3;
            fscalar2 = i * 1.25f;
        }
        
        /* Mix data types in computations */
        short sval = sarr1[i] + sscalar1;
        unsigned char cval = carr1[i] + cscalar1;
        
        /* Function call with many arguments - clobbers caller-saved registers */
        helper_func(scalar1, scalar2, scalar3 + offset1,
                   fscalar1 + foffset, dscalar1 + doffset,
                   sval, ptr1, ptr2, llscalar1 + i,
                   cval, fptr1, dptr1);
        
        /* Use results after call, keeping values live across call */
        arr1[i] = scalar1 + *ptr1 + offset1;
        arr2[i & 0x3F] = scalar2 + *ptr2 + offset2;
        farr1[i] = fscalar1 + *fptr1 + foffset;
        darr1[i] = dscalar1 + *dptr1 + doffset;
        
        /* More computations to increase pressure */
        scalar3 = scalar1 + scalar2 + (i % 5);
        fscalar2 = fscalar1 * 1.1f + (i % 3);
        dscalar2 = dscalar1 * 1.01 + (i % 7);
        sscalar2 = sscalar1 + (i & 0xFF);
        cscalar2 = cscalar1 + (i & 0x7F);
        llscalar2 = llscalar1 + i * 2LL;
        
        /* Another conditional branch */
        if (i % 11 == 0) {
            helper_func(scalar3, scalar1, scalar2,
                       fscalar2, dscalar2,
                       sscalar2, &arr1[(i + 1) & 0x3FF], &arr2[(i + 2) & 0x3FF],
                       llscalar2, cscalar2, fptr2, NULL);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)darr1[i] + sarr1[i] + carr1[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
