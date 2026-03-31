/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */
/* Also try: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat */

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
        t0 = src1 + src2;           /* reg = reg + reg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src1;             /* Creates reg-to-reg copy pattern */
        t2 = t1 + src2;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            t3 = t2 + src3;
            asm volatile("" : : "r"(t3));
            
            /* More chains across the branch */
            t4 = t3 - src1;
            t5 = t4 * 2;
        } else {
            /* Unreachable but creates control flow structure */
            t4 = src1;
            t5 = src2;
        }
        
        /* Continue chain after branch */
        t6 = t5 + t2;    /* Uses value defined before branch */
        asm volatile("" : : "r"(t6));
        
        /* More copy chains */
        t7 = t6 - src3;
        t8 = t7 * 3;
        t9 = t8 + src1;
        
        /* Force use of helper functions with virtual registers */
        int r1 = helper1(t9, t6);
        int r2 = helper2(t8, t5);
        int r3 = helper3(r1, r2);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 = (src1 + 1) & 0xFFF;
        src2 = (src2 + 2) & 0xFFF;
        src3 = (src3 + 3) & 0xFFF;
    }
    
    return sink != 0;
}
