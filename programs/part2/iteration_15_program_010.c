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
    return x * 2 + y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 100;
    int src2 = argc > 2 ? atoi(argv[2]) : 200;
    int src3 = argc > 3 ? atoi(argv[3]) : 300;
    
    /* Ensure src1 > 0 for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg1 = reg_src1 + reg_src2 */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* reg2 = reg1 + reg_src3 */
        t2 = t1 + src1;             /* reg3 = reg2 + reg_src1 */
        
        /* Split basic block with predictable branch */
        if (src1 > 0) {  /* Always true */
            t3 = t2 + src2;         /* reg4 = reg3 + reg_src2 */
            asm volatile("" : : "r"(t3));
            
            /* Chain 2: More dependent computations */
            t4 = t3 - src3;         /* reg5 = reg4 - reg_src3 */
            t5 = t4 + t2;           /* reg6 = reg5 + reg3 */
        } else {
            /* Unreachable but creates control flow */
            t4 = 0;
            t5 = 0;
        }
        
        /* Continue chain across basic block boundary */
        t6 = t5 + src1;             /* reg7 = reg6 + reg_src1 */
        asm volatile("" : : "r"(t6));
        
        /* Chain 3: More operations creating register pressure */
        t7 = t6 - src2;             /* reg8 = reg7 - reg_src2 */
        t8 = t7 * 2;                /* reg9 = reg8 * 2 */
        t9 = t8 + t5;               /* reg10 = reg9 + reg6 */
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t9, t6);
        int r2 = helper2(r1, t3);
        int r3 = helper3(r2, t0);
        
        /* Prevent dead code elimination */
        sink += r1 + r2 + r3;
        
        /* Modify sources slightly to prevent complete optimization */
        src1 = (src1 + 1) & 0xFFF;
        src2 = (src2 + 2) & 0xFFF;
    }
    
    return sink != 0;
}
