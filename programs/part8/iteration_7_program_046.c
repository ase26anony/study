/* early-remat-test.c
 * Designed to trigger uncovered lines in GCC's early_remat pass
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile_sink;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void clobber_helper(int a, int b, short c, float d, double e, 
                    int *p1, int *p2, float *f1, double *d1)
{
    /* Complex enough to prevent inlining, simple enough to not dominate runtime */
    global_volatile_sink = a + b + c + (int)d + (int)e;
    if (p1) global_volatile_sink += *p1;
    if (p2) global_volatile_sink += *p2;
    if (f1) global_volatile_sink += (int)*f1;
    if (d1) global_volatile_sink += (int)*d1;
    
    /* Additional operations to increase register pressure around call */
    asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                 "rsi", "rdi", "r8", "r9", "r10", "r11", 
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", 
                 "xmm5", "xmm6", "xmm7", "xmm8", "xmm9");
}

int main(void) {
    /* Large arrays to work with */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    float arr4[1024];
    double arr5[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = (float)(i * 0.5f);
        arr5[i] = (double)(i * 0.25);
    }
    
    /* Many scalar variables with different types to increase register pressure */
    int scalar1 = 42;
    int scalar2 = 123;
    short scalar3 = 789;
    float scalar4 = 3.14159f;
    double scalar5 = 2.71828;
    int scalar6 = 999;
    int scalar7 = 111;
    short scalar8 = 222;
    float scalar9 = 4.5f;
    double scalar10 = 5.5;
    int scalar11 = 333;
    int scalar12 = 444;
    
    /* Loop with high register pressure and expensive-to-spill computations */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime-dependent condition */
        if (i % 7 == 0) {
            scalar1 += 1;
        }
        
        /* Expensive-to-spill address computations (rematerialization candidates) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        short *ptr3 = &arr3[i % 512];           /* Different type */
        float *ptr4 = &arr4[(i * 3) % 1024];    /* Non-trivial index computation */
        double *ptr5 = &arr5[(i + 5) % 1024];   /* Another variant */
        
        /* More address computations with different modes */
        int *ptr6 = &arr1[(i * 4) % 1024];      /* i*4 is cheap to recompute */
        int *ptr7 = &arr2[(i + scalar1) % 1024]; /* Mixed with scalar */
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);          /* i*4 - cheap */
        int offset2 = (i & 0xFF) << 2;          /* masked shift - cheap */
        float float_offset = (float)(i * 2);    /* Different mode */
        double dbl_offset = (double)(i * 3);    /* Another mode */
        
        /* Use all scalars to keep them live */
        scalar2 += scalar1;
        scalar3 = (short)(scalar2 & 0xFFFF);
        scalar4 = scalar4 * 1.01f;
        scalar5 = scalar5 * 1.001;
        scalar6 = scalar6 ^ scalar1;
        scalar7 = scalar7 | scalar2;
        scalar8 = (short)(scalar8 + scalar3);
        scalar9 = scalar9 + 0.1f;
        scalar10 = scalar10 - 0.01;
        scalar11 = scalar11 * 2;
        scalar12 = scalar12 / 2;
        
        /* Non-inline call that clobbers registers - forces save/restore or rematerialization */
        clobber_helper(
            scalar1, scalar2, scalar3, scalar4, scalar5,
            ptr1, ptr2, ptr4, ptr5
        );
        
        /* Use computed addresses after call - they need to be available */
        *ptr1 = scalar1;
        *ptr2 = scalar2;
        *ptr3 = scalar3;
        *ptr4 = scalar4;
        *ptr5 = scalar5;
        
        /* More uses of recomputable values after call */
        arr1[(i * 4) % 1024] = scalar6 + offset1;
        arr2[(i + scalar1) % 1024] = scalar7 + offset2;
        
        /* Conditional store based on runtime value to prevent optimization */
        if (i % 13 == 0) {
            *ptr6 = scalar8;
            *ptr7 = scalar9;
        }
        
        /* Use float/double computations */
        arr4[i] = float_offset + scalar9;
        arr5[i] = dbl_offset + scalar10;
        
        /* Cross-type operations to increase pressure */
        scalar4 = (float)scalar11 + scalar9;
        scalar5 = (double)scalar12 + scalar10;
        
        /* Another conditional to create control flow complexity */
        if (i % 17 == 0) {
            scalar11 = scalar11 + *ptr1;
            scalar12 = scalar12 + *ptr2;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += arr3[i];
        checksum += (unsigned long long)arr4[i];
        checksum += (unsigned long long)arr5[i];
    }
    
    /* Also use scalars in final computation */
    checksum += scalar1 + scalar2 + scalar3 + (int)scalar4 + (int)scalar5 +
                scalar6 + scalar7 + scalar8 + (int)scalar9 + (int)scalar10 +
                scalar11 + scalar12;
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
