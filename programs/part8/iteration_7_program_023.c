/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int c, float d, double e, 
                 short f, int* ptr1, int* ptr2, float* fptr, double* dptr) {
    /* Perform operations that use all arguments to prevent dead argument elimination */
    global_volatile = a + b + c;
    *ptr1 = (*ptr2) * 2;
    *fptr = (float)(d * 2.0f);
    *dptr = e * 3.0;
    
    /* Additional computation to increase register pressure in caller */
    global_volatile += f;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    int arr3[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    double darr2[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farr1[i] = (float)i * 1.5f;
        farr2[i] = (float)i * 2.5f;
        darr1[i] = (double)i * 1.25;
        darr2[i] = (double)i * 3.75;
        sarr1[i] = (short)(i % 256);
    }
    
    /* Declare many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    int var4 = 4;
    int var5 = 5;
    int var6 = 6;
    int var7 = 7;
    int var8 = 8;
    float fvar1 = 1.1f;
    float fvar2 = 2.2f;
    double dvar1 = 3.3;
    double dvar2 = 4.4;
    short svar1 = 10;
    short svar2 = 20;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent loop invariant code motion with runtime condition */
        if (i % 7 == 0) {
            var1 = i * 2;      /* Expensive to spill, cheap to recompute */
            var2 = i + 5;
        } else {
            var1 = i * 3;
            var2 = i + 10;
        }
        
        /* More conditional computations to prevent hoisting */
        if (i % 13 == 0) {
            var3 = i * 4;
            var4 = i / 2;
        } else {
            var3 = i * 5;
            var4 = i / 3;
        }
        
        /* Compute address expressions - these are rematerialization candidates */
        int *ptr1 = &arr1[i];                    /* &arr1[i] - cheap to recompute */
        int *ptr2 = &arr2[i & 0x3F];             /* &arr2[i & 0x3F] - cheap with mask */
        float *fptr1 = &farr1[i % 512];          /* Different index computation */
        double *dptr1 = &darr1[(i * 3) % 1024];  /* More complex address computation */
        
        /* Additional computations that use the loop index in different ways */
        int offset1 = i * sizeof(int);           /* i * 4 - cheap to recompute */
        int offset2 = (i << 2) + 16;             /* i*4 + 16 - another candidate */
        float foffset = (float)i * 4.0f;         /* Float computation */
        double doffset = (double)i * 8.0;        /* Double computation */
        
        /* Update scalar variables to keep them live */
        var5 = var1 + var2;
        var6 = var3 * var4;
        var7 = var5 - var6;
        var8 = (var7 << 3) | (i & 0xFF);
        
        fvar1 = (float)var1 * 0.5f;
        fvar2 = fvar1 + (float)var2;
        dvar1 = (double)var3 * 0.25;
        dvar2 = dvar1 * (double)var4;
        
        svar1 = (short)(var5 & 0xFFFF);
        svar2 = (short)(var6 & 0x7FFF);
        
        /* Call non-inline function with many arguments - forces register pressure */
        helper_func(var1, var2, var3, fvar1, dvar1,
                   svar1, ptr1, ptr2, fptr1, dptr1);
        
        /* Use results after call - values must be kept live or rematerialized */
        arr3[i] = var7 + *ptr1;
        farr2[i] = fvar2 + (float)var8;
        darr2[i] = dvar2 + (double)(i % 64);
        sarr1[i] = svar2 + (short)(global_volatile & 0xFF);
        
        /* Additional computation using the address expressions */
        if (ptr1 != NULL) {
            *ptr1 = *ptr1 + var5;
        }
        if (ptr2 != NULL) {
            *ptr2 = *ptr2 - var6;
        }
        
        /* More conditional code to prevent optimization */
        if (i % 17 == 0) {
            var1 = var1 ^ var2;
            var3 = var3 | var4;
        }
        
        /* Use computed offsets */
        if (i % 23 == 0) {
            arr1[(i + 1) % 1024] = offset1;
            arr2[(i + 2) % 1024] = offset2;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += arr3[i];
        checksum += (unsigned int)farr1[i];
        checksum += (unsigned int)farr2[i];
        checksum += (unsigned long long)darr1[i];
        checksum += (unsigned long long)darr2[i];
        checksum += sarr1[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
