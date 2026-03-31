/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload -S test.c */

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

/* Global sink to prevent dead code elimination */
volatile int sink = 0;

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Long-running loop to stress register allocation */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;          /* Creates reg = reg + reg */
        asm volatile("" : : "r"(t0));  /* Prevent coalescing */
        
        t1 = t0 + src3;            /* Another copy chain candidate */
        asm volatile("" : : "r"(t1));
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true given initialization */
            t2 = t1 + src1;        /* Definition before if, use after */
            asm volatile("" : : "r"(t2));
        } else {
            t2 = src2;             /* Unreachable but creates CFG complexity */
        }
        
        /* Chain 2: Continue with more temporaries */
        t3 = t2 + src2;
        asm volatile("" : : "r"(t3));
        
        t4 = t3 + src3;
        /* No asm here to allow some coalescing, creating variety */
        
        /* Chain 3: More operations */
        t5 = t4 - src1;
        asm volatile("" : : "r"(t5));
        
        /* Another control flow split */
        if (src2 != 0) {
            t6 = t5 ^ src2;
        } else {
            t6 = t5 | src3;
        }
        asm volatile("" : : "r"(t6));
        
        /* Chain 4: Final computations */
        t7 = t6 + t0;  /* Mix earlier temporaries */
        t8 = t7 - t3;
        t9 = t8 ^ t5;
        
        /* Use noipa helpers to force virtual register passing */
        int result1 = helper1(t9, t7);
        int result2 = helper2(t8, t6);
        int result3 = helper3(result1, result2);
        
        /* Prevent dead code elimination */
        sink += result3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
