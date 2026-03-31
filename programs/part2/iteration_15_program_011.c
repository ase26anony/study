/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

volatile int sink = 0;

/* Force virtual register usage with noipa */
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
    
    /* Long-running loop to stress register allocator */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* (set (reg:SI Vx) (reg:SI Vy)) */
        asm volatile("" : : "r"(t0)); /* Prevent coalescing */
        
        t1 = t0 + src3;             /* Another copy chain candidate */
        t2 = t1 + src1;
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the if */
            t3 = t2 + src2;
            asm volatile("" : : "r"(t3));
            
            /* Use after the if - creates flow across blocks */
            t4 = t3 + src3;
        } else {
            /* Unreachable but creates control flow structure */
            t4 = src1;
        }
        
        /* Continue chain in same block */
        t5 = t4 + t0;               /* Mix temporaries */
        asm volatile("" : : "r"(t5));
        
        t6 = t5 - src2;
        t7 = t6 * 2;
        
        /* Another split to stress the pass */
        if (src2 < 1000) {  /* Always true */
            t8 = t7 + t1;
            asm volatile("" : : "r"(t8));
        } else {
            t8 = 0;
        }
        
        t9 = t8 - t3;
        asm volatile("" : : "r"(t9));
        
        /* Force virtual register usage through noipa calls */
        int r1 = helper1(t9, t5);
        int r2 = helper2(r1, t2);
        int r3 = helper3(r2, t7);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify sources slightly to prevent constant propagation */
        src1 += (iter & 1);
        src2 ^= 1;
    }
    
    return sink != 0;
}
