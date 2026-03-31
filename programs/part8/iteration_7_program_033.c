/* early-remat-test.c
 * Test case to trigger early rematerialization in GCC RTL backend.
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void clobber_function(int a, int b, int c, float d, float e, 
                      int *p1, int *p2, short *p3, double *p4) {
    /* Force register clobbering with inline asm */
    asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
                  "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    
    /* Store to global volatile to prevent DCE */
    global_volatile = a + b + c + (int)d + (int)e;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_value(int base, int offset, float scale, double factor) {
    /* Complex enough to not be inlined, simple enough to rematerialize */
    int result = (int)((base + offset) * scale * factor);
    
    /* Use inline asm to prevent optimization */
    asm volatile ("" : "+r"(result) : : "memory");
    return result;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
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
        arr6[i] = 0;
    }
    
    /* Declare many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    int var4 = 4;
    int var5 = 5;
    float var6 = 6.0f;
    float var7 = 7.0f;
    double var8 = 8.0;
    short var9 = 9;
    int var10 = 10;
    float var11 = 11.0f;
    double var12 = 12.0;
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            var1 = i * 2;      /* Simple computation - good remat candidate */
        } else {
            var1 = i * 3;      /* Different computation path */
        }
        
        /* More conditional computations to prevent optimization */
        var2 = (i % 3 == 0) ? i * 4 : i * 5;
        var3 = (i % 5 == 0) ? i * 6 : i * 7;
        
        /* Compute addresses - these are good rematerialization candidates */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked offset */
        short *ptr3 = &arr3[i % 512];            /* Different type */
        float *ptr4 = &arr4[i];                  /* Float pointer */
        double *ptr5 = &arr5[i];                 /* Double pointer */
        
        /* More address computations with different scales */
        int *ptr6 = &arr6[i * 2 % 1024];         /* Non-linear access */
        
        /* Scalar computations that could be rematerialized */
        int offset1 = i * sizeof(int);           /* Constant multiplication */
        int offset2 = (i << 2) + 16;             /* Shift and add */
        float scale1 = (float)i * 0.125f;        /* Float computation */
        double scale2 = (double)i * 0.0625;      /* Double computation */
        
        /* Call function that clobbers registers - forces save/restore or remat */
        clobber_function(var1, var2, var3, var6, var7,
                        ptr1, ptr2, ptr3, ptr5);
        
        /* Use computed values after call - they need to be live or rematerialized */
        int computed = compute_value(var1, offset1, scale1, scale2);
        
        /* More computations using the results */
        *ptr1 = var1 + var2 + var3;
        *ptr2 = computed;
        *ptr3 = (short)(var9 + i);
        *ptr4 = var6 + var7 + var11;
        *ptr5 = var8 + var12 + scale2;
        *ptr6 = var4 + var5 + var10;
        
        /* Update some variables to create dependencies */
        var4 = var4 + 1;
        var5 = var5 - 1;
        var6 = var6 * 1.01f;
        var7 = var7 * 0.99f;
        var8 = var8 + 0.5;
        var9 = (short)(var9 + 2);
        var10 = var10 * 3 % 100;
        var11 = var11 + 0.1f;
        var12 = var12 - 0.05;
        
        /* Another conditional to prevent optimization */
        if (i % 13 == 0) {
            /* Use all variables to keep them live */
            arr6[i] = var1 + var2 + var3 + var4 + var5 + var10 + 
                     (int)var6 + (int)var7 + (int)var8 + (int)var11 + (int)var12;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)arr4[i] + 
                   (int)arr5[i] + arr6[i];
    }
    
    /* Also use global volatile to prevent optimization */
    checksum += global_volatile;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
