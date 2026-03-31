/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and exercise the uncovered block in early-remat.cc lines 930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *ptr1, int *ptr2, 
                 float f1, float f2, double d1, short s1)
{
    /* Simple operations that use all arguments */
    global_volatile = a + b;
    if (ptr1) *ptr1 += (int)f1;
    if (ptr2) *ptr2 += (int)d1 + s1;
    
    /* Additional computation to ensure function isn't trivial */
    for (int i = 0; i < 3; i++) {
        global_volatile += i;
    }
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    int arr3[1024];
    float farray1[1024];
    float farray2[1024];
    double darray[1024];
    short sarray[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farray1[i] = i * 0.5f;
        farray2[i] = i * 0.25f;
        darray[i] = i * 0.125;
        sarray[i] = (short)(i % 256);
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    float fv1 = 1.1f, fv2 = 2.2f, fv3 = 3.3f;
    double dv1 = 4.4, dv2 = 5.5;
    short sv1 = 11, sv2 = 12;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Complex conditional to prevent hoisting */
        if (i % 7 == 0) {
            v1 += i;
            fv1 += i * 0.1f;
        } else if (i % 13 == 0) {
            v2 += i;
            fv2 += i * 0.2f;
        }
        
        /* Expensive-to-spill address computations */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        int *ptr3 = &arr3[(i * 3) % 1024];       /* Scaled index */
        float *fptr1 = &farray1[i];
        float *fptr2 = &farray2[(i + 5) % 1024];
        double *dptr = &darray[(i * 2) % 1024];
        short *sptr = &sarray[(i + 3) % 1024];
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Scaled offset */
        int offset2 = (i & 0xFF) * 4;            /* Masked and scaled */
        float foffset = i * 0.5f;
        double doffset = i * 0.25;
        
        /* Update many variables to keep them live */
        v3 = v1 + v2;
        v4 = v2 * 3;
        v5 = v3 - v4;
        v6 = offset1 / 4;
        v7 = offset2 + i;
        v8 = (int)(fv1 * 10.0f);
        v9 = (int)(dv1 * 20.0);
        v10 = sv1 + sv2;
        
        fv3 = fv1 + fv2 + foffset;
        dv2 = dv1 + doffset;
        sv1 = (short)(i % 128);
        sv2 = (short)((i + 1) % 128);
        
        /* Function call that clobbers caller-saved registers */
        helper_func(v1, v2, ptr1, ptr2, fv1, fv2, dv1, sv1);
        
        /* Use computed values after the call - forces them to be live across call */
        *ptr3 = v3 + v4 + v5;
        *fptr1 = fv3 * 2.0f;
        *fptr2 = fv3 * 3.0f;
        *dptr = dv2 * 1.5;
        *sptr = (short)(sv1 + sv2);
        
        /* More computations using results */
        arr1[i] += v6 + v7;
        arr2[i & 0x3F] += v8 + v9;
        arr3[(i * 3) % 1024] += v10;
        
        /* Conditional store based on runtime value */
        if (i % 17 == 0) {
            farray1[i] += fv1;
            darray[i] += dv1;
        }
        
        /* Update variables for next iteration */
        v1 = (v1 + 1) % 100;
        v2 = (v2 + 2) % 100;
        fv1 = fv1 * 1.01f;
        fv2 = fv2 * 0.99f;
        dv1 = dv1 * 1.001;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += arr3[i];
        checksum += (unsigned long long)farray1[i];
        checksum += (unsigned long long)farray2[i];
        checksum += (unsigned long long)darray[i];
        checksum += sarray[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
