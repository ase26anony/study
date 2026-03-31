/* Target: early-remat.cc lines 930-937 */
/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */

#include <stdlib.h>

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

volatile int sink = 0;

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Force compiler to keep these as actual values */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Creates reg:SI copy pattern */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Another copy chain link */
        t2 = t1 + src1;             /* Dependent computation */
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the split */
            t3 = t2 - src2;
            asm volatile("" : : "r"(t3));
            
            /* More computations in this block */
            t4 = t3 ^ src3;
        } else {
            /* Unreachable but creates control flow structure */
            t4 = 0;
        }
        
        /* Uses after the split - creates virtual register flow across edges */
        t5 = t4 + src1;
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More dependent computations */
        t6 = t5 * 3;                /* Different operation to avoid CSE */
        t7 = t6 - src2;
        
        /* Force another basic block split */
        if (src2 != 0) {  /* Always true */
            t8 = t7 | src3;
        } else {
            t8 = t7 & src3;
        }
        
        t9 = t8 ^ src1;
        asm volatile("" : : "r"(t9));
        
        /* Call noipa helpers to force virtual register passing */
        int r1 = helper1(t5, t9);
        int r2 = helper2(t8, r1);
        int r3 = helper3(t7, r2);
        
        /* Prevent dead code elimination */
        sink += r1 + r2 + r3;
        
        /* Modify source slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink & 255;
}
