/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */
#include <stdlib.h>

volatile int sink = 0;

/* Prevent inlining and constant propagation to force virtual register usage */
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
    
    /* Force these to be live throughout the loop */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t0));  /* Block coalescing */
        
        t1 = t0 + src1;             /* Another copy-like operation */
        t2 = t1 + src2;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true given initialization */
            t3 = t2 + src3;
            asm volatile("" : : "r"(t3));
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src1;
        t5 = t4 + src2;
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More operations to increase virtual register count */
        t6 = src3 - src1;
        t7 = t6 + t5;
        
        /* Another split to create more control flow edges */
        if (src2 > 0) {
            t8 = t7 * 2;
            asm volatile("" : : "r"(t8));
        }
        
        t9 = t8 - src3;
        
        /* Use noipa helpers to force virtual register passing */
        int result1 = helper1(t9, t5);
        int result2 = helper2(result1, t2);
        int result3 = helper3(result2, t8);
        
        /* Prevent dead code elimination */
        sink += result3;
        
        /* Modify source variables slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 ^= 2;
        src3 ^= 3;
    }
    
    return sink != 0;
}
