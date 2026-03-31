/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend.
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, short c, float d, double e, 
                int *p1, int *p2, float *p3, double *p4, char *p5) {
    /* Complex enough to use many registers, but not optimized away */
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
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.5;
        sarr1[i] = i & 0xFF;
        carr1[i] = i & 0x7F;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    short s1 = 10, s2 = 20, s3 = 30;
    float f1 = 100.0f, f2 = 200.0f, f3 = 300.0f;
    double d1 = 1000.0, d2 = 2000.0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            v1 = i * 2;  /* This computation is cheap to rematerialize */
        }
        
        /* Expensive-to-spill address computations (cheap to recompute) */
        int *ptr1 = &arr1[i];           /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];    /* More complex addressing */
        float *ptr3 = &farr1[i];
        double *ptr4 = &darr1[i];
        char *ptr5 = &carr1[i];
        
        /* More cheap computations that could be rematerialized */
        int offset1 = i * 4;            /* Multiplication by constant */
        int offset2 = i << 2;           /* Shift alternative */
        float foffset = i * 1.5f;
        double doffset = i * 3.5;
        
        /* Use all variables to keep them live */
        v2 = v1 + offset1;
        v3 = v2 + offset2;
        v4 = v3 + (int)foffset;
        v5 = v4 + (int)doffset;
        s1 = (short)(v5 & 0xFF);
        f1 = foffset * 2.0f;
        f2 = f1 + (float)offset1;
        d1 = doffset * 2.0;
        d2 = d1 + (double)offset2;
        
        /* Function call that clobbers caller-saved registers */
        use_values(v1, v2, s1, f1, d1, 
                  ptr1, ptr2, ptr3, ptr4, ptr5);
        
        /* Use values after call - they need to be preserved/rematerialized */
        arr3[i] = v3 + v4 + v5;
        farr2[i] = f2 + (float)v6;
        sarr1[i] = s1 + s2 + s3;
        
        /* More conditional code to prevent optimization */
        if (i % 13 == 0) {
            v6 = i * 3;
            s2 = (short)(i & 0x7F);
            f3 = i * 7.0f;
        }
        
        /* Cross-type operations to use different machine modes */
        darr1[i] = d1 + d2 + (double)f3;
        carr1[i] = (char)((v1 + v2 + v3 + v4 + v5) & 0x7F);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
        checksum += (unsigned long long)farr1[i];
        checksum += (unsigned long long)farr2[i];
        checksum += (unsigned long long)darr1[i];
        checksum += sarr1[i];
        checksum += carr1[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
