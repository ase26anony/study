/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC RTL backend
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile = 0;

/* Non-inline helper function to create register pressure across calls */
__attribute__((noinline))
void helper_func(int a, int b, short c, float d, double e,
                 int *p1, int *p2, float *p3, double *p4)
{
    /* Simple operations that use all arguments */
    global_volatile = a + b + c;
    *p1 = a * 2;
    *p2 = b / 2;
    *p3 = d * 2.0f;
    *p4 = e * 0.5;
    
    /* Additional computation to prevent tail-call optimization */
    for (int i = 0; i < 3; i++) {
        global_volatile += i;
    }
}

int main(void)
{
    /* Declare many arrays to work with */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    float arr4[1024];
    double arr5[1024];
    int arr6[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = (float)(i * 0.5f);
        arr5[i] = (double)(i * 0.25);
        arr6[i] = i * 2;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    short var4 = 4;
    short var5 = 5;
    float var6 = 6.0f;
    float var7 = 7.0f;
    double var8 = 8.0;
    double var9 = 9.0;
    int var10 = 10;
    int var11 = 11;
    int var12 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        short *ptr3 = &arr3[(i * 3) % 1024];     /* Complex index */
        float *ptr4 = &arr4[i];                  /* Float pointer */
        double *ptr5 = &arr5[1023 - i];          /* Reverse index */
        
        /* More address computations with different modes */
        int offset1 = i * sizeof(int);           /* Byte offset */
        int *ptr6 = (int *)((char *)arr6 + offset1);
        
        int offset2 = (i % 32) * 4;              /* Scaled offset */
        int *ptr7 = &arr1[offset2 / sizeof(int)];
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i * 2;
            var6 = (float)i * 0.123f;
        } else if (i % 13 == 0) {
            var2 = i / 3;
            var8 = (double)i * 0.456;
        }
        
        /* Update many variables to keep them live */
        var3 = var1 + var2;
        var4 = (short)(var3 % 256);
        var5 = (short)(var4 * 2);
        var7 = var6 * 2.0f;
        var9 = var8 * 1.5;
        var10 = var3 * var2;
        var11 = var10 - i;
        var12 = var11 / (var1 ? var1 : 1);
        
        /* Call helper function with many arguments - creates register pressure */
        helper_func(var1, var2, var4, var6, var8,
                   ptr1, ptr2, ptr4, ptr5);
        
        /* Use results after call - forces values to be live across call */
        arr1[i] = var3 + *ptr1;
        arr2[i & 0x3F] = var10 + *ptr2;
        arr3[(i * 3) % 1024] = (short)(var4 + var5);
        arr4[i] = var6 + var7;
        arr5[1023 - i] = var8 + var9;
        arr6[i] = var11 + var12;
        
        /* Additional computation to use all variables */
        if (i % 17 == 0) {
            *ptr6 = var1 + var2 + var3 + var10 + var11 + var12;
        }
        
        if (i % 19 == 0) {
            *ptr7 = var4 + var5;
        }
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
    
    /* Use checksum to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    
    return (int)(checksum % 1000);
}
