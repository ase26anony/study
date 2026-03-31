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
    return x * y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 123;
    int src2 = argc > 2 ? atoi(argv[2]) : 456;
    int src3 = argc > 3 ? atoi(argv[3]) : 789;
    
    /* Force src1 to be positive for predictable branch */
    if (src1 <= 0) src1 = 1;
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* reg = reg + reg */
        asm volatile("" : : "r"(t0));  /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Creates (set (reg) (reg)) pattern */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src1;             /* Another copy chain link */
        
        /* Split basic block with predictable condition */
        if (src1 > 0) {  /* Always true */
            t3 = t2 + src2;         /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        } else {
            t3 = src3;              /* Unreachable but needed for CFG */
        }
        
        /* Chain continues after if */
        t4 = t3 + src3;
        asm volatile("" : : "r"(t4));
        
        t5 = t4 - src1;
        asm volatile("" : : "r"(t5));
        
        /* Another block split opportunity */
        if (src2 != 0) {  /* Always true with our initialization */
            t6 = t5 * 2;
            asm volatile("" : : "r"(t6));
        } else {
            t6 = src1;
        }
        
        t7 = t6 + t0;    /* Mix earlier temporary */
        asm volatile("" : : "r"(t7));
        
        t8 = t7 - src2;
        asm volatile("" : : "r"(t8));
        
        t9 = t8 * src3;
        asm volatile("" : : "r"(t9));
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t9, t7);
        int r2 = helper2(r1, t5);
        int r3 = helper3(r2, t3);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source slightly to prevent complete loop invariant removal */
        src1 += (iter & 1);
        src2 ^= (iter << 1) & 0xFF;
    }
    
    return sink != 0;
}
