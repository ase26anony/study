/* Target: early-remat.cc lines 930-937 */
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
    int src1 = argc > 1 ? atoi(argv[1]) : 42;
    int src2 = argc > 2 ? atoi(argv[2]) : 17;
    int src3 = argc > 3 ? atoi(argv[3]) : 9;
    
    /* Force compiler to keep these as live values */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple dependent operations */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        t1 = t0 + src1;             /* Copy chain candidate */
        
        /* Block splitting with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            t2 = t1 + src2;
            t3 = t2 + src3;
            
            /* Force t3 to be considered used, prevent coalescing */
            asm volatile("" : : "r"(t3));
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src1;             /* t3 defined before if, used after */
        t5 = t4 + src2;
        
        /* Force t5 to be considered used */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More dependent operations */
        t6 = src3 + src1;
        t7 = t6 + t5;               /* Cross-chain dependency */
        
        /* Another block split */
        if (src2 != 0) {  /* Always true */
            t8 = t7 - src2;
        }
        
        t9 = t8 + src3;
        
        /* Force t9 to be considered used */
        asm volatile("" : : "r"(t9));
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t5, t9);
        int r2 = helper2(t9, t3);
        int r3 = helper3(r1, r2);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source variables slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
