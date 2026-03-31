/* early-remat-test.c
 * Test case for GCC early rematerialization pass coverage
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, int *g, long h, float *i, double *j,
                 int k, int l) {
    /* Use all arguments to prevent optimization */
    global_volatile = a + b + *c + (int)d + (int)e + f + *g + (int)h;
    if (i) global_volatile += (int)(*i);
    if (j) global_volatile += (int)(*j);
    global_volatile += k + l;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    double darr2[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        farr1[i] = i * 0.5f;
        farr2[i] = (1023 - i) * 0.25f;
        darr1[i] = i * 0.125;
        darr2[i] = (1023 - i) * 0.0625;
        sarr1[i] = i & 0x7FFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 1.11, dvar2 = 2.22;
    short svar1 = 10, svar2 = 20;
    long lvar1 = 100, lvar2 = 200;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float *fptr1 = &farr1[(i * 3) % 1024];   /* Complex addressing */
        float *fptr2 = &farr2[(i * 5) % 1024];
        double *dptr1 = &darr1[(i * 7) % 1024];
        double *dptr2 = &darr2[(i * 11) % 1024];
        short *sptr1 = &sarr1[(i * 13) % 1024];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication */
        int offset2 = (i & 0xFF) * 4;            /* Masked multiplication */
        float fscale = i * 0.333f;               /* Float multiplication */
        double dscale = i * 0.777;               /* Double multiplication */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = var2 * 3;
            fvar1 = fvar2 * 2.0f;
        } else if (i % 13 == 0) {
            var2 = var3 * 4;
            fvar2 = fvar3 * 3.0f;
        }
        
        /* Update many variables to keep them live */
        var3 = var4 + i;
        var4 = var5 + (i & 0xF);
        var5 = var1 + var2;
        fvar3 = fvar1 + fvar2 + i * 0.1f;
        dvar1 = dvar2 + i * 0.01;
        dvar2 = dvar1 * 1.1;
        svar1 = (svar2 + i) & 0x7F;
        svar2 = svar1 * 2;
        lvar1 = lvar2 + i * 10L;
        lvar2 = lvar1 - 5L;
        
        /* Call helper function with many arguments - clobbers caller-saved regs */
        helper_func(var1, var2, ptr1, fvar1, dvar1,
                   svar1, ptr2, lvar1, fptr1, dptr1,
                   offset1, offset2);
        
        /* Use computed values after call (forces save/restore or rematerialization) */
        arr1[i] = *ptr1 + var3 + offset1;
        arr2[i & 0x3F] = *ptr2 + var4 + offset2;
        farr1[(i * 3) % 1024] = *fptr1 + fvar3 + fscale;
        farr2[(i * 5) % 1024] = *fptr2 + fvar1 + fvar2;
        darr1[(i * 7) % 1024] = *dptr1 + dvar1 + dscale;
        darr2[(i * 11) % 1024] = *dptr2 + dvar2;
        sarr1[(i * 13) % 1024] = *sptr1 + svar1 + svar2;
        
        /* Another conditional to create control flow complexity */
        if (i % 17 == 0) {
            /* Additional computations that could be rematerialized */
            int *alt_ptr = &arr1[(i + 1) % 1024];
            float *alt_fptr = &farr1[(i + 2) % 1024];
            *alt_ptr = var5 + i;
            *alt_fptr = fvar3 + i * 0.5f;
            
            /* Second function call in high-pressure region */
            helper_func(var5, var3, alt_ptr, fvar3, dvar2,
                       svar2, ptr1, lvar2, alt_fptr, dptr2,
                       i, i * 2);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
        checksum += (int)darr1[i] + (int)darr2[i] + sarr1[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
