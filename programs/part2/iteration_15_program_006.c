/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

volatile int sink = 0;

/* Prevent inlining and constant propagation to force virtual register usage */
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
    
    /* Force register pressure with many iterations */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;          /* reg <- reg + reg */
        asm volatile("" : : "r"(t0));  /* Prevent coalescing */
        
        t1 = t0 + src3;            /* Creates (set (reg) (reg)) pattern */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;            /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true with given initialization */
            t3 = t2 + src2;        /* Definition before block split */
            asm volatile("" : : "r"(t3));
        }
        
        /* Use after block split - creates flow across edges */
        t4 = t3 + src3;            /* Use of t3 from previous block */
        asm volatile("" : : "r"(t4));
        
        /* Chain 2: More operations to increase virtual register count */
        t5 = helper1(t4, src1);    /* Forces virtual regs for args/return */
        asm volatile("" : : "r"(t5));
        
        t6 = helper2(t5, src2);
        asm volatile("" : : "r"(t6));
        
        /* Another block split opportunity */
        if (src2 != 0) {
            t7 = helper3(t6, src3);
            asm volatile("" : : "r"(t7));
        }
        
        t8 = t7 + t6;              /* Use values from both sides of if */
        asm volatile("" : : "r"(t8));
        
        /* Final chain element using noipa helper */
        t9 = helper1(t8, t5);
        asm volatile("" : : "r"(t9));
        
        /* Prevent dead code elimination */
        sink += t9;
        
        /* Modify source variables slightly to prevent complete optimization */
        src1 = (src1 + 1) & 0xFFF;
        src2 = (src2 - 1) & 0xFFF;
    }
    
    return sink != 0;
}
