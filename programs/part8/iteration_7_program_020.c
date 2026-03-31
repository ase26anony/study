/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -c early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

/* Non-inline function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void clobber_function(int a, int b, int c, float d, double e, 
                      int *p1, int *p2, short *p3, float *p4) {
    /* Use all parameters to prevent dead argument elimination */
    global_volatile = a + b + c + (int)d + (int)e;
    if (p1) global_volatile += *p1;
    if (p2) global_volatile += *p2;
    if (p3) global_volatile += *p3;
    if (p4) global_volatile += (int)*p4;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_value(int x, float y, double z, int *ptr) {
    return x + (int)y + (int)z + (ptr ? *ptr : 0);
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    float arr4[1024];
    double arr5[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = (float)(i * 0.5f);
        arr5[i] = i * 0.25;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    int var4 = 4;
    short var5 = 5;
    float var6 = 6.0f;
    float var7 = 7.0f;
    double var8 = 8.0;
    double var9 = 9.0;
    int var10 = 10;
    int var11 = 11;
    int var12 = 12;
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            var1 = i * 2;
            var6 = (float)i * 0.3f;
        }
        
        /* Expensive-to-spill address computations (remat candidates) */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        short *ptr3 = &arr3[(i * 3) % 1024];     /* Complex index */
        float *ptr4 = &arr4[(i + 5) % 1024];     /* Offset index */
        double *ptr5 = &arr5[(i * 2) % 1024];    /* Another complex index */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication */
        int offset2 = (i + 1) * sizeof(short);   /* Another multiplication */
        float scale = (float)i * 0.01f;          /* Float multiplication */
        double dscale = (double)i * 0.001;       /* Double multiplication */
        
        /* Use all variables to keep them live */
        var2 = var1 + offset1;
        var3 = var2 * 2;
        var4 = var3 - offset2;
        var5 = (short)(var4 & 0xFFFF);
        var7 = var6 * scale;
        var8 = (double)var7 + dscale;
        var9 = var8 * 2.0;
        var10 = (int)var9;
        var11 = var10 + i;
        var12 = var11 * 3;
        
        /* Function call that clobbers caller-saved registers */
        clobber_function(var1, var2, var3, var6, var8, 
                        ptr1, ptr2, ptr3, ptr4);
        
        /* Use results after call - forces save/restore or remat */
        if (ptr1) *ptr1 += var4;
        if (ptr2) *ptr2 += var5;
        if (ptr3) *ptr3 = (short)(*ptr3 + var10);
        if (ptr4) *ptr4 += var7;
        if (ptr5) *ptr5 += var9;
        
        /* Another conditional to prevent optimization */
        if (i % 13 == 0) {
            /* Another function call with different arguments */
            int val = compute_value(var11, var6, var8, ptr1);
            arr1[i] += val;
        }
        
        /* Cross-iteration dependencies to prevent loop unrolling */
        var1 = arr1[i] % 100;
        var6 = (float)(arr4[i] * 0.5f);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += arr3[i];
        checksum += (unsigned long long)arr4[i];
        checksum += (unsigned long long)arr5[i];
    }
    
    /* Use checksum to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
