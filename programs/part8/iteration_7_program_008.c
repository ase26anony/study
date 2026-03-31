/* Program to trigger early rematerialization in GCC RTL backend */
/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_counter = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, char *h, int i, float j) {
    /* Use all arguments to prevent optimization */
    global_counter += a + b + *c + (int)d + (int)e + f + (int)g + (int)*h + i + (int)j;
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
        sarr1[i] = i % 32767;
        carr1[i] = i % 127;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 10.1, dvar2 = 20.2;
    short svar1 = 100, svar2 = 200;
    long lvar1 = 1000, lvar2 = 2000;
    char cvar1 = 'A', cvar2 = 'B';
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            var1 = i * 2;      /* Expensive to spill, cheap to recompute */
        } else if (i % 13 == 0) {
            var2 = i * 3;      /* Another recomputation candidate */
        }
        
        /* Compute addresses that are cheap to recompute but used multiple times */
        int *ptr1 = &arr1[i];               /* Address computation 1 */
        int *ptr2 = &arr2[i & 0x3F];        /* Address computation 2 */
        float *fptr1 = &farr1[i % 512];     /* Address computation 3 */
        double *dptr1 = &darr1[i];          /* Address computation 4 */
        short *sptr1 = &sarr1[i];           /* Address computation 5 */
        char *cptr1 = &carr1[i];            /* Address computation 6 */
        
        /* More computations that could be rematerialized */
        int offset1 = i * 4;                /* Simple arithmetic - remat candidate */
        int offset2 = i * 8 + 16;           /* Another remat candidate */
        float foffset = i * 1.5f;           /* Float computation */
        double doffset = i * 2.5;           /* Double computation */
        
        /* Use all variables to keep them live */
        var3 = offset1 + var1;
        var4 = offset2 + var2;
        fvar1 = foffset + fvar2;
        dvar1 = doffset + dvar2;
        svar1 = (short)(i + svar2);
        lvar1 = i + lvar2;
        cvar1 = (char)(i % 26 + 'A');
        
        /* Function call that clobbers caller-saved registers */
        /* Pass many arguments to increase pressure */
        helper_func(var1, var2, ptr1, fvar1, dvar1, 
                   svar1, lvar1, cptr1, var3, fvar2);
        
        /* Use values after call - they need to be preserved/rematerialized */
        arr1[i] = var3 + *ptr1;
        arr2[i & 0x3F] = var4 + *ptr2;
        farr1[i % 512] = fvar1 + *fptr1;
        darr1[i] = dvar1 + *dptr1;
        sarr1[i] = svar1 + *sptr1;
        carr1[i] = cvar1 + *cptr1;
        
        /* More computations to increase pressure */
        var5 = var1 + var2 + var3 + var4;
        fvar3 = fvar1 + fvar2;
        dvar2 = dvar1 * 1.1;
        
        /* Another conditional to prevent optimization */
        if (i % 11 == 0) {
            helper_func(var5, var1, ptr2, fvar3, dvar2,
                       svar2, lvar2, cptr1, var2, fvar1);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + 
                   (int)darr1[i] + sarr1[i] + carr1[i];
    }
    
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    return checksum % 256;
}
