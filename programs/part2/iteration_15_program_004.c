/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload -S test.c */

static int __attribute__((noipa)) helper1(int x, int y) {
    return x + y;
}

static int __attribute__((noipa)) helper2(int x, int y) {
    return x - y;
}

static int __attribute__((noipa)) helper3(int x, int y) {
    return x * 2 + y;
}

volatile int sink = 0;

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? argv[1][0] : 123;
    int src2 = argc > 2 ? argv[2][0] : 456;
    int src3 = argc > 3 ? argv[3][0] : 789;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple dependent assignments */
        t0 = src1 + src2;          /* Vreg1 = src1 + src2 */
        t1 = t0 + src3;            /* Vreg2 = Vreg1 + src3 */
        
        /* Force t1 to be considered used, preventing coalescing */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;            /* Vreg3 = Vreg2 + src1 */
        t3 = t2 + src2;            /* Vreg4 = Vreg3 + src2 */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true with our initialization */
            /* Definitions before the if */
            t4 = t3 * 2;           /* Vreg5 = Vreg4 * 2 */
            
            /* Force t4 to be considered used */
            asm volatile("" : : "r"(t4));
            
            /* Uses after the if - creates copy propagation across edge */
            t5 = t4 + src3;        /* Vreg6 = Vreg5 + src3 */
        } else {
            /* Unreachable but needed for control flow */
            t5 = src1;
        }
        
        /* Chain 2: More dependent assignments */
        t6 = t5 - src2;            /* Vreg7 = Vreg6 - src2 */
        asm volatile("" : : "r"(t6));
        
        t7 = t6 * 3;               /* Vreg8 = Vreg7 * 3 */
        t8 = t7 + t5;              /* Vreg9 = Vreg8 + Vreg6 */
        asm volatile("" : : "r"(t8));
        
        /* Final computation using noipa helper */
        t9 = helper1(t8, t6);
        
        /* Use result to prevent dead code elimination */
        sink += helper2(t9, src1);
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink;
}
