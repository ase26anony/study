/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */
/* Additional flags to try: -O3 -fno-omit-frame-pointer -fno-split-wide-types */

#include <stdlib.h>
#include <stdio.h>

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
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Force compiler to keep these as actual values */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Creates reg = reg + reg */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src1;             /* Dependent copy chain */
        t2 = t1 + src2;             /* Another dependent copy */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            t3 = t2 + src3;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        } else {
            /* Unreachable but creates control flow */
            t3 = src1;
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + t0;               /* Use t3 defined before if */
        t5 = t4 + src2;
        
        /* More temporaries to increase register pressure */
        t6 = t5 * 2;
        asm volatile("" : : "r"(t6));
        
        t7 = t6 - src1;
        t8 = t7 ^ src3;
        
        /* Force another split */
        if (src2 != 0) {
            t9 = t8 | src1;
            asm volatile("" : : "r"(t9));
        } else {
            t9 = src2;
        }
        
        /* Create more copy chains with helper calls */
        int u0 = helper1(t9, t6);   /* Forces virtual regs for args */
        int u1 = helper2(u0, t3);
        int u2 = helper3(u1, t0);
        
        /* Use all results to prevent elimination */
        sink += u0 + u1 + u2 + t1 + t2 + t4 + t5 + t7 + t8;
        
        /* Modify source slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    printf("Result: %d\n", sink);
    return 0;
}
