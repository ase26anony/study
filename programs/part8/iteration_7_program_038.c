/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, char *h, int *i, float *j) {
    /* Use all arguments to prevent optimization */
    global_volatile = a + b + *c + (int)d + (int)e + f + g + (int)*h;
    *i = *c + a;
    *j = d + 0.5f;
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
        arr2[i] = 1024 - i;
        arr3[i] = i * 0.5f;
        arr4[i] = i * 0.25;
        arr5[i] = i & 0x7FFF;
        arr6[i] = (i % 26) + 'a';
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    int var4 = 4;
    float var5 = 5.0f;
    double var6 = 6.0;
    short var7 = 7;
    long var8 = 8L;
    int var9 = 9;
    int var10 = 10;
    float var11 = 11.0f;
    double var12 = 12.0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Remat candidate: &arr1[i] */
        int *ptr2 = &arr2[i & 0x3F];             /* Remat candidate: &arr2[i & 0x3F] */
        float *ptr3 = &arr3[i % 512];           /* Different mode (SF) */
        double *ptr4 = &arr4[(i * 3) % 1024];   /* Different mode (DF) */
        short *ptr5 = &arr5[i];                 /* Different mode (HI) */
        char *ptr6 = &arr6[i % 256];            /* Different mode (QI) */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);          /* Remat candidate: i * 4 */
        int offset2 = (i + 1) * sizeof(int);    /* Remat candidate: (i + 1) * 4 */
        float float_offset = i * 0.125f;        /* Different mode */
        double double_offset = i * 0.0625;      /* Different mode */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = offset1 + var2;
            var5 = float_offset + var11;
        } else if (i % 13 == 0) {
            var2 = offset2 + var1;
            var6 = double_offset + var12;
        }
        
        /* Function call that clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, var5, var6, 
                   var7, var8, ptr6, ptr2, ptr3);
        
        /* Use results after call - values must be live or rematerialized */
        arr1[i] = var1 + *ptr2;
        arr2[i] = var2 + *ptr1;
        arr3[i] = var5 + *ptr3;
        arr4[i] = var6 + *ptr4;
        
        /* More computations using the pointers */
        if (i % 3 == 0) {
            *ptr5 = (short)(var7 + i);
            var7 = (*ptr5) & 0xFF;
        }
        
        /* Update scalars to keep them live */
        var1 = (var1 * 3) % 100;
        var2 = (var2 * 5) % 100;
        var3 = (var3 + var4) % 100;
        var4 = (var4 + i) % 100;
        var5 = var5 * 1.1f;
        var6 = var6 * 1.05;
        var8 = (var8 + i) % 1000;
        var9 = (var9 * 7) % 100;
        var10 = (var10 + var9) % 100;
        var11 = var11 * 0.9f;
        var12 = var12 * 0.95;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)arr3[i] + (int)arr4[i] + arr5[i] + arr6[i];
    }
    
    checksum += var1 + var2 + var3 + var4 + (int)var5 + (int)var6 + 
                var7 + var8 + var9 + var10 + (int)var11 + (int)var12;
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum % 1000);
}
