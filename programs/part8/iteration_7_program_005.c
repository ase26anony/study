/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and exercise the uncovered lines in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void helper_func(int a, int b, int *c, short d, float e, double f, 
                 long g, char *h, unsigned int i, int *j, float *k, double *l) {
    /* Complex enough to use many registers */
    global_sink = a + b + d + (int)e + (int)f + g + *h + i;
    *c = (*c) * 2;
    *j = (*j) + 1;
    *k = (*k) + 1.0f;
    *l = (*l) + 1.0;
    
    /* Prevent tail call optimization */
    asm volatile("" : : : "memory");
}

int main(void) {
    /* Declare many arrays to create register pressure */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    float arr4[1024];
    double arr5[1024];
    char arr6[1024];
    unsigned int arr7[1024];
    
    /* Initialize arrays with pattern to prevent constant propagation */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = (float)(i * 0.5f);
        arr5[i] = (double)(i * 0.25);
        arr6[i] = (char)(i & 0x7F);
        arr7[i] = (unsigned int)(i * 7);
    }
    
    /* Many scalar variables to increase register pressure */
    int scalar1 = 1;
    int scalar2 = 2;
    short scalar3 = 3;
    float scalar4 = 4.0f;
    double scalar5 = 5.0;
    long scalar6 = 6L;
    char scalar7 = 7;
    unsigned int scalar8 = 8U;
    int scalar9 = 9;
    float scalar10 = 10.0f;
    double scalar11 = 11.0;
    int scalar12 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Candidate for remat */
        int *ptr2 = &arr2[i & 0x3F];             /* Non-linear addressing */
        short *ptr3 = &arr3[(i * 3) % 1024];     /* More complex address */
        float *ptr4 = &arr4[(i + 5) % 1024];     /* Different mode (SF) */
        double *ptr5 = &arr5[(i * 2) % 1024];    /* Different mode (DF) */
        char *ptr6 = &arr6[i];                   /* QI mode */
        unsigned int *ptr7 = &arr7[(i + 3) % 1024]; /* SI mode */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Simple multiplication */
        int offset2 = (i & 0xF) << 2;            /* Shift operation */
        float float_idx = (float)i * 0.1f;       /* Float computation */
        double double_idx = (double)i * 0.05;    /* Double computation */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            scalar1 = scalar2 * 3;
            ptr1 = &arr1[(i + 1) % 1024];        /* Change pointer */
        } else if (i % 13 == 0) {
            scalar4 = scalar4 + 1.0f;
            ptr4 = &arr4[(i + 2) % 1024];
        }
        
        /* Another conditional branch */
        if (i % 11 == 0) {
            scalar6 = scalar6 * 2L;
            scalar8 = scalar8 + 1U;
        }
        
        /* Function call that clobbers caller-saved registers */
        helper_func(scalar1, scalar2, ptr1, scalar3, scalar4, scalar5,
                   scalar6, ptr6, scalar8, ptr2, ptr4, ptr5);
        
        /* Use computed values after the call - forces them to be live across call */
        arr1[i] = *ptr1 + offset1;
        arr2[i & 0x3F] = *ptr2 + offset2;
        arr3[i] = (short)(*ptr3 + scalar3);
        arr4[i] = *ptr4 + float_idx;
        arr5[i] = *ptr5 + double_idx;
        
        /* Update scalars to create dependencies */
        scalar1 = scalar1 + 1;
        scalar2 = scalar2 + *ptr7;
        scalar3 = (short)(scalar3 + 1);
        scalar4 = scalar4 + *ptr4;
        scalar5 = scalar5 + *ptr5;
        scalar6 = scalar6 + i;
        scalar7 = (char)(scalar7 + arr6[i]);
        scalar8 = scalar8 + *ptr7;
        scalar9 = scalar9 + scalar1;
        scalar10 = scalar10 + scalar4;
        scalar11 = scalar11 + scalar5;
        scalar12 = scalar12 + scalar2;
        
        /* Another conditional to create control flow complexity */
        if (i % 17 == 0) {
            /* More address computations */
            int *alt_ptr = &arr1[(i * 5) % 1024];
            float *alt_fptr = &arr4[(i * 7) % 1024];
            *alt_ptr = *alt_ptr + scalar9;
            *alt_fptr = *alt_fptr + scalar10;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (unsigned int)arr4[i] 
                   + (unsigned long long)arr5[i] + arr6[i] + arr7[i];
    }
    
    /* Also use scalars in checksum */
    checksum += scalar1 + scalar2 + scalar3 + scalar4 + scalar5 
               + scalar6 + scalar7 + scalar8 + scalar9 + scalar10 
               + scalar11 + scalar12;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
