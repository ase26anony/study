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
    return x ^ y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Long-running loop to stress register allocator */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* set (reg:SI Vxx) (plus:SI (reg:SI src1) (reg:SI src2)) */
        t1 = t0 + src3;             /* Potential: set (reg:SI Vyy) (reg:SI Vxx) */
        
        /* Force t1 to be considered used, preventing coalescing */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        t3 = t2 + src2;
        
        /* Split basic block with always-true condition */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the split */
            t4 = t3 + src3;
            t5 = t4 + src1;
            
            /* Force t5 to be distinct virtual register */
            asm volatile("" : : "r"(t5));
        } else {
            /* Unreachable but needed for control flow */
            t4 = 0;
            t5 = 0;
        }
        
        /* Uses after the split - creates phi-like virtual register flow */
        t6 = t5 + src2;             /* t5 defined before if, used after */
        t7 = t6 + src3;
        
        /* More chains to increase virtual register count */
        t8 = helper1(t7, src1);     /* Function call forces virtual regs for args */
        t9 = helper2(t8, src2);
        
        /* Final use prevents dead code elimination */
        int result = helper3(t9, src3);
        sink += result;  /* Volatile sink prevents optimization */
        
        /* Modify source variables slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
