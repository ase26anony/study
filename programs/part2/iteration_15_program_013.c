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
    return x ^ y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 42;
    int src2 = argc > 2 ? atoi(argv[2]) : 17;
    int src3 = argc > 3 ? atoi(argv[3]) : 99;
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        t1 = t0 + src3;             /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        
        /* Prevent coalescing of t1 */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t3 = t2 + src2;         /* Definition before if, use after */
        } else {
            t3 = src3;              /* Never taken, but creates control flow */
        }
        
        /* Chain continues after if */
        t4 = t3 + src3;
        asm volatile("" : : "r"(t4));  /* Force t4 to be "used" */
        
        t5 = t4 - src1;
        t6 = t5 ^ src2;
        
        /* Another block split opportunity */
        if (src2 != 0) {  /* Always true with our initialization */
            t7 = t6 + t3;           /* Use t3 defined before first if */
        } else {
            t7 = src1;
        }
        
        t8 = t7 * 2;
        asm volatile("" : : "r"(t8));
        
        t9 = t8 - t5;
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t9, t7);
        int r2 = helper2(r1, t4);
        int r3 = helper3(r2, t6);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
