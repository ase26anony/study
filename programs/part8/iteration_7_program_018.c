/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, int *h, float *i, double *j) {
    /* Complex enough to use many registers */
    global_volatile = a + b + *c + (int)d + (int)e + f + g;
    if (h) *h = a * 2;
    if (i) *i = d * 2.0f;
    if (j) *j = e * 2.0;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    double darr2[1024];
    short sarr1[1024];
    long larr1[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        farr1[i] = i * 1.5f;
        farr2[i] = (1024 - i) * 1.5f;
        darr1[i] = i * 2.5;
        darr2[i] = (1024 - i) * 2.5;
        sarr1[i] = i & 0x7FFF;
        larr1[i] = i * 1000L;
    }
    
    int checksum = 0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Declare many local variables to increase register pressure */
        int v1 = arr1[i];
        int v2 = arr2[i];
        float f1 = farr1[i];
        float f2 = farr2[i];
        double d1 = darr1[i];
        double d2 = darr2[i];
        short s1 = sarr1[i];
        long l1 = larr1[i];
        
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];
        int *ptr2 = &arr2[i & 0x3F];  /* Non-linear access pattern */
        float *fptr1 = &farr1[i];
        float *fptr2 = &farr2[(i * 3) & 0x3FF];
        double *dptr1 = &darr1[i];
        double *dptr2 = &darr2[(i * 5) & 0x3FF];
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);
        int offset2 = (i & 0x1F) * 4;
        float foffset = i * 1.5f;
        double doffset = i * 2.5;
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            v1 = v1 * 2;
            f1 = f1 * 2.0f;
            d1 = d1 * 2.0;
        } else if (i % 13 == 0) {
            v2 = v2 / 2;
            f2 = f2 / 2.0f;
            d2 = d2 / 2.0;
        }
        
        /* Function call that clobbers caller-saved registers */
        helper_func(v1, v2, ptr1, f1, d1, s1, l1, ptr2, fptr1, dptr1);
        
        /* Use values after call (must be kept live or rematerialized) */
        int result1 = v1 + *ptr1 + offset1;
        float result2 = f1 + *fptr1 + foffset;
        double result3 = d1 + *dptr1 + doffset;
        
        /* More conditional code to prevent optimization */
        if (i % 3 == 0) {
            arr1[i] = result1;
            farr1[i] = result2;
            darr1[i] = result3;
        } else if (i % 5 == 0) {
            arr2[i & 0x3F] = result1 + v2;
            farr2[(i * 3) & 0x3FF] = result2 + f2;
            darr2[(i * 5) & 0x3FF] = result3 + d2;
        }
        
        /* Update checksum to prevent dead code elimination */
        checksum += result1 + (int)result2 + (int)result3;
        checksum += s1 + (int)l1;
        
        /* Another conditional with different pattern */
        if ((i & 0xF) == 0) {
            helper_func(v2, v1, ptr2, f2, d2, (short)(s1 ^ 0xFF), 
                       l1 >> 1, ptr1, fptr2, dptr2);
        }
    }
    
    /* Final computation to use all arrays */
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
        checksum += (int)darr1[i] + (int)darr2[i] + sarr1[i] + (int)larr1[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
