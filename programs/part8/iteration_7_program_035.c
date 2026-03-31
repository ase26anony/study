/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, char *h, int i, float j) {
    /* Perform operations that use all arguments */
    global_volatile = a + b + *c + (int)d + (int)e + f + (int)g + (int)*h + i + (int)j;
    
    /* Modify pointer argument */
    if (c != NULL) {
        *c += a + b;
    }
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[512];
    short sarr1[2048];
    char carr1[4096];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        if (i < 512) darr1[i] = i * 4.2;
        if (i < 2048) sarr1[i] = i & 0xFF;
        if (i < 4096) carr1[i] = (i % 128) + 1;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 12345;
    int var2 = 67890;
    int var3 = 13579;
    int var4 = 24680;
    float fvar1 = 3.14159f;
    float fvar2 = 2.71828f;
    double dvar1 = 1.41421356;
    short svar1 = 32000;
    short svar2 = -32000;
    long lvar1 = 999999999L;
    long lvar2 = 888888888L;
    char cvar1 = 'A';
    char cvar2 = 'Z';
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        float *fptr1 = &farr1[i * 2 % 1024];     /* Non-linear computation */
        float *fptr2 = &farr2[(i + 1) % 1024];   /* Offset computation */
        double *dptr1 = &darr1[i % 512];         /* Modulo computation */
        short *sptr1 = &sarr1[i * 3 % 2048];     /* Complex index */
        char *cptr1 = &carr1[i * 4 % 4096];      /* Another complex index */
        
        /* More computations that are cheap to recompute */
        int idx1 = i * 4;                        /* Simple multiplication */
        int idx2 = i + (i << 2);                 /* Shift and add */
        int idx3 = (i % 16) * 64;                /* Modulo and multiply */
        float fidx1 = i * 1.234f;                /* Float multiplication */
        double didx1 = i * 5.678;                /* Double multiplication */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 += i;
            fvar1 += i * 0.1f;
            dvar1 += i * 0.01;
        } else if (i % 13 == 0) {
            var2 -= i;
            fvar2 -= i * 0.2f;
        } else {
            var3 ^= i;
            var4 |= i;
        }
        
        /* Function call with many arguments - clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, fvar1, dvar1, 
                   svar1, lvar1, cptr1, idx1, fidx1);
        
        /* Use computed values after call (must be rematerialized or saved) */
        arr1[i] = *ptr1 + idx2 + (int)fidx1;
        arr2[i & 0x3F] = *ptr2 + idx3;
        farr1[i * 2 % 1024] = *fptr1 + fidx1;
        farr2[(i + 1) % 1024] = *fptr2 + (float)idx1;
        
        /* More operations keeping values live */
        if (i % 3 == 0) {
            darr1[i % 512] = *dptr1 + didx1;
            sarr1[i * 3 % 2048] = *sptr1 + (short)idx2;
            carr1[i * 4 % 4096] = *cptr1 + (char)idx3;
        }
        
        /* Update scalars to keep them live across iterations */
        var1 += arr1[i];
        var2 += arr2[i & 0x3F];
        var3 ^= (int)farr1[i * 2 % 1024];
        var4 |= (int)farr2[(i + 1) % 1024];
        fvar1 += farr1[i * 2 % 1024] * 0.5f;
        fvar2 += farr2[(i + 1) % 1024] * 0.25f;
        dvar1 += darr1[i % 512];
        svar1 += sarr1[i * 3 % 2048];
        svar2 -= sarr1[i * 3 % 2048];
        lvar1 += arr1[i];
        lvar2 += arr2[i & 0x3F];
        cvar1 ^= carr1[i * 4 % 4096];
        cvar2 |= carr1[i * 4 % 4096];
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += (unsigned int)farr1[i];
        checksum += (unsigned int)farr2[i];
        if (i < 512) checksum += (unsigned long long)darr1[i];
        if (i < 2048) checksum += sarr1[i];
        if (i < 4096) checksum += carr1[i];
    }
    
    checksum += var1 + var2 + var3 + var4;
    checksum += (unsigned int)fvar1 + (unsigned int)fvar2;
    checksum += (unsigned long long)dvar1;
    checksum += svar1 + svar2;
    checksum += lvar1 + lvar2;
    checksum += cvar1 + cvar2;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
