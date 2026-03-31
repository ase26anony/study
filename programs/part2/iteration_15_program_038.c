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
    return x * 2 + y;
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
        /* Declare many short-lived temporaries */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0)); /* Block coalescing */
        
        t1 = t0 + src3;             /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t3 = t2 + src2;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        } else {
            t3 = src3;              /* Never taken but creates control flow */
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + t0;               /* Use t3 defined before if */
        asm volatile("" : : "r"(t4));
        
        t5 = t4 + src3;
        t6 = t5 + src1;
        asm volatile("" : : "r"(t6));
        
        /* More chains to increase register pressure */
        t7 = helper1(t6, src2);     /* Forces virtual regs for args/return */
        t8 = helper2(t7, src3);
        t9 = helper3(t8, src1);
        
        /* Final use to prevent elimination */
        sink += helper1(t9, iter);
        
        /* Modify sources slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
