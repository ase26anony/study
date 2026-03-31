/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload -o test test.c */
/* Also try: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat -o test test.c */

#include <stdlib.h>
#include <stdio.h>

volatile int sink = 0;

/* Prevent inlining and interprocedural analysis to force virtual register usage */
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
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the if, uses after */
            t3 = t2 + src2;         /* Cross-block copy chain */
            asm volatile("" : : "r"(t3));
            
            t4 = t3 - src3;         /* More virtual register copies */
        } else {
            /* Unreachable but creates control flow structure */
            t4 = 0;
        }
        
        /* Continue chain after the if */
        t5 = t4 + t2;               /* Uses value from before if */
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More dependent computations */
        t6 = src2 + src3;
        t7 = t6 + t5;               /* Mix chains */
        asm volatile("" : : "r"(t7));
        
        t8 = t7 * 2 - src1;
        asm volatile("" : : "r"(t8));
        
        /* Final computation using noipa helpers */
        t9 = helper1(t8, t5);
        int result = helper2(t9, t7);
        result = helper3(result, t6);
        
        /* Prevent dead code elimination */
        sink += result;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 = (src1 + 1) & 0xFFF;
        src2 = (src2 + 2) & 0xFFF;
        src3 = (src3 + 3) & 0xFFF;
    }
    
    printf("Result: %d\n", sink);
    return 0;
}
