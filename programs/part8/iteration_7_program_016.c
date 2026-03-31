/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and reach uncovered lines in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function to create register pressure */
__attribute__((noinline, noipa))
void use_registers(int a, int b, int c, float d, double e, 
                   short f, char g, int *h, float *i, double *j) {
    /* Force register usage and prevent optimization */
    global_volatile = a + b + c + (int)d + (int)e + f + g;
    if (h) global_volatile += *h;
    if (i) global_volatile += (int)*i;
    if (j) global_volatile += (int)*j;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_value(int base, int offset, float factor, double scale) {
    /* Complex enough to not be inlined, simple enough to rematerialize */
    return (int)((base + offset) * factor * scale);
}

int main(void) {
    /* Declare many arrays to create memory pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    double darr2[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        farr1[i] = i * 0.5f;
        farr2[i] = (1024 - i) * 0.25f;
        darr1[i] = i * 0.125;
        darr2[i] = (1024 - i) * 0.0625;
        sarr1[i] = (short)(i % 256);
        carr1[i] = (char)(i % 128);
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 1.11, dvar2 = 2.22;
    short svar1 = 10, svar2 = 20;
    char cvar1 = 'a', cvar2 = 'b';
    
    /* Pointer variables that will need addressing computations */
    int *ptr1, *ptr2, *ptr3, *ptr4;
    float *fptr1, *fptr2;
    double *dptr1, *dptr2;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent loop invariant hoisting with conditional */
        if (i % 7 == 0) {
            var1 = i * 2;      /* Expensive to spill, cheap to recompute */
            fvar1 = i * 0.75f;
        } else if (i % 13 == 0) {
            var2 = i * 3;
            fvar2 = i * 1.25f;
        }
        
        /* Complex addressing computations - candidates for rematerialization */
        ptr1 = &arr1[i];                    /* &arr1[i] - cheap to recompute */
        ptr2 = &arr2[i & 0x3F];             /* &arr2[i & 0x3F] - with mask */
        ptr3 = &arr1[(i * 3) % 1024];       /* More complex addressing */
        ptr4 = &arr2[(i + var1) % 1024];    /* Depends on loop-varying var */
        
        fptr1 = &farr1[i];
        fptr2 = &farr2[(i * 2) % 1024];
        
        dptr1 = &darr1[i];
        dptr2 = &darr2[(i * 5) % 1024];
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);      /* i * 4 - cheap arithmetic */
        int offset2 = (i & 0xFF) << 2;      /* (i & 0xFF) * 4 */
        float fscale = i * 0.01f + 1.0f;
        double dscale = i * 0.001 + 1.0;
        
        /* Call non-inline function with many arguments - clobbers registers */
        use_registers(var1, var2, var3, fvar1, dvar1,
                     svar1, cvar1, ptr1, fptr1, dptr1);
        
        /* Use results in conditional stores - values must be live across call */
        if (i % 3 == 0) {
            *ptr2 = compute_value(var1, offset1, fscale, dscale);
            *fptr2 = fvar2 * fscale;
            *dptr2 = dvar2 * dscale;
        } else if (i % 5 == 0) {
            *ptr3 = compute_value(var2, offset2, fvar3, 1.5);
            arr1[i] = var4 + *ptr4;
        }
        
        /* Update variables to prevent optimization */
        var3 = var1 + var2;
        var4 = var2 * var3;
        var5 = var4 - var1;
        
        fvar3 = fvar1 + fvar2;
        dvar2 = dvar1 * 1.01;
        
        svar2 = (short)(svar1 + i);
        cvar2 = (char)(cvar1 + (i % 26));
        
        /* Another function call with different arguments */
        use_registers(var4, var5, var3, fvar3, dvar2,
                     svar2, cvar2, ptr3, fptr2, dptr2);
        
        /* More conditional computations */
        if (i % 11 == 0) {
            int idx = (i * 7) % 1024;
            arr2[idx] = compute_value(var5, i, fvar1, dvar1);
            farr1[idx] = fvar2 * 2.0f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
        checksum += (int)darr1[i] + (int)darr2[i] + sarr1[i] + carr1[i];
    }
    
    /* Mix in the global volatile to ensure all operations matter */
    checksum += global_volatile;
    
    printf("Checksum: %d\n", checksum);
    return checksum % 256;
}
