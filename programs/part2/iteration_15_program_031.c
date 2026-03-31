/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload prog.c */
/* Or: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat prog.c */

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
        t0 = src1 + src2;           /* Creates reg = reg pattern */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t2 = t1 + src1;         /* Definition before if, use after */
            asm volatile("" : : "r"(t2));
        }
        
        /* Continue chain across basic block boundary */
        t3 = t2 + src2;             /* Use of t2 from previous block */
        t4 = t3 + src3;
        asm volatile("" : : "r"(t4));
        
        /* Chain 2: More dependencies */
        t5 = src2 - src1;
        t6 = t5 + t4;
        
        /* Another block split */
        if (src2 != 0) {
            t7 = t6 ^ src3;
            asm volatile("" : : "r"(t7));
        }
        
        t8 = t7 + t5;
        t9 = t8 - t3;
        asm volatile("" : : "r"(t9));
        
        /* Chain 3: Cross-chain dependencies */
        int u0 = t9 + t0;
        int u1 = u0 ^ t4;
        int u2 = u1 + t7;
        asm volatile("" : : "r"(u2));
        
        /* Use helpers to force virtual register passing */
        int r1 = helper1(u2, t9);
        int r2 = helper2(r1, u1);
        int r3 = helper3(r2, t2);
        
        /* Prevent dead code elimination */
        sink += r3 + iter;
        
        /* Modify sources slightly to prevent complete optimization */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
