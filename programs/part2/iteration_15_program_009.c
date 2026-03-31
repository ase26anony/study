/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -c this_file.c */

#include <stdlib.h>

static volatile int sink = 0;

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
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0)); /* Block coalescing */
        
        t1 = t0 + src3;             /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t3 = t2 + src2;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src3;             /* Use of t3 from other block */
        t5 = t4 - src1;
        
        /* More operations to increase register pressure */
        t6 = t5 ^ src2;
        asm volatile("" : : "r"(t6));
        
        t7 = t6 + t0;               /* Reuse earlier temporary */
        t8 = t7 - t2;
        
        /* Final chain using noipa helpers */
        int h1 = helper1(t8, t5);   /* Forces virtual regs for args/return */
        int h2 = helper2(h1, t6);
        int h3 = helper3(h2, t7);
        
        /* Prevent dead code elimination */
        sink += h3;
        
        /* Modify sources slightly to prevent complete optimization */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    return sink != 0;
}
