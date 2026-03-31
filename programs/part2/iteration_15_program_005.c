/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

/* Prevent interprocedural optimization to force virtual register usage */
static int __attribute__((noipa)) helper1(int x, int y) {
    return x + y;
}

static int __attribute__((noipa)) helper2(int x, int y) {
    return x - y;
}

static int __attribute__((noipa)) helper3(int x, int y) {
    return x * y;
}

/* Global sink to prevent dead code elimination */
volatile int sink = 0;

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Force these to be live throughout the loop */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the if, uses after */
            t3 = t2 + src2;         /* Cross-block copy propagation candidate */
            asm volatile("" : : "r"(t3));
        } else {
            /* Unreachable but creates control flow structure */
            t3 = 0;
        }
        
        /* Continue chain after the if */
        t4 = t3 + src3;             /* Use of t3 from different block */
        asm volatile("" : : "r"(t4));
        
        t5 = t4 - src1;             /* Different operation to prevent CSE */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More complex dependencies */
        t6 = helper1(t5, src2);     /* Function call forces virtual regs */
        asm volatile("" : : "r"(t6));
        
        t7 = helper2(t6, src3);
        asm volatile("" : : "r"(t7));
        
        /* Another control flow split */
        if (src2 != 0) {  /* Always true */
            t8 = helper3(t7, src1);
        } else {
            t8 = t7;
        }
        
        t9 = t8 + t6;               /* Use values from both sides */
        asm volatile("" : : "r"(t9));
        
        /* Final use to prevent elimination and create register pressure */
        int result = helper1(t9, iter);
        sink += result;             /* Volatile write ensures side effect */
        
        /* Modify sources slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
