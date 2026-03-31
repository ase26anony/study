/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-split-wide-types */

#include <stdlib.h>

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

volatile int global_sink = 0;

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
        t0 = src1 + src2;           /* reg = reg + reg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Creates reg-to-reg copy candidate */
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t3 = t2 - src2;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        } else {
            t3 = src3;              /* Never taken, but creates CFG complexity */
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src1;             /* Uses t3 defined before if */
        t5 = t4 ^ src2;
        
        /* More temporaries to increase register pressure */
        t6 = t5 + src3;
        asm volatile("" : : "r"(t6));
        
        t7 = t6 - src1;
        t8 = t7 ^ src2;
        t9 = t8 + src3;
        
        /* Use helpers to force virtual register passing */
        int r1 = helper1(t5, t6);
        int r2 = helper2(t7, t8);
        int r3 = helper3(t9, r1);
        
        /* Prevent dead code elimination */
        global_sink += r1 + r2 + r3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return global_sink != 0;
}
