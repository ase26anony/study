/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and exercise the uncovered code block in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, int *h, float *i, double *j) {
    /* Complex enough to use many registers */
    global_volatile = a + b + *c + (int)d + (int)e + f + g;
    if (h) *h = a * 2;
    if (i) *i = d * 2.0f;
    if (j) *j = e * 2.0;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Declare many arrays to create register pressure */
    int arr1[1024], arr2[1024], arr3[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farr1[i] = i * 0.5f;
        farr2[i] = i * 1.5f;
        darr1[i] = i * 0.25;
        sarr1[i] = i & 0x7FFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float fv1 = 1.1f, fv2 = 2.2f, fv3 = 3.3f;
    double dv1 = 1.11, dv2 = 2.22;
    short sv1 = 10, sv2 = 20;
    long lv1 = 100, lv2 = 200;
    
    /* Pointer variables */
    int *p1, *p2, *p3;
    float *fp1, *fp2;
    double *dp1;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent loop invariant hoisting with conditional */
        if (i % 7 == 0) {
            v1 = i * 2;      /* Expensive to spill, cheap to recompute */
            fv1 = i * 0.7f;
        } else if (i % 5 == 0) {
            v2 = i * 3;
            fv2 = i * 1.7f;
        }
        
        /* Compute addresses - these are rematerialization candidates */
        p1 = &arr1[i];               /* Base + offset */
        p2 = &arr2[i & 0x3F];        /* More complex addressing */
        p3 = &arr3[(i * 3) % 1024];  /* Even more complex */
        
        fp1 = &farr1[i];
        fp2 = &farr2[(i * 2) % 1024];
        dp1 = &darr1[(i * 5) % 1024];
        
        /* More computations that could be rematerialized */
        int offset1 = i * sizeof(int);      /* i * 4 */
        int offset2 = (i << 2) + 1;         /* i * 4 + 1 */
        float foffset = i * 0.125f;
        double doffset = i * 0.0625;
        
        /* Use all variables to keep them live */
        v3 = v1 + v2 + v4 + v5;
        fv3 = fv1 + fv2;
        dv2 = dv1 + (double)fv3;
        sv2 = (short)(sv1 + i);
        lv2 = lv1 + i;
        
        /* Call helper function - clobbers caller-saved registers */
        helper_func(v1, v2, p1, fv1, dv1, sv1, lv1, p2, fp1, dp1);
        
        /* Use values after call - forces save/restore or rematerialization */
        *p1 = v3 + *p2;
        *fp1 = fv3 + *fp2;
        *dp1 = dv2 + *dp1;
        
        /* More computations using the addresses */
        arr1[(i + 1) % 1024] = *p1 + offset1;
        farr1[(i + 2) % 1024] = *fp1 + foffset;
        darr1[(i + 3) % 1024] = *dp1 + doffset;
        
        /* Conditional store to prevent optimization */
        if (i % 13 == 0) {
            sarr1[i] = sv2;
            arr2[i] = offset2;
        }
        
        /* Update some variables for next iteration */
        v4 = v4 + i;
        v5 = v5 - (i % 3);
        fv2 = fv2 * 1.01f;
        dv1 = dv1 * 1.001;
        sv1 = sv1 + 1;
        lv1 = lv1 + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)farr1[i] + 
                   (int)darr1[i] + sarr1[i];
    }
    
    /* Use checksum so it can't be optimized away */
    printf("Checksum: %ld\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
