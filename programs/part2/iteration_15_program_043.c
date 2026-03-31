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
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Create copy chains with simple arithmetic */
        t0 = src1 + src2;           /* reg:SI Vxx = reg:SI Vyy + reg:SI Vzz */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Another copy-like operation */
        t2 = t1 + src1;             /* Chain continues */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            /* Definitions before the if, uses after */
            t3 = t2 + src2;
            asm volatile("" : : "r"(t3));
            
            t4 = t3 - src3;
        } else {
            /* Unreachable but needed for control flow */
            t4 = 0;
        }
        
        /* Chain continues after the if */
        t5 = t4 + t2;               /* Use value from before if */
        asm volatile("" : : "r"(t5));
        
        /* More chains to increase virtual register pressure */
        t6 = t5 * 2;
        t7 = t6 - src1;
        asm volatile("" : : "r"(t7));
        
        t8 = t7 ^ src2;
        t9 = t8 | src3;
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t9, t5);
        int r2 = helper2(r1, t7);
        int r3 = helper3(r2, t3);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
