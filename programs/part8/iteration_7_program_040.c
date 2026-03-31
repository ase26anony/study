/* Program to trigger early rematerialization in GCC RTL backend */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, int c, int d, float e, float f, 
                double g, double h, short i, short j, int *ptr1, int *ptr2) {
    /* Complex enough to use many registers */
    global_sink = a + b + c + d;
    global_sink += (int)e + (int)f;
    global_sink += (int)g + (int)h;
    global_sink += i + j;
    if (ptr1) global_sink += *ptr1;
    if (ptr2) global_sink += *ptr2;
}

int main(void) {
    /* Large arrays to work with */
    int arr1[1024], arr2[1024], arr3[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = 0;
        farr1[i] = i * 0.5f;
        farr2[i] = i * 0.25f;
        darr1[i] = i * 0.125;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with conditional */
        if (i % 7 == 0) {
            v1 = i * 2;
            f1 = i * 0.7f;
            d1 = i * 0.9;
        }
        
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        int *ptr3 = &arr3[(i * 3) % 1024];       /* Complex index */
        float *fptr1 = &farr1[i];
        float *fptr2 = &farr2[(i + 5) % 1024];
        double *dptr1 = &darr1[i];
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication */
        int offset2 = (i << 2) + 16;             /* Shift and add */
        float foffset = i * 0.33f;
        double doffset = i * 0.77;
        
        /* Update many live variables to keep them active */
        v2 = v1 + i;
        v3 = v2 * 2;
        v4 = v3 - i;
        v5 = v4 + v1;
        v6 = v5 & 0xFF;
        v7 = v6 | 0x80;
        v8 = v7 ^ i;
        
        f2 = f1 + i * 0.1f;
        f3 = f2 * 2.0f;
        f4 = f3 - i * 0.05f;
        
        d2 = d1 + i * 0.01;
        
        s2 = s1 + (i & 0xF);
        s3 = s2 * 2;
        s4 = s3 - (i & 0x7);
        
        /* Function call that clobbers caller-saved registers */
        use_values(v1, v2, v3, v4, f1, f2, d1, d2, s1, s2, ptr1, ptr2);
        
        /* Use computed values after call (forces them to be live across call) */
        *ptr3 = v5 + v6 + v7 + v8;
        *fptr1 = f3 + f4;
        *fptr2 = f3 - f4;
        *dptr1 = d2 * 2.0;
        
        /* More computations using the offsets */
        arr1[i] += offset1;
        arr2[i & 0x3F] += offset2;
        farr1[i] += foffset;
        darr1[i] += doffset;
        
        /* Conditional store based on runtime value */
        if (i % 13 == 0) {
            arr3[i] = v8 + s4;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
        checksum += (int)farr1[i] + (int)farr2[i];
        checksum += (int)darr1[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
