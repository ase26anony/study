/* Program to trigger early rematerialization in GCC RTL backend */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, int c, float d, double e, 
                short f, int *p1, int *p2, float *p3, double *p4) {
    /* Force register pressure by using all arguments */
    global_sink = a + b + c + (int)d + (int)e + f;
    if (p1) global_sink += *p1;
    if (p2) global_sink += *p2;
    if (p3) global_sink += (int)*p3;
    if (p4) global_sink += (int)*p4;
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
        int *ptr3 = &arr3[i * 2];       /* Base + scaled index */
        float *ptr4 = &farr1[i];        /* Different mode */
        double *ptr5 = &darr1[i];       /* Different mode */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * 4;            /* Simple arithmetic */
        int offset2 = (i + 1) * 4;      /* Another computation */
        float foffset = i * 1.5f;       /* Float computation */
        double doffset = i * 3.14159;   /* Double computation */
        
        /* Use all variables to keep them live */
        v2 = v1 + offset1;
        v3 = v2 + offset2;
        v4 = v3 + (int)foffset;
        v5 = v4 + (int)doffset;
        f1 = foffset * 2.0f;
        f2 = f1 + (float)offset1;
        d1 = doffset * 2.0;
        d2 = d1 + (double)offset2;
        s1 = (short)(i & 0xFF);
        s2 = s1 + 1;
        
        /* Function call clobbers caller-saved registers */
        use_values(v1, v2, v3, f1, d1, s1, 
                  ptr1, ptr2, ptr4, ptr5);
        
        /* Use values after call (forces save/restore or rematerialization) */
        arr1[i] = v4 + v5;
        arr2[i & 0x3F] = v6 + (int)f2;
        arr3[i * 2] = v7 + (int)d2;
        farr1[i] = f2 + (float)v8;
        farr2[i] = (float)v9 + (float)v10;
        darr1[i] = d2 + (double)v1;
        sarr1[i] = s2;
        
        /* Modify scalars to prevent optimization */
        v6 = v5 + 1;
        v7 = v6 + 2;
        v8 = v7 + 3;
        v9 = v8 + 4;
        v10 = v9 + 5;
        
        /* Another conditional to prevent invariant motion */
        if (i % 13 == 0) {
            /* Force different computation path */
            ptr1 = &arr2[i];
            ptr2 = &arr1[i & 0x3F];
            offset1 = i * 8;  /* Different computation */
        }
        
        /* Second function call with different arguments */
        use_values(v6, v7, v8, f2, d2, s2,
                  ptr1, ptr2, &farr2[i], &darr1[i]);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + 
                   (int)farr1[i] + (int)farr2[i] + 
                   (int)darr1[i] + sarr1[i];
        /* Prevent loop optimization */
        if (checksum % 1000 == 0) {
            checksum ^= i;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0;
}
