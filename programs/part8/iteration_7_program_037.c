/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int c, float d, double e, 
                 short f, int *ptr1, int *ptr2, float *ptr3, double *ptr4)
{
    /* Perform operations that use all arguments to prevent optimization */
    global_volatile = a + b + c;
    *ptr1 = a * b;
    *ptr2 = c + 1;
    *ptr3 = d * 2.0f;
    *ptr4 = e / 2.0;
    
    /* Additional operations to increase register pressure in caller */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
                  "r"(f), "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4) : "memory");
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float arr3[1024];
    double arr4[1024];
    short arr5[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = i * 0.5f;
        arr4[i] = i * 0.25;
        arr5[i] = i & 0x7F;
    }
    
    /* Declare many scalar variables with different types */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    float var4 = 4.0f;
    double var5 = 5.0;
    short var6 = 6;
    int var7 = 7;
    int var8 = 8;
    float var9 = 9.0f;
    double var10 = 10.0;
    short var11 = 11;
    int var12 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float *ptr3 = &arr3[(i * 3) % 1024];     /* More complex address */
        double *ptr4 = &arr4[(i + 5) % 1024];    /* Another address */
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * 4;                     /* Multiplication */
        int offset2 = (i + 1) * 8;               /* Another multiplication */
        float scale1 = i * 0.125f;               /* Float multiplication */
        double scale2 = i * 0.0625;              /* Double multiplication */
        
        /* Use conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = offset1 + 1;
            var4 = scale1 * 2.0f;
        } else if (i % 13 == 0) {
            var2 = offset2 - 1;
            var5 = scale2 / 2.0;
        }
        
        /* Update scalar variables to keep them live */
        var3 = var1 + var2;
        var6 = (short)(var3 & 0xFF);
        var7 = var3 * 2;
        var8 = var7 + i;
        var9 = var4 + (float)var5;
        var10 = (double)var4 * var5;
        var11 = (short)((var7 + var8) & 0x7F);
        var12 = var8 * 3;
        
        /* Call helper function - clobbers caller-saved registers */
        helper_func(var1, var2, var3, var4, var5,
                   var6, ptr1, ptr2, ptr3, ptr4);
        
        /* Use values after call (forces them to be live across call) */
        arr1[i] = var7 + *ptr1;
        arr2[i & 0x3F] = var8 + *ptr2;
        arr3[(i * 3) % 1024] = var9 + *ptr3;
        arr4[(i + 5) % 1024] = var10 + *ptr4;
        arr5[i] = var11 + (short)var12;
        
        /* More computations to increase pressure */
        var1 = (var1 + 1) % 100;
        var2 = (var2 * 2) % 200;
        var3 = (var3 + var1 + var2) % 300;
        var4 = var4 * 1.1f;
        var5 = var5 * 1.05;
        var6 = (var6 + 1) & 0x7F;
        var7 = (var7 + var3) % 400;
        var8 = (var8 * 3) % 500;
        var9 = var9 + 0.5f;
        var10 = var10 - 0.25;
        var11 = (var11 + 2) & 0x7F;
        var12 = (var12 + var7) % 600;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)arr3[i] + (int)arr4[i] + arr5[i];
    }
    
    /* Use checksum to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
