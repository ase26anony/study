/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targeting lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int c, int d, 
                 float e, float f, double g, double h,
                 int* ptr1, int* ptr2, short* ptr3, char* ptr4) {
    /* Do something trivial but visible */
    global_volatile = a + b + c + d + (int)e + (int)f + (int)g + (int)h;
    if (ptr1) *ptr1 += a;
    if (ptr2) *ptr2 += b;
    if (ptr3) *ptr3 += (short)c;
    if (ptr4) *ptr4 += (char)d;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    char arr4[1024];
    float arr5[1024];
    double arr6[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = (char)(i % 256);
        arr5[i] = (float)i * 1.5f;
        arr6[i] = (double)i * 2.5;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float var6 = 6.0f, var7 = 7.0f, var8 = 8.0f;
    double var9 = 9.0, var10 = 10.0;
    short var11 = 11;
    char var12 = 12;
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        short *ptr3 = &arr3[(i * 7) % 1024];     /* Complex index */
        char *ptr4 = &arr4[(i + 5) % 1024];      /* Offset index */
        float *ptr5 = &arr5[i % 512];            /* Different size */
        double *ptr6 = &arr6[(i * 3) % 1024];    /* Another complex index */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication */
        int offset2 = (i & 0xFF) << 2;           /* Shift */
        float foffset = (float)i * 4.0f;         /* Float multiplication */
        double doffset = (double)i * 8.0;        /* Double multiplication */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i * 2;
            var6 = (float)i * 3.0f;
            var9 = (double)i * 4.0;
        } else if (i % 13 == 0) {
            var2 = i * 3;
            var7 = (float)i * 5.0f;
            var10 = (double)i * 6.0;
        }
        
        /* Function call that clobbers caller-saved registers */
        helper_func(var1, var2, var3, var4,
                   var6, var7, var9, var10,
                   ptr1, ptr2, ptr3, ptr4);
        
        /* Use computed values after call (must be rematerialized) */
        arr1[i] += *ptr1 + offset1;
        arr2[i & 0x3F] += *ptr2 + offset2;
        arr3[(i * 7) % 1024] += (short)(*ptr3 + var11);
        arr4[(i + 5) % 1024] += (char)(*ptr4 + var12);
        
        /* More operations with different data types */
        arr5[i % 512] = *ptr5 + foffset + var6 + var7 + var8;
        arr6[(i * 3) % 1024] = *ptr6 + doffset + var9 + var10;
        
        /* Update scalars to keep them live */
        var1 = (var1 + 1) % 100;
        var2 = (var2 + 2) % 100;
        var3 = (var3 + 3) % 100;
        var4 = (var4 + 4) % 100;
        var5 = (var5 + 5) % 100;
        var6 = var6 + 1.0f;
        var7 = var7 + 2.0f;
        var8 = var8 + 3.0f;
        var9 = var9 + 4.0;
        var10 = var10 + 5.0;
        var11 = (var11 + 6) % 32767;
        var12 = (var12 + 7) % 127;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i];
        checksum += (unsigned long long)arr5[i];
        checksum += (unsigned long long)arr6[i];
    }
    
    /* Use checksum and global to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    printf("Global: %d\n", global_volatile);
    
    return (checksum % 1000);
}
