/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early_remat pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline)) 
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, int *h, float *i, double *j) {
    /* Complex enough to use many registers */
    global_volatile_sink = a + b + *c + (int)d + (int)e + f + g;
    if (h) global_volatile_sink += *h;
    if (i) global_volatile_sink += (int)(*i);
    if (j) global_volatile_sink += (int)(*j);
    
    /* Prevent tail call optimization */
    asm volatile("" : : : "memory");
}

int main(void) {
    /* Declare many arrays to work with */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    double darr2[1024];
    short sarr1[1024];
    long larr1[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        farr1[i] = i * 1.5f;
        farr2[i] = (1023 - i) * 2.0f;
        darr1[i] = i * 3.14159;
        darr2[i] = (1023 - i) * 1.41421;
        sarr1[i] = i & 0x7FFF;
        larr1[i] = i * 1000L;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 10.1, dvar2 = 20.2;
    short svar1 = 100, svar2 = 200;
    long lvar1 = 10000, lvar2 = 20000;
    int *ptr1, *ptr2, *ptr3, *ptr4;
    float *fptr1, *fptr2;
    double *dptr1, *dptr2;
    
    int checksum = 0;
    
    /* High register pressure loop with complex addressing */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill address computations - candidates for remat */
        ptr1 = &arr1[i];                    /* Base + index */
        ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        ptr3 = &arr1[(i * 3) & 0x3FF];      /* Base + scaled masked index */
        ptr4 = &arr2[1023 - i];             /* Base + reversed index */
        
        fptr1 = &farr1[i];                  /* Float pointer */
        fptr2 = &farr2[(i + 5) & 0x3FF];    /* Offset float pointer */
        
        dptr1 = &darr1[i];                  /* Double pointer */
        dptr2 = &darr2[(i * 2) & 0x3FF];    /* Scaled double pointer */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i * 2;      /* Simple computation - cheap to remat */
            fvar1 = i * 1.7f;
            dvar1 = i * 2.71828;
        } else if (i % 13 == 0) {
            var2 = i * 3;      /* Another cheap computation */
            fvar2 = i * 3.14f;
            dvar2 = i * 1.61803;
        }
        
        /* More cheap computations */
        var3 = i * 4;          /* Very cheap - good remat candidate */
        var4 = i * 5 + var1;   /* Slightly more complex */
        var5 = (i << 2) | 1;   /* Bit operations */
        
        fvar3 = fvar1 + fvar2; /* Float operation */
        svar1 = (short)(i & 0xFF);
        svar2 = (short)((i + 100) & 0xFF);
        lvar1 = i * 100L;
        lvar2 = i * 200L + lvar1;
        
        /* Function call clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, fvar1, dvar1,
                   svar1, lvar1, ptr2, fptr1, dptr1);
        
        /* Use computed values after call - need to keep them live or remat */
        *ptr3 = var3 + var4;
        *ptr4 = var5 * 2;
        *fptr2 = fvar3 * 2.0f;
        *dptr2 = dvar2 * 3.0;
        
        /* More conditional code to complicate control flow */
        if (i % 11 == 0) {
            arr1[i] += var1;
            farr1[i] += fvar1;
            darr1[i] += dvar1;
        }
        
        if (i % 17 == 0) {
            arr2[i] += var2;
            farr2[i] += fvar2;
            darr2[i] += dvar2;
        }
        
        /* Update checksum to prevent elimination */
        checksum += *ptr1 + *ptr2 + *ptr3 + *ptr4 + 
                   (int)*fptr1 + (int)*fptr2 + 
                   (int)*dptr1 + (int)*dptr2 +
                   var1 + var2 + var3 + var4 + var5 +
                   (int)fvar1 + (int)fvar2 + (int)fvar3 +
                   (int)dvar1 + (int)dvar2 +
                   svar1 + svar2 + lvar1 + lvar2;
    }
    
    /* Final computation to use all arrays */
    int final_sum = 0;
    for (int i = 0; i < 1024; i++) {
        final_sum += arr1[i] + arr2[i] + 
                    (int)farr1[i] + (int)farr2[i] + 
                    (int)darr1[i] + (int)darr2[i] +
                    sarr1[i] + larr1[i];
    }
    
    checksum += final_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
