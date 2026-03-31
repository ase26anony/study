/* early_remat_test.c
 * Designed to trigger early rematerialization in GCC RTL backend.
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -S early_remat_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent call elimination */
volatile int global_sink;

/* Non-inline function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_registers(int a, int b, int c, float d, double e, 
                   short f, int *p1, int *p2, float *p3, double *p4) {
    /* Complex enough to use many registers */
    global_sink = a + b + c + (int)d + (int)e + f;
    if (p1) *p1 += a;
    if (p2) *p2 += b;
    if (p3) *p3 += d;
    if (p4) *p4 += e;
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
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
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
        sarr1[i] = i & 0x7FFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.111, d2 = 2.222;
    short s1 = 11, s2 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            v1 = i * 2;  /* Force recomputation */
        }
        
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];           /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];    /* Base + masked index */
        int *ptr3 = &arr3[i % 128];     /* Base + modulo index */
        float *ptr4 = &farr1[i * 2];    /* Base + scaled index */
        float *ptr5 = &farr2[i / 2];    /* Base + shifted index */
        double *ptr6 = &darr1[1023 - i]; /* Base + reversed index */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);      /* Scaled offset */
        int offset2 = (i << 2) + 1;         /* Shifted offset with constant */
        float foffset = i * 0.25f;          /* Float offset */
        double doffset = i * 0.125;         /* Double offset */
        
        /* Use all variables to keep them live */
        v2 = v1 + offset1;
        v3 = v2 * 2 + offset2;
        v4 = v3 - i;
        v5 = v4 + (int)foffset;
        v6 = v5 + (int)doffset;
        f1 = foffset * 2.0f;
        f2 = f1 + i * 0.5f;
        d1 = doffset * 1.5;
        d2 = d1 + i * 0.25;
        s1 = (short)(i & 0xFF);
        s2 = s1 + 1;
        
        /* Function call that clobbers caller-saved registers */
        use_registers(v1, v2, v3, f1, d1, s1, 
                     ptr1, ptr2, &f1, &d1);
        
        /* Use results after call (forces save/restore or rematerialization) */
        *ptr1 += v4;
        *ptr2 += v5;
        *ptr3 += v6;
        *ptr4 += f2;
        *ptr5 += f2 * 0.5f;
        *ptr6 += d2;
        
        /* More computations using the same values */
        v7 = v4 + v5 + v6;
        v8 = v7 * 2 - i;
        v9 = v8 + (int)(*ptr4);
        v10 = v9 + (int)(*ptr6);
        
        /* Conditional store to prevent optimization */
        if (i % 13 == 0) {
            arr1[i] = v7;
            arr2[i] = v8;
            arr3[i] = v9;
            farr1[i] = f2;
            darr1[i] = d2;
        }
        
        /* Another call with different arguments */
        use_registers(v7, v8, v9, f2, d2, s2,
                     ptr3, ptr4, &f2, &d2);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)farr1[i] + (int)darr1[i];
        checksum &= 0xFFFF;
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum;
}
