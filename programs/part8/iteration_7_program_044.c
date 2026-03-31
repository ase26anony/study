/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int c, float d, double e, 
                 int *p1, int *p2, short *p3, float *p4, double *p5) {
    /* Complex enough to use many registers */
    global_volatile = a + b + c + (int)d + (int)e;
    if (p1) *p1 = a;
    if (p2) *p2 = b;
    if (p3) *p3 = (short)c;
    if (p4) *p4 = d;
    if (p5) *p5 = e;
    
    /* Additional computation to increase register pressure in helper */
    int tmp1 = a * b;
    int tmp2 = c * (int)d;
    float tmp3 = d * 2.0f;
    double tmp4 = e * 3.0;
    global_volatile += tmp1 + tmp2 + (int)tmp3 + (int)tmp4;
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
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = i * 1.5f;
        arr5[i] = i * 2.5;
        arr6[i] = i % 256;
        arr7[i] = (i * 7) % 512;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 12345;
    int var2 = 67890;
    int var3 = 13579;
    int var4 = 24680;
    short var5 = 32000;
    float var6 = 3.14159f;
    double var7 = 2.71828;
    int var8 = 999;
    int var9 = 888;
    float var10 = 1.41421f;
    double var11 = 1.73205;
    int var12 = 777;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime-dependent condition */
        if (i % 7 == 0) {
            var1 += i;
            var6 += 0.1f;
        }
        
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        short *ptr3 = &arr3[(i * 3) % 1024];     /* Complex index */
        float *ptr4 = &arr4[i / 2];              /* Division */
        double *ptr5 = &arr5[1023 - i];          /* Reverse index */
        
        /* More address computations with different modes */
        int *ptr6 = &arr6[i % 256];
        int *ptr7 = &arr7[(i * 7) % 512];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication */
        int offset2 = (i & 0xFF) << 2;           /* Shift */
        float scale1 = i * 0.25f;
        double scale2 = i * 0.125;
        
        /* Update some variables to keep them live */
        var2 += offset1;
        var3 += offset2;
        var7 += scale2;
        var10 += scale1;
        
        /* Function call that clobbers caller-saved registers */
        helper_func(var1, var2, var3, var6, var7,
                   ptr1, ptr2, ptr3, ptr4, ptr5);
        
        /* Use computed values after the call (forces them to be live across call) */
        if (ptr6) {
            *ptr6 = var4 + var8 + (int)(var10 * 10.0f);
        }
        if (ptr7) {
            *ptr7 = var5 + var9 + (int)(var11 * 20.0);
        }
        
        /* More computations to increase pressure */
        var4 = var4 * 2 - i;
        var5 = (var5 + i) % 32768;
        var8 = (var8 * 3) % 1000;
        var9 = (var9 + var12) % 1000;
        var11 = var11 * 1.01;
        var12 = (var12 + i) % 1000;
        
        /* Conditional store based on runtime value */
        if (i % 13 == 0) {
            arr1[i] = var1 + var2 + var3;
            arr4[i / 2] = var6 + var10;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)arr4[i] + (int)arr5[i];
        checksum += arr6[i] + arr7[i];
    }
    checksum += var1 + var2 + var3 + var4 + var5 + (int)var6 + (int)var7;
    checksum += var8 + var9 + (int)var10 + (int)var11 + var12;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
