/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

volatile int sink = 0;

/* Force virtual register creation - no interprocedural analysis */
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
    int src3 = argc > 3 ? atoi(argv[3]) : 8;
    
    /* Force register pressure with many iterations */
    for (int iter = 0; iter < 100000; iter++) {
        /* Declare many short-lived temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple dependent assignments */
        t0 = src1 + src2;           /* Vreg creation */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Copy propagation candidate */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true with default args */
            t3 = t2 + src2;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        } else {
            t3 = src1;              /* Never taken, but creates CFG complexity */
        }
        
        /* Chain continues after if - creates flow across basic blocks */
        t4 = t3 + src3;             /* Use of t3 across block boundary */
        asm volatile("" : : "r"(t4));
        
        t5 = t4 + t0;               /* Mix with earlier temporary */
        t6 = t5 + src1;
        
        /* More chains to increase virtual register pressure */
        t7 = helper1(t6, src2);     /* Forces virtual regs for args/return */
        asm volatile("" : : "r"(t7));
        
        t8 = helper2(t7, src3);
        t9 = helper3(t8, t5);
        
        /* Final use to prevent elimination */
        sink += helper1(t9, iter);
        
        /* Modify sources slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
