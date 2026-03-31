/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */
#include <stdlib.h>

volatile int sink = 0;

/* Prevent interprocedural analysis to force virtual register usage */
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
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg <- reg + reg */
        t1 = t0 + src1;             /* reg <- reg + reg (copy chain) */
        
        /* Prevent coalescing of t1 */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src2;             /* Another copy chain link */
        t3 = t2 + src3;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            /* Definitions before the if, uses after */
            t4 = t3 * 2;
            
            /* Force t4 to be live across the block boundary */
            asm volatile("" : : "r"(t4));
        }
        
        /* Continuation after the if - uses t4 defined before if */
        t5 = t4 + src1;             /* Use across basic block boundary */
        
        /* Chain 2: More operations creating register pressure */
        t6 = t5 - src2;
        asm volatile("" : : "r"(t6));  /* Prevent coalescing */
        
        t7 = t6 * 3;
        t8 = t7 + t5;
        
        /* Another split to create more control flow edges */
        if (src2 > 0) {  /* Always true */
            t9 = t8 >> 1;
            asm volatile("" : : "r"(t9));
        }
        
        /* Final computation using noipa helpers to force virtual registers */
        int result1 = helper1(t5, t6);
        int result2 = helper2(t8, t9);
        int result3 = helper3(result1, result2);
        
        /* Prevent dead code elimination */
        sink += result3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 += (iter & 1);
        src2 ^= 1;
    }
    
    return sink != 0;
}
