/* early-remat-test.c
 * Designed to trigger uncovered lines 930-937 in early-remat.cc
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization */
volatile int global_sink;

/* Non-inline function to clobber caller-saved registers */
__attribute__((noinline))
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
    /* Declare many arrays to increase register pressure */
    int arr1[1024], arr2[1024], arr3[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farr1[i] = i * 3.14f;
        farr2[i] = i * 2.71f;
        darr1[i] = i * 1.618;
        sarr1[i] = i & 0xFF;
        carr1[i] = i & 0x7F;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    short s1 = 100, s2 = 200, s3 = 300;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.111, d2 = 2.222;
    
    int checksum = 0;
    
    /* High-pressure loop with many live variables */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];           /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];    /* More complex addressing */
        int *ptr3 = &arr3[(i * 3) & 0x3FF];
        float *fptr1 = &farr1[i];
        float *fptr2 = &farr2[i & 0x1F];
        double *dptr1 = &darr1[i];
        short *sptr1 = &sarr1[i];
        char *cptr1 = &carr1[i];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);      /* Multiplication */
        int offset2 = (i << 2) + 16;        /* Shift and add */
        float foffset = i * 0.5f;
        double doffset = i * 0.25;
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            v1 = v2 + v3;
            f1 = f2 + f3;
            d1 = d2 * 1.5;
        } else if (i % 13 == 0) {
            v4 = v5 - v6;
            f1 = f2 - f3;
            d1 = d2 / 2.0;
        }
        
        /* Update many variables to keep them live */
        v1 += i;
        v2 += v1;
        v3 += v2;
        v4 += v3;
        v5 += v4;
        v6 += v5;
        v7 += v6;
        v8 += v7;
        v9 += v8;
        v10 += v9;
        
        s1 += i & 0xFF;
        s2 += s1;
        s3 += s2;
        
        f1 += i * 0.1f;
        f2 += f1;
        f3 += f2;
        
        d1 += i * 0.01;
        d2 += d1;
        
        /* Function call clobbers caller-saved registers */
        use_values(v1, v2, s1, f1, d1, 
                  ptr1, ptr2, fptr1, dptr1, cptr1);
        
        /* Use computed values after call (must be rematerialized or saved) */
        arr1[i] += offset1 + v1;
        arr2[i & 0x3F] += offset2 + v2;
        farr1[i] += foffset + f1;
        darr1[i] += doffset + d1;
        
        /* More conditional code to complicate control flow */
        if (i % 11 == 0) {
            *ptr3 = v3 + v4;
            *fptr2 = f2 + f3;
        }
        
        /* Update checksum to prevent dead code elimination */
        checksum += arr1[i] + arr2[i & 0x3F] + (int)farr1[i] + (int)darr1[i];
    }
    
    /* Final computation to use all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
               + s1 + s2 + s3
               + (int)f1 + (int)f2 + (int)f3
               + (int)d1 + (int)d2
               + checksum;
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
