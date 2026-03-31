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
    return x * y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to stress register allocation */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg = reg + reg */
        t1 = t0 + src3;             /* reg = reg + reg */
        
        /* Block splitting with predictable condition */
        if (src1 > 0) {  /* Always true */
            t2 = t1 + src1;         /* reg = reg + reg */
            t3 = t2 + src2;         /* reg = reg + reg */
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src3;             /* reg = reg + reg */
        t5 = t4 + src1;             /* reg = reg + reg */
        
        /* Force specific temporary to be "used" */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More operations with different patterns */
        t6 = helper1(t5, src2);     /* Function call creates virtual regs */
        t7 = helper2(t6, src3);
        
        /* Another block split */
        if (src2 != 0) {  /* Always true */
            t8 = helper3(t7, src1);
        }
        
        t9 = t8 + t6 + t4;          /* Complex use pattern */
        
        /* Final use to prevent dead code elimination */
        sink += helper1(t9, iter);
        
        /* Modify source variables slightly to prevent complete optimization */
        src1 ^= 1;
        src2 += 1;
        src3 -= 1;
    }
    
    return sink != 0;
}
