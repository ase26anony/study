/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and exercise the uncovered code block in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *c, float d, double e, 
                 short f, long g, char *h, int i, float j) {
    /* Use all arguments to prevent optimization */
    global_volatile = a + b + *c + (int)d + (int)e + f + (int)g + (int)*h + i + (int)j;
    
    /* Additional computation to increase register pressure in caller */
    for (int k = 0; k < 2; k++) {
        global_volatile += k;
    }
}

int main(void) {
    /* Declare arrays to work with */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    double darr1[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        darr1[i] = i * 2.5;
        sarr1[i] = i * 11;
        carr1[i] = (i % 128) + 1;
    }
    
    /* Declare many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 10.1, dvar2 = 20.2;
    short svar1 = 100, svar2 = 200;
    long lvar1 = 1000, lvar2 = 2000;
    char cvar1 = 'A', cvar2 = 'B';
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            var1 += i;
            fvar1 += i * 0.5f;
        }
        
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        float *fptr1 = &farr1[i % 512];          /* Different type, modulo */
        double *dptr1 = &darr1[(i * 3) % 1024];  /* Complex index */
        short *sptr1 = &sarr1[i];                /* Short type */
        char *cptr1 = &carr1[i % 256];           /* Char type */
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Multiplication */
        int offset2 = (i << 2) + 16;             /* Shift and add */
        float foffset = i * 2.5f;                /* Float multiplication */
        double doffset = i * 3.14159;            /* Double multiplication */
        
        /* Use all variables to keep them live */
        var2 = var1 + offset1;
        var3 = var2 * 2 + (int)foffset;
        var4 = var3 - offset2;
        var5 = var4 + (int)doffset;
        
        fvar2 = fvar1 + foffset;
        fvar3 = fvar2 * 2.0f;
        
        dvar2 = dvar1 + doffset;
        
        svar2 = svar1 + (short)i;
        
        lvar2 = lvar1 + i * 100L;
        
        cvar2 = cvar1 + (i % 26);
        
        /* Call helper function - clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, fvar1, dvar1,
                   svar1, lvar1, cptr1, var5, fvar3);
        
        /* Use computed values after call - forces save/restore or rematerialization */
        *ptr1 = var3 + var4;
        *ptr2 = var5 - var2;
        *fptr1 = fvar2 + fvar3;
        *dptr1 = dvar2 * 2.0;
        *sptr1 = svar2;
        *cptr1 = cvar2;
        
        /* More computations to increase pressure */
        if (i % 13 == 0) {
            var1 = *ptr1 + *ptr2;
            fvar1 = *fptr1 * 0.5f;
            dvar1 = *dptr1 + 1.0;
        }
        
        /* Cross-type operations to create different machine modes */
        var2 = (int)(fvar2 + *fptr1);
        var3 = (int)(dvar2 + *dptr1);
        var4 = (int)svar2 + (int)*sptr1;
        var5 = (int)cvar2 + (int)*cptr1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + (unsigned int)farr1[i] + 
                   (unsigned long long)darr1[i] + sarr1[i] + carr1[i];
    }
    
    /* Use checksum and global volatile */
    printf("Checksum: %llu, Global: %d\n", checksum, global_volatile);
    
    return (int)(checksum % 1000);
}
