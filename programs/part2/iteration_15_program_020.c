/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-split-wide-types */

#include <stdlib.h>
#include <stdio.h>

volatile int sink = 0;

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
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic chain */
        t0 = src1 + src2;           /* Creates reg:SI copy */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Another reg:SI copy */
        asm volatile("" : : "r"(t1));
        
        /* Split basic block here - control flow divergence */
        if (src1 > 0) {  /* Always true, but compiler doesn't know */
            t2 = t1 + src1;         /* Definition before if */
            asm volatile("" : : "r"(t2));
        } else {
            t2 = src2;              /* Never taken, but creates CFG edge */
        }
        
        /* Chain continues after if - creates copy across basic blocks */
        t3 = t2 + src2;             /* Use of t2 after if */
        asm volatile("" : : "r"(t3));
        
        t4 = t3 ^ src3;             /* Another dependent computation */
        asm volatile("" : : "r"(t4));
        
        /* Chain 2: More complex dependencies */
        t5 = helper1(t4, src1);     /* Function call forces virtual regs */
        asm volatile("" : : "r"(t5));
        
        t6 = t5 - src2;
        asm volatile("" : : "r"(t6));
        
        /* Another control flow split */
        if (src2 != 0) {            /* Always true with our inputs */
            t7 = helper2(t6, src3);
            asm volatile("" : : "r"(t7));
        } else {
            t7 = t6;
        }
        
        t8 = t7 * 3;                /* Simple constant operation */
        asm volatile("" : : "r"(t8));
        
        t9 = helper3(t8, t4);       /* Mix values from both chains */
        asm volatile("" : : "r"(t9));
        
        /* Final computation to prevent dead code elimination */
        int result = helper1(t9, iter);
        sink += result;             /* Volatile write to force computation */
        
        /* Modify source slightly to prevent complete loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    printf("Result: %d\n", sink);
    return 0;
}
