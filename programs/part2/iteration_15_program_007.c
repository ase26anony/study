/* Target: early-remat.cc lines 930-937 */
#include <stdlib.h>

static volatile int sink = 0;

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
        /* Declare many short-lived temporaries */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0)); /* Block coalescing */
        
        t1 = t0 + src1;             /* Creates (set (reg:SI Vxx) (reg:SI Vyy)) */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src2;
        asm volatile("" : : "r"(t2));
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true, but compiler doesn't know */
            /* Definitions before if, uses after if */
            t3 = t2 + src3;
            asm volatile("" : : "r"(t3));
            
            t4 = t3 - src1;
            /* No asm here - allow potential coalescing */
        } else {
            /* Dead code path to create control flow edge */
            t4 = 0;
        }
        
        /* Continuation after if - uses t4 defined before if */
        t5 = t4 * 2;
        asm volatile("" : : "r"(t5));
        
        /* Chain 2: More dependent operations */
        t6 = helper1(t5, src2);  /* Forces virtual register args/return */
        asm volatile("" : : "r"(t6));
        
        t7 = helper2(t6, src1);
        asm volatile("" : : "r"(t7));
        
        t8 = helper3(t7, src3);
        asm volatile("" : : "r"(t8));
        
        t9 = t8 + t5;
        asm volatile("" : : "r"(t9));
        
        /* Final use to prevent elimination */
        sink += helper1(t9, iter);
        
        /* Modify sources slightly to prevent constant propagation */
        src1 = (src1 + 1) & 0xFF;
        src2 = (src2 + 2) & 0xFF;
        src3 = (src3 + 3) & 0xFF;
    }
    
    return sink != 0;
}
