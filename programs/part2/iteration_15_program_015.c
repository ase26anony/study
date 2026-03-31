/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

volatile int sink = 0;

/* Force virtual register usage with no interprocedural analysis */
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
    int src1 = argc > 1 ? atoi(argv[1]) : 100;
    int src2 = argc > 2 ? atoi(argv[2]) : 200;
    int src3 = argc > 3 ? atoi(argv[3]) : 300;
    
    /* Long-running loop to stress register allocation */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Creates reg:SI copy pattern */
        t1 = t0 + src3;             /* Dependent on t0 */
        
        /* Force t1 to be considered "used" - prevents coalescing */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another dependent copy */
        t3 = t2 - src2;             /* Creates more virtual regs */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the if */
            t4 = t3 * 2;
            
            /* Force t4 usage */
            asm volatile("" : : "r"(t4));
            
            /* More computations in the then-block */
            t5 = t4 + src3;
        } else {
            /* Unreachable but creates control flow structure */
            t5 = 0;
        }
        
        /* Continuation after the if - uses value defined before if */
        t6 = t5 - src1;
        
        /* Force t6 usage */
        asm volatile("" : : "r"(t6));
        
        /* Chain 2: More dependent computations */
        t7 = t6 + src2;
        t8 = t7 * 3;
        
        /* Force t8 usage */
        asm volatile("" : : "r"(t8));
        
        t9 = t8 - src3;
        
        /* Use helper functions to force virtual register passing */
        int result1 = helper1(t9, src1);
        int result2 = helper2(result1, src2);
        int result3 = helper3(result2, src3);
        
        /* Prevent dead code elimination */
        sink += result3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 = (src1 + 1) & 0xFFF;
        src2 = (src2 + 2) & 0xFFF;
        src3 = (src3 + 3) & 0xFFF;
    }
    
    return sink != 0;
}
