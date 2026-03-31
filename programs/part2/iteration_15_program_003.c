/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -c test.c */
/* Or: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat -c test.c */

#include <stdlib.h>

volatile int global_sink = 0;

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

/* Main function with register pressure and copy chains */
int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 42;
    int src2 = argc > 2 ? atoi(argv[2]) : 17;
    int src3 = argc > 3 ? atoi(argv[3]) : 99;
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create hot code region */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy dependencies */
        t0 = src1 + src2;           /* Vreg1 = src1 + src2 */
        
        /* Force t0 to be "used" preventing coalescing */
        asm volatile("" : : "r"(t0));
        
        t1 = t0 + src3;             /* Vreg2 = Vreg1 + src3 */
        t2 = t1 + src1;             /* Vreg3 = Vreg2 + src1 */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            /* Definitions before the split */
            t3 = t2 + src2;         /* Vreg4 = Vreg3 + src2 */
            asm volatile("" : : "r"(t3));
            
            /* Use after the split - creates flow across blocks */
            t4 = t3 + src3;         /* Vreg5 = Vreg4 + src3 */
        } else {
            /* Unreachable but compiler doesn't know */
            t4 = 0;
        }
        
        /* Continue chain in same block */
        t5 = t4 + src1;             /* Vreg6 = Vreg5 + src1 */
        asm volatile("" : : "r"(t5));
        
        t6 = t5 + src2;             /* Vreg7 = Vreg6 + src2 */
        t7 = t6 + src3;             /* Vreg8 = Vreg7 + src3 */
        
        /* Chain 2: More operations to increase virtual register count */
        t8 = t7 ^ src1;             /* Vreg9 = Vreg8 ^ src1 */
        asm volatile("" : : "r"(t8));
        
        t9 = t8 | src2;             /* Vreg10 = Vreg9 | src2 */
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t5, t6);
        int r2 = helper2(t7, t8);
        int r3 = helper3(t9, r1);
        
        /* Prevent dead code elimination */
        global_sink += r1 + r2 + r3;
        
        /* Modify sources slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return global_sink != 0;
}
