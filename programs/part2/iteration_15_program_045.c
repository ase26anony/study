/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-reload */
/* Also try: gcc -O3 -fno-omit-frame-pointer -fno-split-wide-types -fdump-rtl-early_remat */

#include <stdlib.h>
#include <stdio.h>

volatile int sink = 0;

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
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic chain */
        t0 = src1 + src2;
        asm volatile("" : : "r"(t0));  /* Prevent coalescing */
        
        t1 = t0 + src3;
        t2 = t1 + src1;
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t3 = t2 + src2;
            asm volatile("" : : "r"(t3));
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src3;
        t5 = t4 + src1;
        
        /* Chain 2: Another dependent chain */
        t6 = src2 + src3;
        asm volatile("" : : "r"(t6));
        
        t7 = t6 + src1;
        
        /* Another split to create more control flow edges */
        if (src2 > 0) {  /* Always true */
            t8 = t7 + src2;
        }
        
        t9 = t8 + src3;
        asm volatile("" : : "r"(t9));
        
        /* Use helpers to force virtual register passing */
        int r1 = helper1(t5, t9);
        int r2 = helper2(r1, t2);
        int r3 = helper3(r2, t6);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source variables slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 += iter & 1;
    }
    
    printf("Result: %d\n", sink);
    return 0;
}
