/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

volatile int sink = 0;

/* Prevent inlining and constant propagation */
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
    int src3 = argc > 3 ? atoi(argv[3]) : 9;
    
    /* Ensure src1 > 0 for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to stress register allocator */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* (set (reg:SI Vx) (reg:SI Vy)) */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Another copy chain candidate */
        t2 = t1 + src1;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            /* Definitions before branch */
            t3 = t2 + src2;
            asm volatile("" : : "r"(t3));
            
            /* Uses after branch - creates flow across edges */
            t4 = t3 + src3;
        } else {
            /* Unreachable but needed for CFG */
            t4 = src1;
        }
        
        /* Continue chain in same block */
        t5 = t4 + t0;               /* Mix earlier temporary */
        asm volatile("" : : "r"(t5));
        
        t6 = t5 + src1;
        t7 = t6 + src2;
        
        /* Another split opportunity */
        if (src2 != 0) {
            t8 = t7 + src3;
            asm volatile("" : : "r"(t8));
        } else {
            t8 = t7;
        }
        
        /* Final computation using noipa helper */
        t9 = helper1(t8, t5);
        
        /* Use helper2 and helper3 to create more virtual register usage */
        int r1 = helper2(t9, t2);
        int r2 = helper3(r1, t6);
        
        /* Prevent dead code elimination */
        sink += t9 + r1 + r2;
        
        /* Modify source slightly to prevent complete optimization */
        src1 = (src1 + 1) & 0xFF;
        src2 = (src2 * 3) & 0xFF;
    }
    
    return sink != 0;
}
