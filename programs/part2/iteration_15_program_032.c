/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

volatile int sink = 0;

/* Force virtual register usage with no interprocedural analysis */
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
    
    /* Force compiler to keep these as actual values */
    asm volatile("" : "+r"(src1), "+r"(src2), "+r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple dependent computations */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src1;             /* Creates copy chain */
        t2 = t1 + src2;             /* Another dependent computation */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            t3 = t2 + src3;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        } else {
            t3 = src1;              /* Never taken but creates CFG edge */
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + t0;               /* Use value defined before if */
        t5 = t4 + src2;
        asm volatile("" : : "r"(t5));
        
        /* More computations to increase virtual register count */
        t6 = t5 * 2;
        t7 = t6 - src1;
        
        /* Another block split opportunity */
        if (src2 != 0) {
            t8 = t7 + t3;
        } else {
            t8 = t7 - t3;
        }
        
        t9 = t8 + t4;
        asm volatile("" : : "r"(t9));
        
        /* Force virtual register usage through noipa calls */
        int r1 = helper1(t5, t9);
        int r2 = helper2(t8, t7);
        int r3 = helper3(r1, r2);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 ^= 2;
    }
    
    return sink != 0;
}
