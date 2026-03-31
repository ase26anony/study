/* early_remat_coverage.c
 * Designed to trigger early rematerialization in GCC's RTL backend
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile_sink;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, int c, float d, double e, 
                short f, int *p1, int *p2, float *p3, double *p4) {
    /* Complex enough to prevent inlining, uses all arguments */
    global_volatile_sink = a + b + c + (int)d + (int)e + f;
    if (p1) global_volatile_sink += *p1;
    if (p2) global_volatile_sink += *p2;
    if (p3) global_volatile_sink += (int)*p3;
    if (p4) global_volatile_sink += (int)*p4;
    
    /* Memory clobber to force register reloads */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Large arrays to work with */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        darr1[i] = i * 2.5;
        sarr1[i] = i & 0xFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 10.1, dvar2 = 20.2;
    short svar1 = 100, svar2 = 200;
    int *ptr1, *ptr2, *ptr3;
    float *fptr1, *fptr2;
    double *dptr1;
    
    /* Result accumulator */
    int checksum = 0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *addr1 = &arr1[i];                    /* Base + index */
        int *addr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float *addr3 = &farr1[(i * 3) % 1024];    /* More complex index */
        double *addr4 = &darr1[(i * 5) % 1024];
        short *addr5 = &sarr1[(i * 7) % 1024];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);            /* Loop invariant pattern */
        int offset2 = (i % 16) * 4;               /* Varying offset */
        float foffset = i * 0.25f;
        double doffset = i * 0.125;
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            addr1 = &arr1[(i + 1) % 1024];
            offset1 += sizeof(int);
        }
        
        if (i % 13 == 0) {
            addr3 = &farr1[(i + 2) % 1024];
            foffset += 1.0f;
        }
        
        /* Use all variables to keep them live */
        var1 = offset1 + var2;
        var3 = offset2 + var4;
        fvar1 = foffset + fvar2;
        dvar1 = doffset + dvar2;
        svar1 = (i & 0xFF) + svar2;
        
        /* Function call that clobbers caller-saved registers */
        use_values(var1, var2, var3, fvar1, dvar1,
                  svar1, addr1, addr2, addr3, addr4);
        
        /* More computations after call (values must be rematerialized) */
        ptr1 = addr1 + (offset1 / sizeof(int));
        ptr2 = addr2 + (offset2 / sizeof(int));
        fptr1 = addr3 + (int)foffset;
        dptr1 = addr4 + (int)doffset;
        
        /* Conditional stores using rematerialized values */
        if (i % 3 == 0) {
            *ptr1 = var1 + var2;
            checksum += *ptr1;
        }
        
        if (i % 5 == 0) {
            *ptr2 = var3 + var4;
            checksum += *ptr2;
        }
        
        if (i % 11 == 0) {
            *fptr1 = fvar1 + fvar2;
            checksum += (int)*fptr1;
        }
        
        if (i % 17 == 0) {
            *dptr1 = dvar1 + dvar2;
            checksum += (int)*dptr1;
        }
        
        /* Update some variables for next iteration */
        var2 = (var2 + 1) % 100;
        var4 = (var4 + 2) % 100;
        fvar2 = fvar2 + 0.5f;
        dvar2 = dvar2 + 0.25;
        svar2 = (svar2 + 1) % 256;
    }
    
    /* Final computation to prevent dead code elimination */
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)darr1[i] + sarr1[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
