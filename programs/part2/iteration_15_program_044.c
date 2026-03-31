/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */
/* Also try: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat */

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
    
    /* Ensure src1 > 0 for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;          /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) pattern */
        
        /* Force t0 to be considered used, preventing coalescing */
        asm volatile("" : : "r"(t0));
        
        t1 = t0 + src1;            /* Dependent copy chain */
        t2 = t1 + src2;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {            /* Always true, but creates control flow edge */
            /* Definitions before the if, uses after it */
            t3 = t2 + src3;
            
            /* Force t3 to be distinct virtual register */
            asm volatile("" : : "r"(t3));
        } else {
            /* Unreachable but needed for control flow */
            t3 = 0;
        }
        
        /* Chain continues after if, using t3 defined before if */
        t4 = t3 + src1;
        t5 = t4 + src2;
        
        /* Force t5 to be considered used */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More operations with different temporaries */
        t6 = src3 - src1;
        t7 = t6 ^ src2;
        
        /* Another split to create more basic blocks */
        if (src2 > 0) {
            t8 = t7 + t5;
        } else {
            t8 = t7 - t5;
        }
        
        /* Force t8 to be considered used */
        asm volatile("" : : "r"(t8));
        
        t9 = t8 * 2;
        
        /* Final chain mixing temporaries */
        int r1 = t9 + t3;
        int r2 = r1 - t0;
        int r3 = r2 ^ t5;
        
        /* Use noipa helpers to force virtual register passing */
        int h1 = helper1(r1, r2);
        int h2 = helper2(r3, t8);
        int h3 = helper3(h1, h2);
        
        /* Prevent dead code elimination */
        sink += h3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 = (src1 + 1) & 0x7FFF;
        src2 = (src2 + 2) & 0x7FFF;
        src3 = (src3 + 3) & 0x7FFF;
    }
    
    return sink != 0;
}
