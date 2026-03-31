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
    
    /* Force register pressure with many iterations */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries to stress register allocator */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;          /* Vreg <- Vreg + Vreg */
        asm volatile("" : : "r"(t0));  /* Block coalescing */
        
        t1 = t0 + src3;            /* Another Vreg copy chain */
        t2 = t1 + src1;            /* Dependent chain continues */
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true given initialization */
            t3 = t2 + src2;        /* Definition before if, use after */
            asm volatile("" : : "r"(t3));
        }
        
        /* Continue chain across basic block boundary */
        t4 = t3 + src3;            /* Use of t3 from previous block */
        t5 = t4 + t0;              /* Mix with earlier temporary */
        
        /* More chains with artificial uses */
        t6 = helper1(t5, src1);    /* Function call forces Vreg args */
        asm volatile("" : : "r"(t6));
        
        t7 = helper2(t6, src2);
        t8 = helper3(t7, src3);
        asm volatile("" : : "r"(t8));
        
        /* Final chain element with another function call */
        t9 = helper1(t8, t5);
        
        /* Prevent dead code elimination */
        sink += t9;
        
        /* Modify sources slightly to prevent complete optimization */
        src1 ^= 1;
        src2 += iter & 1;
        src3 -= iter & 1;
    }
    
    return sink != 0;
}
