/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */
/* Also try: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat */

#include <stdlib.h>

volatile int sink = 0;

/* Prevent inlining to force virtual register usage */
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
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic chain */
        t0 = src1 + src2;           /* Vreg definition */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Copy chain: (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy in chain */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            /* Definitions before the split */
            t3 = t2 + src2;
            asm volatile("" : : "r"(t3));
            
            /* Use after the split - creates flow across edge */
            t4 = t3 + src3;
        } else {
            /* Unreachable but needed for CFG */
            t4 = src1;
        }
        
        /* Chain continues after if */
        t5 = t4 + t0;               /* Mix temporaries */
        asm volatile("" : : "r"(t5));
        
        t6 = t5 + t1;
        asm volatile("" : : "r"(t6));
        
        /* Another chain with different operations */
        t7 = src2 - src1;
        t8 = t7 ^ src3;
        t9 = t8 & src1;
        
        /* Force use of all temporaries in noipa functions */
        int r1 = helper1(t2, t3);
        int r2 = helper2(t4, t5);
        int r3 = helper3(t6, t7);
        
        /* Prevent dead code elimination */
        sink += r1 + r2 + r3 + t8 + t9;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
