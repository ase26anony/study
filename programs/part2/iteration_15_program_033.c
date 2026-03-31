/* Target: early-remat.cc lines 930-937 */
/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */

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
        t0 = src1 + src2;                /* (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t0));    /* Prevent coalescing */
        
        t1 = t0 + src3;                  /* Another copy chain candidate */
        t2 = t1 + src1;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {                  /* Always true */
            t3 = t2 + src2;
            asm volatile("" : : "r"(t3));
            
            /* Chain continues across basic block boundary */
            t4 = t3 + src3;
        } else {
            /* Unreachable but creates control flow */
            t4 = src1;
        }
        
        /* More temporaries in the same chain */
        t5 = t4 + t0;                    /* Use t0 defined before if */
        asm volatile("" : : "r"(t5));
        
        t6 = t5 + src2;
        t7 = t6 + src1;
        
        /* Another block split opportunity */
        if (src2 > 0) {                  /* Always true */
            t8 = t7 + src3;
        } else {
            t8 = src2;
        }
        
        t9 = t8 + t5;                    /* Use t5 from earlier */
        asm volatile("" : : "r"(t9));
        
        /* Chain 2: Interleaved dependencies */
        int u0 = src3 + src1;
        int u1 = u0 + src2;
        int u2 = u1 + src3;
        asm volatile("" : : "r"(u2));
        
        /* Force virtual register usage through noipa calls */
        int v0 = helper1(t9, u2);
        int v1 = helper2(v0, src1);
        int v2 = helper3(v1, src2);
        
        /* Prevent dead code elimination */
        sink += v2;
        
        /* Modify source slightly to prevent loop invariant removal */
        src1 += (iter & 1);
        src2 ^= 1;
    }
    
    return sink != 0;
}
