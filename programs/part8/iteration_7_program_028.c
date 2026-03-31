/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void clobber_registers(int a, int b, int c, float d, double e, 
                       short f, char g, int *h, float *i, double *j) {
    /* Complex enough to use many registers but not optimized away */
    global_volatile = a + b + c + (int)d + (int)e + f + g;
    if (h) global_volatile += *h;
    if (i) global_volatile += (int)*i;
    if (j) global_volatile += (int)*j;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
int compute_offset(int base, int scale, int mask) {
    /* Creates rematerialization candidates */
    return (base * scale) & mask;
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    float farr2[1024];
    double darr1[512];
    double darr2[512];
    short sarr1[2048];
    char carr1[4096];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        if (i < 512) {
            darr1[i] = i * 1.25;
            darr2[i] = i * 3.75;
        }
        if (i < 2048) sarr1[i] = i & 0xFF;
        if (i < 4096) carr1[i] = (i * 13) & 0x7F;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 12345;
    int var2 = 67890;
    int var3 = 13579;
    int var4 = 24680;
    float fvar1 = 3.14159f;
    float fvar2 = 2.71828f;
    double dvar1 = 1.41421356;
    double dvar2 = 1.73205080;
    short svar1 = 1000;
    short svar2 = 2000;
    char cvar1 = 'A';
    char cvar2 = 'Z';
    int *ptr1 = &arr1[0];
    int *ptr2 = &arr2[0];
    float *fptr1 = &farr1[0];
    float *fptr2 = &farr2[0];
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Create many rematerialization candidates */
        /* These addresses are cheap to recompute but expensive to keep in registers */
        int *addr1 = &arr1[i];                    /* Candidate 1 */
        int *addr2 = &arr2[i & 0x3F];             /* Candidate 2 */
        float *addr3 = &farr1[i];                 /* Candidate 3 */
        float *addr4 = &farr2[(i * 3) & 0x1FF];   /* Candidate 4 */
        double *addr5 = &darr1[i >> 1];           /* Candidate 5 */
        short *addr6 = &sarr1[i * 2];             /* Candidate 6 */
        char *addr7 = &carr1[i * 4];              /* Candidate 7 */
        
        /* More computations that are cheap to recompute */
        int offset1 = compute_offset(i, 4, 0xFF);  /* Candidate 8 */
        int offset2 = compute_offset(i, 8, 0x7F);  /* Candidate 9 */
        int offset3 = (i * 16) & 0x3FF;            /* Candidate 10 */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 += offset1;
            fvar1 += i * 0.1f;
        } else if (i % 13 == 0) {
            var2 += offset2;
            dvar1 += i * 0.01;
        } else {
            var3 += offset3;
            fvar2 += i * 0.05f;
        }
        
        /* Another conditional with different computation */
        if ((i & 0xF) == 0) {
            var4 = (var4 * 3 + i) & 0xFFFF;
            svar1 = (svar1 + i) & 0x7FFF;
        }
        
        /* Function call that clobbers caller-saved registers */
        /* Many arguments force register pressure */
        clobber_registers(
            var1, var2, var3, fvar1, dvar1,
            svar1, cvar1,
            addr1, addr3, addr5
        );
        
        /* Use the computed addresses after the call */
        /* This forces them to be live across the call */
        *addr1 = *addr1 + var1;
        *addr2 = *addr2 + var2;
        *addr3 = *addr3 + fvar1;
        if (i < 512) {
            *addr5 = *addr5 + dvar1;
        }
        
        /* More computations using the offsets */
        arr1[offset1 & 0x3FF] += var3;
        arr2[offset2 & 0x3FF] += var4;
        
        /* Update pointers with cheap-to-recompute expressions */
        ptr1 = &arr1[(i + 1) & 0x3FF];  /* Another candidate */
        ptr2 = &arr2[(i * 2) & 0x3FF];  /* Another candidate */
        fptr1 = &farr1[(i + 3) & 0x3FF];
        fptr2 = &farr2[(i * 4) & 0x3FF];
        
        /* Use all variables to keep them live */
        svar2 += svar1;
        cvar2 += cvar1;
        dvar2 += dvar1;
        
        /* Another conditional store */
        if (i % 23 == 0) {
            *ptr1 = *ptr1 * 2;
            *fptr1 = *fptr1 * 1.5f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += (unsigned int)farr1[i];
        checksum += (unsigned int)farr2[i];
        if (i < 512) {
            checksum += (unsigned long long)darr1[i];
            checksum += (unsigned long long)darr2[i];
        }
    }
    
    checksum += var1 + var2 + var3 + var4;
    checksum += (unsigned int)fvar1 + (unsigned int)fvar2;
    checksum += (unsigned long long)dvar1 + (unsigned long long)dvar2;
    checksum += svar1 + svar2 + cvar1 + cvar2;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
