/* early-remat-test.c
 * Test case for GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -S early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization */
volatile int global_sink;

/* Non-inline function to force register pressure around calls */
__attribute__((noinline, noipa))
void use_values(int a, int b, int c, int d, 
                float e, float f, double g, double h,
                int* p1, int* p2, short* p3, char* p4) {
    /* Use all arguments to prevent elimination */
    global_sink = a + b + c + d;
    global_sink += (int)(e + f + g + h);
    if (p1) global_sink += *p1;
    if (p2) global_sink += *p2;
    if (p3) global_sink += *p3;
    if (p4) global_sink += *p4;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_offset(int base, int scale, int offset) {
    return (base * scale + offset) & 0xFF;
}

int main(void) {
    /* Declare many arrays to work with */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    char arr4[1024];
    float farr1[1024];
    double darr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = (char)(i % 256);
        farr1[i] = (float)i * 0.5f;
        darr1[i] = (double)i * 0.25;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22;
    short s1 = 10, s2 = 20;
    char c1 = 'a', c2 = 'b';
    
    /* Pointer variables */
    int *p1, *p2, *p3, *p4;
    short *sp1, *sp2;
    char *cp1, *cp2;
    float *fp1;
    double *dp1;
    
    int checksum = 0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with conditional computation */
        if (i % 7 == 0) {
            v1 = i * 2;      /* Expensive to spill, cheap to recompute */
            f1 = (float)i * 0.33f;
        } else if (i % 5 == 0) {
            v2 = i * 3;
            d1 = (double)i * 0.44;
        }
        
        /* Compute addresses - these are rematerialization candidates */
        p1 = &arr1[i];                    /* Base + offset */
        p2 = &arr2[i & 0x3F];             /* More complex addressing */
        sp1 = &arr3[compute_offset(i, 2, 1)];  /* Function call in address computation */
        cp1 = &arr4[(i * 3) % 1024];
        fp1 = &farr1[(i + 1) % 1024];
        dp1 = &darr1[(i * 2) % 1024];
        
        /* More address computations */
        p3 = &arr1[(i + v1) % 1024];
        p4 = &arr2[(i + v2) % 1024];
        sp2 = &arr3[(i * 2) & 0x3FF];
        cp2 = &arr4[(i * 5) % 1024];
        
        /* Update scalars to keep them live */
        v3 = v1 + v2;
        v4 = v2 * 2;
        v5 = v3 - v1;
        v6 = v4 + i;
        v7 = v5 * 3;
        v8 = v6 / 2;
        
        f2 = f1 * 2.0f;
        f3 = f2 + (float)v1;
        f4 = f3 - 0.5f;
        
        d2 = d1 * 1.5;
        
        s1 = (short)(v1 + v2);
        s2 = (short)(v3 + v4);
        
        c1 = (char)((i + v1) % 256);
        c2 = (char)((i + v2) % 256);
        
        /* Function call that clobbers caller-saved registers */
        use_values(v1, v2, v3, v4, f1, f2, d1, d2, p1, p2, sp1, cp1);
        
        /* Use computed values after call - forcing them to be live across call */
        *p3 = v5 + v6;
        *p4 = v7 + v8;
        *sp2 = s1 + s2;
        *cp2 = c1 + c2;
        *fp1 = f3 + f4;
        *dp1 = d1 + d2;
        
        /* Conditional store based on runtime value */
        if (i % 13 == 0) {
            arr1[i] = v1 + v3 + v5 + v7;
            arr2[i] = v2 + v4 + v6 + v8;
        }
        
        /* Update checksum to prevent elimination */
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i];
    }
    
    /* Final computation to use all variables */
    int final_result = checksum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8
                     + (int)f1 + (int)f2 + (int)f3 + (int)f4
                     + (int)d1 + (int)d2 + s1 + s2 + c1 + c2;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
