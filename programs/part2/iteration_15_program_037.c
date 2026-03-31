/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload -o test test.c */
/* Or: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat -o test test.c */

#include <stdlib.h>

volatile int sink = 0;

/* Prevent interprocedural optimization to force virtual register usage */
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
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg1 = src1 + src2 */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* reg2 = reg1 + src3 */
        t2 = t1 + src1;             /* reg3 = reg2 + src1 */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            t3 = t2 + src2;         /* reg4 = reg3 + src2 */
            asm volatile("" : : "r"(t3));
            
            t4 = t3 + src3;         /* reg5 = reg4 + src3 */
        } else {
            /* Unreachable but creates control flow */
            t4 = src1;
        }
        
        /* Chain 2: Continue after the branch */
        t5 = t4 + t0;               /* reg6 = reg5 + reg1 */
        asm volatile("" : : "r"(t5));
        
        t6 = t5 + t1;               /* reg7 = reg6 + reg2 */
        t7 = t6 + t2;               /* reg8 = reg7 + reg3 */
        
        /* Chain 3: More operations to increase live ranges */
        t8 = t7 + t3;               /* reg9 = reg8 + reg4 */
        asm volatile("" : : "r"(t8));
        
        t9 = t8 + t4;               /* reg10 = reg9 + reg5 */
        
        /* Use helper functions to force virtual register passing */
        int r1 = helper1(t5, t6);
        int r2 = helper2(t7, t8);
        int r3 = helper3(t9, r1);
        
        /* Prevent dead code elimination */
        sink += r1 + r2 + r3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 += (iter & 1);
        src2 -= (iter & 1);
    }
    
    return sink != 0;
}
