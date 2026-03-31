/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend.
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -o test early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent call elimination */
volatile int global_sink;

/* Non-inline function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, int c, float d, double e, 
                short f, int *p1, int *p2, float *p3, double *p4) {
    global_sink = a + b + c + (int)d + (int)e + f;
    if (p1) *p1 += a;
    if (p2) *p2 += b;
    if (p3) *p3 += d;
    if (p4) *p4 += e;
}

int main(void) {
    /* Declare arrays to work with */
    int arr1[1024];
    int arr2[1024];
    float farray[1024];
    double darray[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        farray[i] = i * 0.5f;
        darray[i] = i * 0.25;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.11, d2 = 2.22;
    short s1 = 10, s2 = 20;
    int *base1 = arr1, *base2 = arr2;
    
    /* Loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent invariant hoisting with condition */
        if (i % 7 == 0) {
            v1 = i & 0xF;
            f1 = (i % 13) * 0.1f;
        }
        
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* &arr1[i] - cheap to recompute */
        int *ptr2 = &arr2[i & 0x3F];             /* &arr2[i & 0x3F] - cheap to recompute */
        float *ptr3 = &farray[(i * 3) % 1024];   /* Complex addressing */
        double *ptr4 = &darray[(i * 5) % 1024];  /* More complex addressing */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* i * 4 */
        int offset2 = (i + 1) * sizeof(float);   /* (i+1) * 4 */
        int scaled_index = i << 2;               /* i * 4 via shift */
        
        /* Use all variables to keep them live */
        v2 = v1 + offset1;
        v3 = v2 + scaled_index;
        v4 = v3 + (int)f1;
        v5 = v4 + (int)d1;
        f2 = f1 + (float)offset2;
        f3 = f2 + (float)scaled_index;
        d2 = d1 + (double)offset1;
        s2 = s1 + (short)(i & 0xFF);
        
        /* Function call clobbers caller-saved registers */
        use_values(v1, v2, v3, f1, d1, s1, ptr1, ptr2, ptr3, ptr4);
        
        /* Use values after call - forces save/restore or rematerialization */
        arr1[i] += v4 + v5;
        arr2[i & 0x3F] += (int)f2 + (int)f3;
        farray[i] += f2 + f3;
        darray[i] += d1 + d2;
        
        /* More computations using the same cheap values */
        int *ptr1_again = &arr1[i];              /* Same as ptr1 - candidate for remat */
        int *ptr2_again = &arr2[i & 0x3F];       /* Same as ptr2 - candidate for remat */
        
        /* Use them again */
        *ptr1_again += s2;
        *ptr2_again += (int)(f3 * 2.0f);
        
        /* Conditional to prevent optimization */
        if (i % 11 == 0) {
            base1 = ptr1;
            base2 = ptr2;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (unsigned)farray[i] + (unsigned)darray[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
