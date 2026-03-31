/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and reach uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, char *h, int i, int j) {
    /* Use all arguments to prevent optimization */
    global_volatile = a + b + *c + (int)d + (int)e + f + (int)g + (int)*h + i + j;
}

/* Another non-inline helper with different signature */
__attribute__((noinline, noipa))
int compute_offset(int idx, int scale) {
    /* Complex enough to not be inlined, but simple enough to rematerialize */
    return (idx * scale + 7) & 0xFF;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    double darr1[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        farr1[i] = i * 1.5f;
        darr1[i] = i * 2.5;
        sarr1[i] = i & 0xFFFF;
        carr1[i] = i & 0xFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 1.11, dvar2 = 2.22;
    short svar1 = 10, svar2 = 20;
    long lvar1 = 100, lvar2 = 200;
    char cvar1 = 'A', cvar2 = 'B';
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            var1 = i * 2;  /* This will be live across the call */
        } else {
            var1 = i * 3;
        }
        
        /* Compute addresses - these are cheap to rematerialize */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* More complex addressing */
        float *fptr1 = &farr1[compute_offset(i, 3)];  /* Function call in address */
        double *dptr1 = &darr1[i % 256];
        short *sptr1 = &sarr1[(i * 5) & 0x3FF];
        char *cptr1 = &carr1[(i + 13) & 0x3FF];
        
        /* More computations that could be rematerialized */
        int offset1 = i * sizeof(int);           /* Simple multiplication */
        int offset2 = (i << 2) + 16;             /* Shift and add */
        float foffset = i * 0.25f;               /* Float computation */
        double doffset = i * 0.125;              /* Double computation */
        
        /* Use all computed values before call to keep them live */
        int temp1 = *ptr1 + offset1;
        int temp2 = *ptr2 + offset2;
        float ftemp = *fptr1 + foffset;
        double dtemp = *dptr1 + doffset;
        
        /* Call helper - clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, fvar1, dvar1,
                   svar1, lvar1, cptr1, temp1, temp2);
        
        /* Use values after call - forces save/restore or rematerialization */
        if (i % 13 == 0) {
            /* Conditional use to prevent optimization */
            *ptr1 = temp1 + global_volatile;
            *fptr1 = ftemp + global_volatile;
            *dptr1 = dtemp + global_volatile;
            *sptr1 = (short)(temp2 + global_volatile);
            *cptr1 = (char)((temp1 + temp2) & 0xFF);
        } else {
            /* Different computation path */
            arr1[i] = temp1 - global_volatile;
            farr1[i] = ftemp - global_volatile;
            darr1[i] = dtemp - global_volatile;
        }
        
        /* Update scalars to keep them live across iterations */
        var2 = var1 + i;
        var3 = var2 * 2;
        var4 = var3 + var1;
        var5 = var4 - i;
        fvar1 = fvar2 + i * 0.1f;
        fvar2 = fvar3 - i * 0.2f;
        fvar3 = fvar1 * 2.0f;
        dvar1 = dvar2 + i * 0.01;
        dvar2 = dvar1 * 1.5;
        svar1 = (short)(svar2 + i);
        svar2 = (short)(svar1 - i);
        lvar1 = lvar2 + i;
        lvar2 = lvar1 * 2;
        cvar1 = (char)(cvar2 + (i & 0xF));
        cvar2 = (char)(cvar1 - (i & 0x7));
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)darr1[i] + 
                   sarr1[i] + carr1[i];
    }
    
    /* Also use all scalar variables in final computation */
    checksum += var1 + var2 + var3 + var4 + var5 +
               (int)fvar1 + (int)fvar2 + (int)fvar3 +
               (int)dvar1 + (int)dvar2 +
               svar1 + svar2 + lvar1 + lvar2 + cvar1 + cvar2;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
