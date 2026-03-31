/* Target: early-remat.cc lines 930-937 */
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
    return x * y;
}

int main(int argc, char **argv) {
    /* Initialize source variables with input-dependent values */
    int src1 = argc > 1 ? atoi(argv[1]) : 100;
    int src2 = argc > 2 ? atoi(argv[2]) : 200;
    int src3 = argc > 3 ? atoi(argv[3]) : 300;
    
    /* Force compiler to keep them as live values */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many local temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple dependent assignments */
        t0 = src1 + src2;           /* Vreg <- Vreg + Vreg */
        t1 = t0 + src1;             /* Copy chain candidate */
        
        /* Block artificial use to prevent coalescing */
        asm volatile("" : : "r"(t1));
        
        t2 = t1 + src2;
        t3 = t2 + src3;
        
        /* Control flow split to create multiple basic blocks */
        if (src1 > 0) {  /* Always true with our initialization */
            /* Definitions before the split */
            t4 = t3 - src1;
            asm volatile("" : : "r"(t4));
            
            /* More computations in the true branch */
            t5 = t4 * src2;
            t6 = t5 + src3;
        } else {
            /* Unreachable but creates control flow structure */
            t5 = src1;
            t6 = src2;
        }
        
        /* Uses after the split - creates phi-like situations */
        t7 = t6 - t3;
        asm volatile("" : : "r"(t7));
        
        /* Chain 2: Another set of dependent computations */
        t8 = helper1(t7, src1);     /* Function call forces Vreg args */
        t9 = helper2(t8, src2);
        
        /* Final use prevents dead code elimination */
        int result = helper3(t9, src3);
        sink += result;
        
        /* Modify source values slightly to prevent constant propagation */
        src1 = (src1 + 1) & 0xFFF;
        src2 = (src2 + 2) & 0xFFF;
        src3 = (src3 + 3) & 0xFFF;
    }
    
    return sink != 0;
}
