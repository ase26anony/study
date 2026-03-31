/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -o test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, int *h, float *i, double *j) {
    /* Perform operations that clobber registers */
    global_volatile = a + b + *c + (int)d + (int)e + f + g;
    if (h) *h = a;
    if (i) *i = d * 2.0f;
    if (j) *j = e * 3.0;
}

/* Another non-inline helper with different signature */
__attribute__((noinline, noipa))
int helper_func2(int *ptr1, int *ptr2, float f1, double d1, 
                 int i1, int i2, short s1, long l1) {
    int sum = *ptr1 + *ptr2 + (int)f1 + (int)d1 + i1 + i2 + s1 + l1;
    global_volatile += sum;
    return sum & 0xFF;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    int arr3[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[512];
    short sarr1[2048];
    long larr1[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        larr1[i] = i * 4L;
    }
    for (int i = 0; i < 512; i++) {
        darr1[i] = i * 3.14159;
    }
    for (int i = 0; i < 2048; i++) {
        sarr1[i] = i & 0x7FFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 1.111, dvar2 = 2.222;
    short svar1 = 10, svar2 = 20;
    long lvar1 = 100, lvar2 = 200;
    int *ptr1, *ptr2, *ptr3;
    float *fptr1, *fptr2;
    double *dptr1;
    
    int checksum = 0;
    
    /* High-pressure loop with many live variables */
    for (int i = 0; i < 1024; i++) {
        /* Compute address expressions that are cheap to rematerialize */
        ptr1 = &arr1[i];                    /* Base + index */
        ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        ptr3 = &arr3[(i * 3) % 1024];       /* More complex addressing */
        fptr1 = &farr1[i];
        fptr2 = &farr2[(i + 5) % 1024];
        dptr1 = &darr1[i % 512];
        
        /* Use conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i * 2;      /* Simple computation */
            fvar1 = i * 1.7f;
            dvar1 = i * 2.9;
        } else if (i % 13 == 0) {
            var2 = i * 3;
            fvar2 = i * 2.8f;
            dvar2 = i * 4.7;
        }
        
        /* More cheap computations */
        int offset1 = i * sizeof(int);      /* Could be rematerialized */
        int offset2 = (i << 2) + 16;        /* Shift + add */
        float foffset = i * 4.0f / 3.0f;
        double doffset = i * 6.28318 / 2.0;
        
        /* Call helper - clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, fvar1, dvar1, 
                   svar1, lvar1, ptr2, fptr1, dptr1);
        
        /* Use values after call - they need to be preserved/rematerialized */
        int temp1 = *ptr1 + var3 + offset1;
        int temp2 = *ptr2 + var4 + offset2;
        float ftemp = *fptr1 + fvar3 + foffset;
        double dtemp = *dptr1 + dvar2 + doffset;
        
        /* Another call with different arguments */
        int ret = helper_func2(ptr3, &arr1[(i + 1) % 1024], 
                              ftemp, dtemp, temp1, temp2, svar2, lvar2);
        
        /* Conditional store based on runtime value */
        if (ret > 128) {
            arr1[i] = temp1 + ret;
            farr1[i] = ftemp * 1.5f;
        } else {
            arr2[i] = temp2 - ret;
            farr2[i % 1024] = ftemp * 0.5f;
        }
        
        /* Update checksum with mixed computations */
        checksum += *ptr1 + *ptr2 + (int)(*fptr1) + (int)(*dptr1) + ret;
        checksum += var1 + var2 + var3 + var4 + var5;
        checksum += (int)fvar1 + (int)fvar2 + (int)fvar3;
        checksum += (int)dvar1 + (int)dvar2;
        checksum += svar1 + svar2;
        checksum += (int)(lvar1 & 0xFF) + (int)(lvar2 & 0xFF);
        
        /* Modify some variables to create data dependencies */
        var3 = (var3 + i) & 0xFF;
        var4 = (var4 * 2 - i) & 0xFFF;
        var5 = (var5 ^ i) & 0xFF;
        fvar3 = fvar3 * 1.01f;
        dvar2 = dvar2 * 1.001;
        svar1 = (svar1 + 1) & 0x7FFF;
        svar2 = (svar2 - 1) & 0x7FFF;
        lvar1 = lvar1 + i;
        lvar2 = lvar2 - i;
    }
    
    /* Final computation to prevent dead code elimination */
    int final_sum = checksum;
    for (int i = 0; i < 1024; i++) {
        final_sum += arr1[i] + arr2[i] + arr3[i];
        final_sum += (int)farr1[i] + (int)farr2[i];
    }
    for (int i = 0; i < 512; i++) {
        final_sum += (int)darr1[i];
    }
    
    printf("Result: %d\n", final_sum);
    return final_sum & 0xFF;
}
