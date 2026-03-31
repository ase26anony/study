/* Target: early-remat.cc copy_propagate uncovered lines */
#include <stdlib.h>

/* Global sink to prevent elimination */
volatile int global_sink = 0;

/* Non-inlineable helpers to force virtual register usage */
static int __attribute__((noipa)) helper1(int x, int y) {
    return x + y;
}

static int __attribute__((noipa)) helper2(int x, int y) {
    return x - y;
}

static int __attribute__((noipa)) helper3(int x, int y) {
    return x * 2 + y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 42;
    int src2 = argc > 2 ? atoi(argv[2]) : 17;
    int src3 = argc > 3 ? atoi(argv[3]) : 91;
    
    /* Force these to be live throughout */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to stress register allocation */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic chain creating copy opportunities */
        t0 = src1 + src2;           /* set regV1 = regSrc1 + regSrc2 */
        asm volatile("" : : "r"(t0)); /* Block coalescing */
        
        t1 = t0 + src3;             /* set regV2 = regV1 + regSrc3 */
        t2 = t1 + src1;             /* set regV3 = regV2 + regSrc1 */
        
        /* Split basic block here - control flow divergence */
        if (src1 > 0) {  /* Always true with our initialization */
            /* Definitions before the if, uses after */
            t3 = t2 + src2;         /* set regV4 = regV3 + regSrc2 */
            asm volatile("" : : "r"(t3));
            
            t4 = t3 - src3;         /* set regV5 = regV4 - regSrc3 */
        } else {
            /* Unreachable but creates control flow structure */
            t4 = src1;
        }
        
        /* Continue chain after if block */
        t5 = t4 + t2;               /* set regV6 = regV5 + regV3 */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: Another dependent sequence */
        t6 = src2 * 2;
        t7 = t6 + src1;
        t8 = t7 - src3;
        
        /* Force use of t8 to prevent optimization */
        asm volatile("" : : "r"(t8));
        
        /* Chain 3: Cross-chain dependencies */
        t9 = t5 + t8;               /* set regV10 = regV6 + regV9 */
        
        /* Use noipa functions to force virtual register passing */
        int r1 = helper1(t9, src1);
        int r2 = helper2(r1, src2);
        int r3 = helper3(r2, src3);
        
        /* Prevent dead code elimination */
        global_sink += r3;
        
        /* Modify source variables slightly to prevent loop invariant removal */
        src1 = (src1 + 1) & 0xFF;
        src2 = (src2 * 3) & 0xFF;
        src3 = (src3 - 1) & 0xFF;
    }
    
    return global_sink != 0;
}
