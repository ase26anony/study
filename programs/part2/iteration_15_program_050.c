/* Target: early-remat.cc lines 930-937 */
/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */

#include <stdlib.h>

volatile int sink = 0;

/* Force virtual register creation - no interprocedural analysis */
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
        
        /* Chain 1: Simple dependent assignments */
        t0 = src1 + src2;           /* Vreg creation */
        t1 = t0 + src3;             /* Copy chain */
        
        /* Block splitting with always-true condition */
        if (src1 > 0) {  /* Always true - creates control flow edge */
            t2 = t1 + src1;         /* Use t1 across basic block boundary */
        }
        
        /* Continue chain in same basic block */
        t3 = t2 + src2;
        
        /* Force specific temporary to be "used" - prevent coalescing */
        asm volatile("" : : "r"(t3));
        
        /* Chain 2: More dependent operations */
        t4 = t3 - src1;
        t5 = t4 * 2;
        
        /* Another block split */
        if (src2 > 0) {  /* Always true */
            t6 = t5 + src3;
        }
        
        /* More temporaries to increase register pressure */
        t7 = t6 - src2;
        t8 = t7 + t3;    /* Mix with earlier temporary */
        t9 = t8 * 3;
        
        /* Force another temporary to be "used" */
        asm volatile("" : : "r"(t9));
        
        /* Chain 3: Cross-block dependencies */
        int u0 = t9 + src1;
        int u1;
        
        if (src3 > 0) {
            u1 = u0 - src2;
        }
        
        int u2 = u1 + t6;  /* Use t6 from earlier */
        int u3 = u2 * 2;
        int u4 = u3 - src3;
        
        /* Force use */
        asm volatile("" : : "r"(u4));
        
        /* Chain 4: Final computations with helper calls */
        int v0 = helper1(u4, src1);   /* Forces virtual regs for args */
        int v1 = helper2(v0, src2);
        int v2 = helper3(v1, src3);
        
        /* Use result to prevent elimination */
        sink += v2;
        
        /* Modify source slightly to prevent complete loop invariant removal */
        src1 += (iter & 1);
    }
    
    return sink != 0;
}
