/* early_remat_trigger.c
 * Designed to trigger uncovered lines 930-937 in early-remat.cc
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early_remat_trigger.c -o early_remat_trigger
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to force register pressure around calls */
__attribute__((noinline, noipa))
void use_registers(int a, int b, int c, float d, float e, 
                   double f, short g, int *h, int *i, long j) {
    /* Use all arguments to prevent dead argument elimination */
    global_sink = a + b + c + (int)d + (int)e + (int)f + g + *h + *i + (int)j;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_value(int base, int offset, float scale, double factor) {
    return (int)((base + offset) * scale * factor);
}

int main(void) {
    /* Declare many arrays to create memory pressure */
    int arr1[1024];
    int arr2[1024];
    int arr3[1024];
    float farr1[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farr1[i] = i * 0.5f;
        darr1[i] = i * 0.25;
        sarr1[i] = i & 0x7FFF;
    }
    
    /* Declare many scalar variables to increase register pressure */
    int var1 = 1;
    int var2 = 2;
    int var3 = 3;
    int var4 = 4;
    int var5 = 5;
    int var6 = 6;
    int var7 = 7;
    int var8 = 8;
    float fvar1 = 1.1f;
    float fvar2 = 2.2f;
    double dvar1 = 3.3;
    short svar1 = 10;
    long lvar1 = 100;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Create expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* More complex addressing */
        int *ptr3 = &arr3[(i * 3) % 1024];       /* Non-trivial computation */
        
        /* More address computations with different modes */
        float *fptr1 = &farr1[i];
        double *dptr1 = &darr1[i % 512];
        short *sptr1 = &sarr1[(i + 5) % 1024];
        
        /* Scalar computations that are cheap to recompute */
        int offset1 = i * 4;                     /* Multiplication - cheap */
        int offset2 = (i << 2) + 1;              /* Shift + add - cheap */
        float foffset = i * 0.125f;              /* Float multiplication */
        double doffset = i * 0.0625;             /* Double multiplication */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = offset1 + 1;
            fvar1 = foffset * 2.0f;
        } else if (i % 13 == 0) {
            var2 = offset2 - 1;
            dvar1 = doffset / 2.0;
        }
        
        /* Use computed values before call (must be kept live) */
        int temp1 = var1 + offset1;
        int temp2 = var2 + offset2;
        float ftemp = fvar1 + foffset;
        double dtemp = dvar1 + doffset;
        
        /* Function call that clobbers caller-saved registers */
        use_registers(temp1, temp2, var3, ftemp, fvar2, 
                     dtemp, svar1, ptr1, ptr2, lvar1);
        
        /* Use values after call (forces save/restore or rematerialization) */
        arr1[i] = temp1 + var4;
        arr2[i & 0x3F] = temp2 + var5;
        
        /* More computations using the same cheap values */
        if (i % 3 == 0) {
            /* Recompute similar expressions */
            int new_offset1 = i * 4;             /* Same as offset1 - candidate for remat */
            int new_offset2 = (i << 2) + 1;      /* Same as offset2 */
            arr3[i] = new_offset1 + new_offset2 + var6;
            
            /* Call another function with recomputed values */
            int computed = compute_value(new_offset1, new_offset2, foffset, doffset);
            farr1[i] = computed * 0.01f;
        }
        
        /* Update variables to create dependencies */
        var1 = (var1 + var7) & 0xFF;
        var2 = (var2 + var8) & 0xFF;
        fvar1 = fvar1 * 1.01f;
        dvar1 = dvar1 * 1.001;
        svar1 = (svar1 + 1) & 0x7F;
        lvar1 = lvar1 + (i % 5);
        
        /* Another conditional with different computation */
        if (i % 11 == 0) {
            /* More address computations */
            int *ptr4 = &arr1[(i + 10) % 1024];
            float *fptr2 = &farr1[(i * 2) % 1024];
            
            /* Use helper function again */
            use_registers(var1, var2, i, fvar1, fvar2, 
                         dvar1, svar1, ptr3, ptr4, lvar1);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)farr1[i] + (int)darr1[i] + sarr1[i];
    }
    
    /* Mix in scalar variables */
    checksum += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8;
    checksum += (int)fvar1 + (int)fvar2 + (int)dvar1 + svar1 + (int)lvar1;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
