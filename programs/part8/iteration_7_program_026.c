/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper function */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline)) 
void helper_func(int a, int b, int *c, float d, double e, short f, 
                 long g, char *h, int *i, float *j, double *k, int l) {
    /* Simple operations that use all arguments */
    global_volatile = a + b + *c + (int)d + (int)e + f + (int)g;
    if (h) global_volatile += *h;
    if (i) global_volatile += *i;
    if (j) global_volatile += (int)*j;
    if (k) global_volatile += (int)*k;
    global_volatile += l;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[1024];
    double darr2[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
        darr2[i] = i * 2.71828;
        sarr1[i] = i % 32767;
        carr1[i] = i % 127;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 1.111, dvar2 = 2.222;
    short svar1 = 10, svar2 = 20;
    long lvar1 = 100, lvar2 = 200;
    char cvar1 = 'A', cvar2 = 'B';
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];           /* Candidate for rematerialization */
        int *ptr2 = &arr2[i & 0x3F];    /* More complex address calculation */
        float *fptr1 = &farr1[i];
        float *fptr2 = &farr2[i % 512];
        double *dptr1 = &darr1[i];
        double *dptr2 = &darr2[i & 0x1FF];
        short *sptr1 = &sarr1[i];
        char *cptr1 = &carr1[i];
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * 4;            /* Candidate for rematerialization */
        int offset2 = i * 8 + 16;
        float foffset = i * 1.25f;
        double doffset = i * 2.5;
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 += offset1;
            fvar1 += foffset;
            dvar1 += doffset;
        } else if (i % 13 == 0) {
            var2 += offset2;
            fvar2 += foffset * 2;
            dvar2 += doffset / 2;
        }
        
        /* Function call with many arguments - clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, fvar1, dvar1, svar1, lvar1,
                   cptr1, ptr2, fptr1, dptr1, offset1);
        
        /* Use computed values after the call - they need to be kept live or rematerialized */
        if (ptr1) arr1[i] += var3 + offset1;
        if (ptr2) arr2[i & 0x3F] += var4 + offset2;
        if (fptr1) farr1[i] += fvar3 + foffset;
        if (dptr1) darr1[i] += dvar2 + doffset;
        
        /* More conditional logic to prevent optimization */
        if (i % 17 == 0) {
            var3 = var1 * var2;
            fvar3 = fvar1 * fvar2;
        } else if (i % 23 == 0) {
            var4 = var2 + var3;
            dvar2 = dvar1 + 1.0;
        }
        
        /* Update variables to create dependencies */
        var5 += i;
        svar1 += i % 100;
        svar2 += i % 200;
        lvar1 += i;
        lvar2 += i * 2;
        cvar1 += i % 26;
        cvar2 += i % 26;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i] 
                  + (int)darr1[i] + (int)darr2[i] + sarr1[i] + carr1[i];
    }
    
    checksum += var1 + var2 + var3 + var4 + var5 
              + (int)fvar1 + (int)fvar2 + (int)fvar3
              + (int)dvar1 + (int)dvar2
              + svar1 + svar2 + lvar1 + lvar2 + cvar1 + cvar2;
    
    printf("Checksum: %d\n", checksum);
    return checksum % 256;
}
