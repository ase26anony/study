/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -c test.c */

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
    
    /* Force these to be live throughout the loop */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to increase register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple dependent computations */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        t1 = t0 + src3;             /* Copy chain: Vreg <- Vreg */
        
        /* Force t1 to be considered used, preventing coalescing */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 - src1;             /* Another copy-like operation */
        t3 = t2 ^ src2;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the split */
            t4 = t3 + src3;
            t5 = t4 - src1;
            
            /* Force t5 to be used, creating distinct reg RTX */
            asm volatile("" : : "r"(t5));
        } else {
            /* Unreachable but needed for control flow */
            t4 = 0;
            t5 = 0;
        }
        
        /* Uses after the split - creates phi-like virtual register flow */
        t6 = t5 + src2;             /* Vreg from different basic block */
        t7 = t6 ^ src3;
        
        /* More chains to increase rematerialization candidates */
        t8 = helper1(t7, src1);     /* Function call forces virtual regs for args */
        t9 = helper2(t8, src2);
        
        /* Final use to prevent dead code elimination */
        int final = helper3(t9, src3);
        global_sink += final;
        
        /* Modify source variables slightly to prevent complete optimization */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return global_sink & 1;
}
