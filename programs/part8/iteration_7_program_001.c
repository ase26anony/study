/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, short c, float d, double e, 
                int *p1, int *p2, float *p3, double *p4, char *p5) {
    /* Complex enough to use multiple registers */
    global_sink = a + b + c + (int)d + (int)e;
    if (p1) global_sink += *p1;
    if (p2) global_sink += *p2;
    if (p3) global_sink += (int)*p3;
    if (p4) global_sink += (int)*p4;
    if (p5) global_sink += *p5;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Declare many arrays to work with */
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
    short var3 = 3;
    float var4 = 4.0f;
    double var5 = 5.0;
    int var6 = 6;
    int var7 = 7;
    short var8 = 8;
    float var9 = 9.0f;
    double var10 = 10.0;
    int var11 = 11;
    int var12 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            var1 = i * 2;
            var4 = i * 0.33f;
        }
        
        /* Expensive-to-spill address computations (remat candidates) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float *ptr3 = &arr3[i % 512];            /* Different array, different type */
        double *ptr4 = &arr4[(i * 3) % 1024];    /* Non-trivial index computation */
        char *ptr5 = &arr6[(i + 5) & 0x3FF];     /* Another address computation */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication by constant */
        int offset2 = (i << 2) + 16;             /* Shift and add */
        float scale = i * 0.125f + 1.0f;         /* FP computation */
        double dscale = i * 0.0625 + 2.0;        /* Double computation */
        
        /* Use all variables to keep them live */
        var2 = var1 + offset1;
        var3 = (short)(var2 & 0xFFFF);
        var5 = var4 * dscale;
        var6 = var2 + var11;
        var7 = var6 * 3;
        var8 = (short)(var7 & 0x7FFF);
        var9 = var4 * scale;
        var10 = var5 + 1.0;
        var11 = var12 + i;
        var12 = var11 * 2;
        
        /* Function call clobbers caller-saved registers, forcing decisions
           about which values to keep in registers vs. rematerialize */
        use_values(var1, var2, var3, var4, var5,
                   ptr1, ptr2, ptr3, ptr4, ptr5);
        
        /* Use computed values after call - they need to be available */
        arr1[i] = var6 + (int)var9;
        arr2[i & 0x3F] = var7 + (int)var10;
        arr3[i % 512] = var4 * 2.0f;
        arr4[(i * 3) % 1024] = var5 * 1.5;
        
        /* Conditional store based on runtime value */
        if (i % 13 == 0) {
            arr5[i] = var8;
            arr6[(i + 5) & 0x3FF] = (char)(var12 & 0xFF);
        }
        
        /* Another conditional to prevent optimization */
        if (i % 29 == 0) {
            var1 = arr1[i] + arr2[i & 0x3F];
            var4 = arr3[i % 512] + arr4[(i * 3) % 1024];
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)arr3[i] + (int)arr4[i] + arr5[i] + arr6[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
