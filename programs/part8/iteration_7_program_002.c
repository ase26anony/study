/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, short d, float e, double f, 
                 int *g, int h, long i, float *j, double k, int l) {
    /* Use all arguments to prevent optimization */
    global_volatile = a + b + *c + d + (int)e + (int)f + *g + h + (int)i + (int)*j + (int)k + l;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024], arr2[1024], arr3[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
        sarr1[i] = i & 0xFFFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    short var6 = 6, var7 = 7;
    float var8 = 8.0f, var9 = 9.0f;
    double var10 = 10.0;
    long var11 = 11;
    int var12 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        int *ptr3 = &arr3[i * 2 % 1024];         /* Base + computed index */
        float *fptr1 = &farr1[i];
        float *fptr2 = &farr2[(i + 1) % 1024];
        double *dptr1 = &darr1[i];
        short *sptr1 = &sarr1[i];
        
        /* More computations that are cheap to recompute */
        int offset1 = i * 4;                     /* Multiplication */
        int offset2 = (i + 1) * 8;
        float foffset = i * 1.25f;
        double doffset = i * 2.71828;
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i;        /* Modify variable to prevent optimization */
            var8 = i * 0.5f;
        }
        
        if (i % 13 == 0) {
            var2 = i * 2;
            var10 = i * 1.234;
        }
        
        /* Function call with many arguments - clobbers caller-saved regs */
        helper_func(var1, var2, ptr1, var6, var8, var10,
                   ptr2, var3, var11, fptr1, var10, var12);
        
        /* Use computed values after call (must be kept live or rematerialized) */
        arr1[i] = *ptr1 + offset1 + var1;
        arr2[i & 0x3F] = *ptr2 + offset2 + var2;
        arr3[i * 2 % 1024] = *ptr3 + var3;
        
        farr1[i] = *fptr1 + foffset + var8;
        farr2[(i + 1) % 1024] = *fptr2 + var9;
        darr1[i] = *dptr1 + doffset + var10;
        sarr1[i] = *sptr1 + var6;
        
        /* Update variables to create dependencies */
        var1 = (var1 + 1) & 0xFF;
        var2 = (var2 * 3) & 0xFFF;
        var3 = var3 ^ i;
        var6 = (var6 + 2) & 0x7FFF;
        var8 = var8 * 1.01f;
        var9 = var9 - 0.5f;
        var10 = var10 * 1.001;
        var11 = var11 + i;
        var12 = var12 - (i & 1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i % 64] + arr3[i] + 
                   (int)farr1[i] + (int)farr2[i] + (int)darr1[i] + sarr1[i];
        checksum &= 0xFFFFFF;  /* Prevent overflow */
    }
    
    checksum += global_volatile;
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
