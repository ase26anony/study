/* Program to trigger early rematerialization in GCC RTL backend */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to force register pressure around calls */
__attribute__((noinline, noipa))
void use_values(int a, int b, int c, int d, float e, float f, 
                double g, double h, short i, short j, int* ptr1, int* ptr2) {
    /* Use volatile to prevent optimization */
    volatile int sink;
    sink = a + b + c + d;
    sink = (int)e + (int)f;
    sink = (int)g + (int)h;
    sink = i + j;
    if (ptr1) sink = *ptr1;
    if (ptr2) sink = *ptr2;
}

int main(void) {
    /* Large arrays to work with */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 2.5f;
        darr1[i] = i * 1.75;
        sarr1[i] = i & 0xFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    
    /* Loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill address computations (remat candidates) */
        int* ptr1 = &arr1[i];                    /* Base + index */
        int* ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float* fptr1 = &farr1[(i * 3) % 1024];   /* More complex computation */
        double* dptr1 = &darr1[(i * 5) % 1024];
        short* sptr1 = &sarr1[(i * 7) % 1024];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Simple multiplication */
        int offset2 = (i % 16) * 4;              /* Masked multiplication */
        float fscale = i * 0.25f;                /* Float computation */
        double dscale = i * 0.125;               /* Double computation */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            ptr1 = &arr2[i];                     /* Different base occasionally */
            offset1 = i * sizeof(short);         /* Different computation */
        }
        
        if (i % 13 == 0) {
            fscale = i * 0.5f;
            dscale = i * 0.25;
        }
        
        /* Function call that clobbers caller-saved registers */
        use_values(v1 + i, v2 + offset1, v3 + offset2, v4 + (int)fscale,
                  f1 + fscale, f2 + i * 0.1f,
                  d1 + dscale, d2 + i * 0.05,
                  s1 + (i & 0xF), s2 + ((i >> 4) & 0xF),
                  ptr1, ptr2);
        
        /* Use computed values after call (must be kept live or rematerialized) */
        arr1[i] += *ptr1 + offset1 + (int)fscale;
        arr2[i & 0x3F] += *ptr2 + offset2;
        farr1[i] += *fptr1 + fscale;
        darr1[i] += *dptr1 + dscale;
        sarr1[i] += *sptr1 + (i & 0xF);
        
        /* Update scalars to keep them live */
        v1 += arr1[i] & 1;
        v2 += arr2[i] & 1;
        f1 += (float)(arr1[i] % 100) * 0.01f;
        d1 += (double)(arr2[i] % 100) * 0.01;
        s1 += sarr1[i] & 1;
        
        /* More conditional code to complicate control flow */
        if (i % 11 == 0) {
            v3 = v1 * 2;
            f3 = f1 * 2.0f;
            d2 = d1 * 2.0;
        }
        
        if (i % 17 == 0) {
            v4 = v2 / 2;
            f4 = f2 / 2.0f;
            s2 = s1 >> 1;
        }
        
        /* Use all variables to ensure they're live */
        v5 = v3 + v4;
        v6 = v5 * 3;
        v7 = v6 - v1;
        v8 = v7 & 0xFF;
        f4 = f3 + f4;
        s3 = s1 + s2;
        s4 = s3 * 2;
    }
    
    /* Compute checksum to prevent elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += (unsigned int)farr1[i];
        checksum += (unsigned long long)darr1[i];
        checksum += sarr1[i];
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        checksum += (unsigned int)(f1 + f2 + f3 + f4);
        checksum += (unsigned long long)(d1 + d2);
        checksum += s1 + s2 + s3 + s4;
    }
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
