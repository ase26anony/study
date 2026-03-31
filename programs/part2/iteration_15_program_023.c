/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload test.c */
/* Or: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat test.c */

#include <stdlib.h>

volatile int sink = 0;

/* Prevent inlining and constant propagation to force virtual register usage */
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
        t0 = src1 + src2;                 /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0));     /* Prevent coalescing */
        
        t1 = t0 + src1;                   /* Creates copy-like dependency */
        t2 = t1 + src2;                   /* Another dependent copy chain */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {                   /* Always true, but creates CFG edge */
            t3 = t2 + src3;               /* Definition before if, use after */
            asm volatile("" : : "r"(t3)); /* Force distinct register */
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src1;                   /* Use of t3 from other block */
        t5 = t4 - src2;                   /* More dependencies */
        
        /* Chain 2: Alternate operations to prevent optimization */
        t6 = helper1(t5, src3);           /* Function call forces Vreg args */
        asm volatile("" : : "r"(t6));     /* Block register coalescing */
        
        t7 = helper2(t6, src1);           /* Another noipa call */
        t8 = helper3(t7, src2);           /* Creates more virtual registers */
        
        /* Final computation to prevent dead code elimination */
        t9 = helper1(t8, t7);             /* Mix temporaries */
        
        /* Use result to prevent optimization */
        sink += t9;
        
        /* Modify source slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
