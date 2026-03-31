/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload test.c */
/* Or: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat test.c */

#include <stdlib.h>

volatile int global_sink = 0;

/* Prevent interprocedural analysis to force virtual register usage */
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
    int src3 = argc > 3 ? atoi(argv[3]) : 8;
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg <- reg + reg */
        t1 = t0 + src3;             /* reg <- reg + reg (copy chain) */
        
        /* Force t1 to be considered used, preventing coalescing */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy in chain */
        t3 = t2 + src2;
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the split */
            t4 = t3 * 2;
            t5 = t4 - src3;
            
            /* Force t5 usage before crossing block boundary */
            asm volatile("" : : "r"(t5));
            
            /* Continuation after the if block */
            t6 = t5 + src1;
        } else {
            /* Unreachable but needed for CFG */
            t6 = src1;
        }
        
        /* More temporaries to increase register pressure */
        t7 = t6 + t0;               /* Mix earlier values */
        t8 = t7 - src2;
        
        /* Force t8 usage */
        asm volatile("" : : "r"(t8));
        
        t9 = t8 * 3;
        
        /* Chain 2: Another independent set of operations */
        int u0 = src2 + src3;
        int u1 = u0 + t9;
        int u2 = u1 - src1;
        
        /* Force u2 usage */
        asm volatile("" : : "r"(u2));
        
        int u3 = u2 * 2;
        int u4 = u3 + t7;
        
        /* Use noipa helpers to force virtual register passing */
        int result1 = helper1(t9, u4);
        int result2 = helper2(result1, src1);
        int result3 = helper3(result2, src2);
        
        /* Prevent dead code elimination */
        global_sink += result3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 = (src1 + 1) & 0xFFF;
        src2 = (src2 * 3 + 1) & 0xFFF;
        src3 = (src3 + 2) & 0xFFF;
    }
    
    return global_sink != 0;
}
