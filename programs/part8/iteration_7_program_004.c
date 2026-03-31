/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and reach uncovered lines in early-remat.cc (lines 930-937)
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, int *h, float *i, double *j) {
    /* Complex enough to use many registers */
    global_volatile = a + b + *c + (int)d + (int)e + f + g;
    if (h) *h = a * 2;
    if (i) *i = d * 2.0f;
    if (j) *j = e * 2.0;
    
    /* Some arithmetic to ensure it's not optimized away */
    for (int k = 0; k < 4; k++) {
        global_volatile += k;
    }
}

int main(void) {
    /* Declare many arrays to work with */
    int arr1[1024];
    int arr2[1024];
    float arr3[1024];
    double arr4[1024];
    short arr5[1024];
    long arr6[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 1.5f;
        arr4[i] = i * 2.5;
        arr5[i] = i & 0x7FFF;
        arr6[i] = i * 3L;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    float var4 = 4.0f;
    double var5 = 5.0;
    short var6 = 6;
    long var7 = 7L;
    int var8 = 8;
    float var9 = 9.0f;
    double var10 = 10.0;
    int var11 = 11;
    int var12 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float *ptr3 = &arr3[i % 512];            /* Different array type */
        double *ptr4 = &arr4[(i * 3) % 1024];    /* More complex index */
        
        /* More address computations with different modes */
        short *ptr5 = &arr5[(i + 7) % 1024];
        long *ptr6 = &arr6[(i * 5) % 1024];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Could be rematerialized */
        int offset2 = (i & 0xFF) << 2;           /* Shift operation */
        float float_offset = i * 0.25f;          /* Float computation */
        double dbl_offset = i * 0.125;           /* Double computation */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i * 2;
            var4 = i * 1.1f;
        } else if (i % 13 == 0) {
            var2 = i / 2;
            var5 = i * 0.9;
        }
        
        /* Update many variables to keep them live */
        var3 = var1 + var2;
        var6 = (short)(var3 & 0xFFFF);
        var7 = var3 * 3L;
        var8 = offset1 + offset2;
        var9 = float_offset * 2.0f;
        var10 = dbl_offset * 3.0;
        var11 = *ptr1 + *ptr2;
        var12 = (int)(var9 + var10);
        
        /* Function call that clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, var4, var5,
                   var6, var7, ptr2, ptr3, ptr4);
        
        /* Use results after call - forces values to be live across call */
        arr1[i] = var3 + global_volatile;
        arr2[i] = var8 + *ptr1;
        arr3[i] = var9 + float_offset;
        arr4[i] = var10 + dbl_offset;
        arr5[i] = var6 + (short)i;
        arr6[i] = var7 + i;
        
        /* More conditional code to prevent optimization */
        if (i % 17 == 0) {
            /* Additional computations */
            int temp = var11 * var12;
            arr1[i] += temp;
            arr2[i] -= temp;
        }
        
        /* Cross-iteration dependencies to prevent loop unrolling */
        var1 = arr1[i] % 256;
        var2 = arr2[i] % 128;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)arr3[i] + (int)arr4[i] 
                  + arr5[i] + arr6[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum % 1000);
}
