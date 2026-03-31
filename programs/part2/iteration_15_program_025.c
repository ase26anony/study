/* Target: early-remat.cc lines 930-937 */
/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */

#include <stdlib.h>

volatile int sink = 0;

/* Force virtual register usage by preventing inlining */
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
    
    /* Long-running loop to stress register allocator */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg = reg + reg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Creates (set (reg) (reg)) pattern */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            /* Definitions before if, uses after if */
            t3 = t2 + src2;         /* Cross-block copy chain */
            asm volatile("" : : "r"(t3));
        } else {
            t3 = 0;  /* Never taken, but creates control flow */
        }
        
        /* Continue chain after if */
        t4 = t3 + src3;
        asm volatile("" : : "r"(t4));
        
        t5 = t4 + t0;               /* Mix earlier temporary */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More operations to increase virtual register count */
        t6 = src2 - src1;
        asm volatile("" : : "r"(t6));
        
        t7 = t6 + t1;
        asm volatile("" : : "r"(t7));
        
        /* Another control flow split */
        if (src2 != 0) {  /* Always true with our initialization */
            t8 = t7 ^ src3;
        } else {
            t8 = t7;
        }
        
        t9 = t8 + t5;
        asm volatile("" : : "r"(t9));
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t9, t4);
        int r2 = helper2(r1, t2);
        int r3 = helper3(r2, t7);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source slightly to prevent complete loop optimization */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
