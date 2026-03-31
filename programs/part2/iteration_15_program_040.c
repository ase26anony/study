/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -c this_file.c */

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

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Force these to be live throughout the loop */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg1 = reg_src1 + reg_src2 */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* reg2 = reg1 + reg_src3 */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* reg3 = reg2 + reg_src1 */
        
        /* Split basic block here - control flow divergence */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the if, uses after */
            t3 = t2 + src2;         /* reg4 = reg3 + reg_src2 */
            asm volatile("" : : "r"(t3));
            
            t4 = t3 + src3;         /* reg5 = reg4 + reg_src3 */
        } else {
            /* Never taken but creates control flow edge */
            t4 = src1;
        }
        
        /* Continue chain across basic block boundary */
        t5 = t4 + src1;             /* reg6 = reg5 + reg_src1 */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More operations to increase virtual register count */
        t6 = t5 - src2;             /* reg7 = reg6 - reg_src2 */
        t7 = t6 ^ src3;             /* reg8 = reg7 ^ reg_src3 */
        asm volatile("" : : "r"(t7));
        
        t8 = t7 + t5;               /* reg9 = reg8 + reg6 */
        t9 = t8 - t4;               /* reg10 = reg9 - reg5 */
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t9, t7);
        int r2 = helper2(r1, t5);
        int r3 = helper3(r2, t3);
        
        /* Prevent dead code elimination */
        global_sink += r3;
        
        /* Modify sources slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return global_sink;
}
