/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * and exercise the uncovered code block in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void helper_func(int a, int b, int *ptr1, int *ptr2, 
                 float f1, float f2, double d1, short s1)
{
    /* Complex enough to prevent inlining */
    global_volatile = a + b + (int)f1 + (int)f2 + (int)d1 + s1;
    if (ptr1) *ptr1 += 1;
    if (ptr2) *ptr2 -= 1;
    
    /* Additional computation to increase register pressure in caller */
    asm volatile("" : : "r"(a), "r"(b), "r"(ptr1), "r"(ptr2), 
                   "r"(f1), "r"(f2), "r"(d1), "r"(s1) : "memory");
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_offset(int i, int j, int k)
{
    /* Complex addressing computation - potential remat candidate */
    int offset = (i * 3 + j * 7 + k * 11) & 0xFF;
    global_volatile += offset;
    return offset;
}

int main(void)
{
    /* Declare many arrays to increase register pressure */
    int arr1[1024], arr2[1024], arr3[1024];
    float farr1[1024], farr2[1024];
    double darr1[512];
    short sarr1[2048];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        if (i < 512) darr1[i] = i * 3.5;
        if (i < 2048) sarr1[i] = i & 0x7FFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    int var6 = 6, var7 = 7, var8 = 8, var9 = 9, var10 = 10;
    float fvar1 = 1.1f, fvar2 = 2.2f, fvar3 = 3.3f;
    double dvar1 = 4.4, dvar2 = 5.5;
    short svar1 = 100, svar2 = 200;
    
    /* Pointer variables - addresses are good remat candidates */
    int *ptr1, *ptr2, *ptr3, *ptr4;
    float *fptr1, *fptr2;
    double *dptr1;
    short *sptr1;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Complex conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i * 2;
            var2 = i * 3;
        } else if (i % 13 == 0) {
            var3 = i * 4;
            var4 = i * 5;
        }
        
        /* Compute addresses - these are good rematerialization candidates */
        ptr1 = &arr1[i];                    /* Simple address computation */
        ptr2 = &arr2[i & 0x3F];             /* More complex address */
        ptr3 = &arr3[compute_offset(i, var1, var2)];  /* Function call in address */
        
        /* Additional address computations with different modes */
        fptr1 = &farr1[i];
        fptr2 = &farr2[(i * 3) & 0x1FF];
        dptr1 = &darr1[i % 512];
        sptr1 = &sarr1[i * 2];
        
        /* More computations that could be rematerialized */
        int offset1 = i * sizeof(int);      /* Constant multiplication */
        int offset2 = (i << 2) + 16;        /* Shift and add */
        float foffset = i * 2.5f;           /* Float computation */
        double doffset = i * 3.14159;       /* Double computation */
        
        /* Use all variables to keep them live */
        var5 = var1 + var2;
        var6 = var3 * var4;
        var7 = offset1 + offset2;
        var8 = (int)foffset + (int)doffset;
        var9 = *ptr1 + *ptr2;
        var10 = i & 0xFF;
        
        fvar1 = fvar2 * 2.0f + foffset;
        fvar2 = fvar3 / 2.0f;
        fvar3 = (float)i * 0.25f;
        
        dvar1 = dvar2 * 1.5;
        dvar2 = doffset / 2.0;
        
        svar1 = (short)(i & 0x7F);
        svar2 = (short)(svar1 * 2);
        
        /* Function call that clobbers caller-saved registers */
        helper_func(var1, var2, ptr1, ptr2, 
                   fvar1, fvar2, dvar1, svar1);
        
        /* Use values after call - they need to be preserved/rematerialized */
        *ptr3 = var5 + var6 + var7 + var8 + var9 + var10;
        *fptr1 = fvar1 + fvar2 + fvar3;
        *fptr2 = fvar1 * fvar2;
        *dptr1 = dvar1 + dvar2;
        *sptr1 = svar1 + svar2;
        
        /* Another conditional to create control flow complexity */
        if (i % 11 == 0) {
            helper_func(var3, var4, ptr3, &arr1[0],
                       fvar3, fvar1, dvar2, svar2);
        }
        
        /* Update some variables to prevent CSE */
        var1 += global_volatile & 1;
        var2 += global_volatile & 2;
        var3 += global_volatile & 4;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
        checksum += (unsigned int)farr1[i] + (unsigned int)farr2[i];
        if (i < 512) checksum += (unsigned long long)darr1[i];
        if (i < 2048) checksum += sarr1[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
