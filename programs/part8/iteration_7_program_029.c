/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL backend.
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -S early-remat-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent call elimination */
volatile int global_sink;

/* Non-inline function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_registers(int a, int b, int c, float d, double e, 
                   short f, int *p1, int *p2, long long g, 
                   unsigned char h, float i, double j) {
    /* Complex enough to use many registers */
    global_sink = a + b + c + (int)d + (int)e + f + *p1 + *p2 + (int)g + h + (int)i + (int)j;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024], arr2[1024], arr3[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
        sarr1[i] = i & 0x7FFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.111, d2 = 2.222;
    short s1 = 10, s2 = 20;
    unsigned char c1 = 100;
    long long ll1 = 123456789LL;
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent optimization with runtime condition */
        if (i % 7 == 0) {
            v1 = i * 2;
            f1 = i * 1.7f;
        } else if (i % 13 == 0) {
            v2 = i * 3;
            d1 = i * 2.3;
        }
        
        /* Expensive-to-spill computations (address calculations) */
        int *ptr1 = &arr1[i];                    /* Candidate for remat */
        int *ptr2 = &arr2[i & 0x3F];             /* Another candidate */
        float *fptr1 = &farr1[i];                /* Different mode */
        double *dptr1 = &darr1[i];               /* Different mode */
        short *sptr1 = &sarr1[i];                /* Different mode */
        
        /* More computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Candidate for remat */
        int offset2 = (i & 0xFF) * 4;            /* Another candidate */
        float foffset = i * 1.5f;                /* Float mode candidate */
        double doffset = i * 3.14159 / 2.0;      /* Double mode candidate */
        
        /* Use all variables to keep them live */
        v3 = v1 + v2;
        f2 = f1 * 2.0f;
        d2 = d1 / 2.0;
        s1 = (short)(i & 0xFF);
        c1 = (unsigned char)(i & 0xFF);
        ll1 = (long long)i * 1000LL;
        
        /* Call that clobbers caller-saved registers - forces remat decisions */
        use_registers(v1, v2, v3, f1, d1, 
                     s1, ptr1, ptr2, ll1, c1, f2, d2);
        
        /* Use computed values after call - they need to be rematerialized */
        arr3[i] = *ptr1 + offset1;
        farr2[i] = *fptr1 + foffset;
        darr1[i] = *dptr1 + doffset;
        sarr1[i] = *sptr1 + (short)offset2;
        
        /* More computations using the same values */
        v4 = v3 + offset1;
        v5 = v4 + (int)foffset;
        f3 = f2 + (float)doffset;
        
        /* Conditional store to prevent hoisting */
        if (i % 11 == 0) {
            arr1[i] = v4;
            arr2[i] = v5;
        }
        
        /* Update some variables for next iteration */
        v1 = (v1 + 1) & 0xFF;
        f1 = f1 + 0.5f;
        d1 = d1 + 0.25;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + 
                   (int)farr1[i] + (int)farr2[i] + 
                   (int)darr1[i] + sarr1[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
