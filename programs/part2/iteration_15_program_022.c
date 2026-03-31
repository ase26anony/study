/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload -S test.c */
/* Or: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat -S test.c */

#include <stdlib.h>

volatile int sink = 0;

/* Prevent interprocedural optimization to force virtual register usage */
static int __attribute__((noipa)) helper1(int x, int y) {
    return x + y;
}

static int __attribute__((noipa)) helper2(int x, int y) {
    return x - y;
}

static int __attribute__((noipa)) helper3(int x, int y) {
    return x ^ y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;                    /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0));        /* Prevent coalescing */
        
        t1 = t0 + src3;                      /* Another copy chain link */
        t2 = t1 + src1;                      /* Dependent computation */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {                      /* Always true */
            t3 = t2 - src2;                  /* Cross-block definition */
            asm volatile("" : : "r"(t3));    /* Force distinct Vreg */
        } else {
            t3 = src3;                       /* Unreachable but needed for CFG */
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src3;                      /* Use of cross-block Vreg */
        t5 = t4 ^ src1;                      /* Another operation */
        
        /* Chain 2: More operations to increase register pressure */
        t6 = src2 + src3;
        t7 = t6 - src1;
        asm volatile("" : : "r"(t7));        /* Prevent optimization */
        
        /* Use helper functions to force virtual register passing */
        t8 = helper1(t5, t7);                /* Function call prevents coalescing */
        t9 = helper2(t8, src2);
        
        /* Final computation using helper to prevent DCE */
        int result = helper3(t9, src3);
        
        /* Use result to prevent elimination */
        sink += result;
        
        /* Modify source slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
