/* Program to trigger early rematerialization in GCC RTL backend */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to force register pressure around calls */
__attribute__((noinline, noipa))
void use_values(int a, int b, int c, int d, 
                float e, float f, double g, double h,
                int* p1, int* p2, short* p3, char* p4) {
    /* Use volatile to prevent optimization */
    volatile int sink;
    sink = a + b + c + d;
    sink = (int)(e + f + g + h);
    sink = *p1 + *p2 + *p3 + *p4;
    (void)sink;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_offset(int base, int scale, int mask) {
    volatile int result = (base * scale) & mask;
    return result;
}

int main(void) {
    /* Declare many arrays to work with */
    int arr1[1024], arr2[1024], arr3[1024];
    short arr4[1024];
    char arr5[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = i * 2;
        arr4[i] = (short)(i % 256);
        arr5[i] = (char)(i % 128);
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22;
    short s1 = 10, s2 = 20;
    char c1 = 'a', c2 = 'b';
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            v1 = i * 2;      /* Expensive to spill, cheap to recompute */
            f1 = i * 1.5f;
            d1 = i * 3.14159;
        } else if (i % 13 == 0) {
            v2 = i * 3;
            f2 = i * 2.5f;
            d2 = i * 6.28318;
        }
        
        /* Compute addresses - these are rematerialization candidates */
        int* ptr1 = &arr1[i];                /* Base + offset */
        int* ptr2 = &arr2[i & 0x3F];         /* Base + (i & mask) */
        short* ptr3 = &arr4[(i * 3) % 1024]; /* More complex index */
        char* ptr4 = &arr5[(i * 5) % 1024];
        
        /* More address computations with different scales */
        float* fptr1 = &farr1[i];
        float* fptr2 = &farr2[(i + 1) % 1024];
        double* dptr1 = &darr1[(i * 2) % 1024];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);       /* Multiplication by constant */
        int offset2 = (i << 2) + 16;         /* Shift and add */
        int offset3 = compute_offset(i, 4, 255); /* Function call result */
        
        /* Use all computed values before call to keep them live */
        v3 = *ptr1 + offset1;
        v4 = *ptr2 + offset2;
        s1 = (short)(*ptr3 + offset3);
        c1 = (char)(*ptr4 + (i % 64));
        f3 = *fptr1 + (float)offset1;
        f4 = *fptr2 + (float)offset2;
        
        /* Function call clobbers caller-saved registers */
        use_values(v1, v2, v3, v4, 
                  f1, f2, d1, d2,
                  ptr1, ptr2, ptr3, ptr4);
        
        /* Use values after call - they need to be preserved/rematerialized */
        arr3[i] = v1 + v2 + v3 + v4;
        farr1[i] = f1 + f2 + f3 + f4;
        darr1[i] = d1 + d2 + (double)(s1 + c1);
        
        /* More computations using the same addresses */
        if (i % 3 == 0) {
            *ptr1 = *ptr1 + v5;
            *ptr2 = *ptr2 + v6;
            *ptr3 = (short)(*ptr3 + v7);
            *ptr4 = (char)(*ptr4 + v8);
        }
        
        /* Update scalars to create dependencies */
        v5 = v5 + i;
        v6 = v6 + (i % 31);
        v7 = v7 + (i % 127);
        v8 = v8 + (i % 255);
        f1 = f1 + 0.1f;
        f2 = f2 + 0.2f;
        d1 = d1 + 0.01;
        d2 = d2 + 0.02;
    }
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i] + arr5[i];
        checksum += (int)farr1[i] + (int)farr2[i] + (int)darr1[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
