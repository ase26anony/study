/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

volatile int sink = 0;

/* Prevent inlining to force virtual register usage */
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
    int src1 = argc > 1 ? atoi(argv[1]) : 12345;
    int src2 = argc > 2 ? atoi(argv[2]) : 67890;
    int src3 = argc > 3 ? atoi(argv[3]) : 24680;
    
    /* Loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;          /* reg = reg + reg */
        asm volatile("" : : "r"(t0)); /* Block coalescing */
        
        t1 = t0 + src1;            /* Creates (set (reg) (reg)) pattern */
        t2 = t1 + src2;
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true, but compiler doesn't know */
            t3 = t2 + src3;
            asm volatile("" : : "r"(t3));
            
            /* More copy chains across basic block boundary */
            t4 = t3 - src1;
            t5 = t4 * src2;
        } else {
            /* Unreachable but creates control flow complexity */
            t4 = src1;
            t5 = src2;
        }
        
        /* Chain 2: Continue after if block */
        t6 = t5 + t0;    /* Use value from before if block */
        asm volatile("" : : "r"(t6));
        
        t7 = t6 - src3;
        t8 = t7 * src1;
        t9 = t8 + src2;
        
        /* Force virtual register usage through noipa calls */
        int r1 = helper1(t9, t6);
        int r2 = helper2(r1, t3);
        int r3 = helper3(r2, t5);
        
        /* Prevent dead code elimination */
        sink += r1 + r2 + r3;
        
        /* Modify source variables slightly to prevent constant propagation */
        src1 += (iter & 1);
        src2 -= (iter & 2) >> 1;
        src3 ^= (iter & 4) >> 2;
    }
    
    return sink != 0;
}
