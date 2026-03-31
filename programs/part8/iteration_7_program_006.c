/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and exercise the uncovered code block in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, char *h, int i, float j) {
    /* Complex enough to use many registers */
    global_sink = a + b + *c + (int)d + (int)e + f + (int)g + (int)h[i] + (int)j;
    
    /* Some computation to prevent tail-call optimization */
    for (int k = 0; k < 2; k++) {
        global_sink += k;
    }
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float arr3[1024];
    double arr4[1024];
    short arr5[1024];
    char arr6[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = i * 0.5f;
        arr4[i] = i * 0.25;
        arr5[i] = i & 0x7FFF;
        arr6[i] = i & 0xFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    float var4 = 4.0f;
    double var5 = 5.0;
    short var6 = 6;
    long var7 = 7L;
    char var8 = '8';
    int var9 = 9;
    float var10 = 10.0f;
    int var11 = 11;
    double var12 = 12.0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent invariant hoisting with runtime condition */
        if (i % 7 == 0) {
            var1 = i * 2;
        } else if (i % 13 == 0) {
            var2 = i * 3;
        }
        
        /* Compute expensive-to-spill but cheap-to-recompute addresses */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float *ptr3 = &arr3[i % 512];            /* Different array, different type */
        double *ptr4 = &arr4[(i * 3) % 1024];    /* More complex computation */
        char *ptr5 = &arr6[i];                   /* Byte pointer */
        
        /* More address computations with different modes */
        short *ptr6 = &arr5[(i + var1) & 0x3FF];
        int *ptr7 = &arr1[(i << 2) % 1024];
        
        /* Mixed-type computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication by constant */
        int offset2 = (i & 0xFF) << 2;           /* Shift operation */
        float float_offset = i * 0.125f;         /* Float multiplication */
        double dbl_offset = i * 0.0625;          /* Double multiplication */
        
        /* Use all variables to keep them live */
        var3 = var1 + var2;
        var4 = var3 * 0.5f;
        var5 = var4 * 2.0;
        var6 = (var1 + var2) & 0x7FFF;
        var7 = var1 * 100L + var2;
        var8 = (var1 + var2) & 0xFF;
        var9 = var3 * var1;
        var10 = var9 * 0.1f;
        var11 = var9 + var1;
        var12 = var11 * 0.01;
        
        /* Call helper function - clobbers caller-saved registers */
        /* Pass computed addresses and scalars to force spills/remats */
        helper_func(var1, var2, ptr1, var4, var5,
                   var6, var7, ptr5, var9, var10);
        
        /* Use results after call - values need to be rematerialized */
        *ptr1 = var1 + var3;
        *ptr2 = var2 * var11;
        *ptr3 = var4 + var10;
        *ptr4 = var5 + var12;
        *ptr6 = var6 + (short)var1;
        arr1[(i + 1) % 1024] = *ptr7 + var9;
        
        /* More computations to increase pressure */
        if (i % 19 == 0) {
            var1 = var2 + *ptr1;
        }
        if (i % 23 == 0) {
            var2 = var1 + *ptr2;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)arr3[i] + (int)arr4[i] + arr5[i] + arr6[i];
    }
    
    /* Also use all scalar variables in final computation */
    checksum += var1 + var2 + var3 + (int)var4 + (int)var5 + var6 + var7 + var8 + var9 + (int)var10 + var11 + (int)var12;
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
