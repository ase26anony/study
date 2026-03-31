/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC RTL backend
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int c, float d, double e, 
                 int *p1, int *p2, short *p3, float *p4, double *p5) {
    /* Complex enough to prevent inlining, but simple enough to not dominate runtime */
    global_volatile = a + b + c + (int)d + (int)e;
    if (p1) *p1 += a;
    if (p2) *p2 += b;
    if (p3) *p3 = (short)(c & 0xFFFF);
    if (p4) *p4 = d * 2.0f;
    if (p5) *p5 = e * 3.0;
    
    /* Additional computation to increase register pressure in caller */
    asm volatile("" : : : "memory");
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
        arr4[i] = i * 1.5f;
        arr5[i] = i * 2.5;
        arr6[i] = i * 7;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    int var4 = 4;
    int var5 = 5;
    short var6 = 6;
    float var7 = 7.0f;
    double var8 = 8.0;
    int var9 = 9;
    int var10 = 10;
    float var11 = 11.0f;
    double var12 = 12.0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent loop invariant code motion with runtime condition */
        if (i % 13 == 0) {
            var1 = (var1 * 3) % 100;
            var7 = var7 * 1.1f;
        }
        
        /* Compute multiple address expressions - candidates for rematerialization */
        int *ptr1 = &arr1[i];                    /* Simple address computation */
        int *ptr2 = &arr2[i & 0x3F];             /* More complex address with mask */
        short *ptr3 = &arr3[(i * 3) % 1024];     /* Non-trivial index computation */
        float *ptr4 = &arr4[(i + 5) % 1024];     /* Offset address */
        double *ptr5 = &arr5[(i * 2) % 1024];    /* Scaled index */
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Scaled offset - remat candidate */
        int offset2 = (i << 2) + 16;             /* Shift and add - remat candidate */
        float scale_factor = i * 0.25f;          /* Floating computation */
        double dbl_offset = i * 8.0;             /* Double computation */
        
        /* Use all variables to keep them live */
        var2 = var1 + offset1;
        var3 = var2 * 2;
        var4 = var3 - offset2;
        var5 = (var4 + i) % 256;
        var6 = (short)(var5 & 0xFF);
        var8 = var7 + dbl_offset;
        var9 = var10 * scale_factor;
        var11 = var7 * 2.0f + scale_factor;
        var12 = var8 * 1.5;
        
        /* Function call that clobbers caller-saved registers */
        helper_func(var1, var2, var3, var7, var8,
                   ptr1, ptr2, ptr3, ptr4, ptr5);
        
        /* Use computed values after the call - forces save/restore or rematerialization */
        arr6[i] = var4 + var5 + (int)var11 + (int)var12;
        
        /* More address computations after the call - may need rematerialization */
        int *ptr6 = &arr6[(i + var1) % 1024];    /* Depends on var1 which may be remat'd */
        float *ptr7 = &arr4[(i + var2) % 1024];  /* Depends on var2 */
        
        /* Store using rematerialization candidates */
        *ptr6 = var9;
        *ptr7 = var11;
        
        /* Conditional update to prevent optimization */
        if (i % 7 == 0) {
            var10 = var9 + 1;
            var7 = var11 * 0.9f;
        } else if (i % 5 == 0) {
            var10 = var9 - 1;
            var8 = var12 * 0.8;
        }
        
        /* Cross-iteration dependencies to prevent hoisting */
        var1 = var2 + (i % 3);
        var7 = var11 + (i % 10);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)arr4[i] + (int)arr5[i] + arr6[i];
        /* Mix in some operations to use all variables */
        checksum += var1 + var2 + var3 + var4 + var5 + var6 + 
                   (int)var7 + (int)var8 + var9 + var10 + 
                   (int)var11 + (int)var12;
    }
    
    /* Use the checksum to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
