/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload -S test.c */

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
    
    /* Ensure src1 is positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg = reg + reg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Creates reg-to-reg copy candidate */
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t3 = t2 + src2;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        } else {
            t3 = src3;              /* Unreachable but needed for CFG */
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src1;             /* Use of t3 across block boundary */
        t5 = t4 + src2;
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More operations to increase virtual register count */
        t6 = src3 + src1;
        t7 = t6 + t5;               /* Cross-chain dependency */
        
        /* Another block split */
        if (src2 > 0) {  /* Always true */
            t8 = t7 + src3;
        } else {
            t8 = src1;
        }
        
        t9 = t8 + t4;
        asm volatile("" : : "r"(t9));
        
        /* Use helpers to force virtual register passing */
        int h1 = helper1(t5, t9);
        int h2 = helper2(t2, t8);
        int h3 = helper3(h1, h2);
        
        /* Prevent dead code elimination */
        sink += h3;
        
        /* Modify sources slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
