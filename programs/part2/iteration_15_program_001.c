/* Target: early-remat.cc lines 930-937 */
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
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with always-taken branch */
        if (src1 > 0) {  /* Always true */
            /* Definition before branch, use after branch */
            t3 = t2 + src2;
            asm volatile("" : : "r"(t3));
        } else {
            /* Unreachable but creates control flow */
            t3 = 0;
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src3;             /* Use of t3 defined before branch */
        asm volatile("" : : "r"(t4));
        
        /* Chain 2: More operations to increase virtual register count */
        t5 = src2 + src3;
        t6 = t5 + t4;               /* Cross-chain dependency */
        asm volatile("" : : "r"(t6));
        
        t7 = t6 - src1;
        
        /* Another branch to create more control flow edges */
        if (src2 != 0) {  /* Always true with our initialization */
            t8 = t7 ^ src3;
        } else {
            t8 = t7;
        }
        
        t9 = t8 + t3;               /* Use value from first chain */
        asm volatile("" : : "r"(t9));
        
        /* Final computation using noipa helpers - forces virtual registers
           for arguments and return values */
        int h1 = helper1(t9, t4);
        int h2 = helper2(h1, t6);
        int h3 = helper3(h2, t8);
        
        /* Prevent dead code elimination */
        sink += h3;
        
        /* Modify source slightly to prevent complete loop optimization */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
