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
    
    /* Force compiler to keep these as live values */
    asm volatile("" : : "r"(src1), "r"(src2), "r"(src3));
    
    /* Long-running loop to create register pressure */
    for (int iter = 0; iter < 1000000; iter++) {
        /* Declare many short-lived temporaries */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain 1: Simple arithmetic creating copy chains */
        t0 = src1 + src2;          /* reg:SI Vxx = reg:SI Vyy */
        asm volatile("" : : "r"(t0));  /* Prevent coalescing */
        
        t1 = t0 + src3;            /* Another copy chain candidate */
        t2 = t1 + src1;            /* Dependent chain continues */
        
        /* Split control flow to create multiple basic blocks */
        if (src1 > 0) {  /* Always true given initialization */
            /* Definitions before the split */
            t3 = t2 - src2;
            t4 = t3 * src3;
            
            /* Force use to prevent optimization */
            asm volatile("" : : "r"(t3), "r"(t4));
        } else {
            /* Unreachable but needed for CFG */
            t3 = 0;
            t4 = 0;
        }
        
        /* Uses after the split - creates flow across edges */
        t5 = t4 + t2;              /* Uses value from before if */
        asm volatile("" : : "r"(t5));
        
        /* Continue chain with more temporaries */
        t6 = t5 - src1;
        t7 = t6 * src2;
        t8 = t7 + src3;
        t9 = t8 - t5;
        
        /* Force all temporaries to be considered used */
        asm volatile("" : : "r"(t6), "r"(t7), "r"(t8), "r"(t9));
        
        /* Chain 2: Another independent set of temporaries */
        int u0, u1, u2, u3, u4;
        u0 = src2 + src3;
        u1 = u0 - src1;
        
        /* Another control flow split */
        if (src2 != 0) {
            u2 = u1 * src3;
            asm volatile("" : : "r"(u2));
        }
        
        u3 = u1 + t9;              /* Cross-chain dependency */
        u4 = u3 - src2;
        
        /* Use noipa helpers to force virtual register passing */
        int r1 = helper1(t9, u4);
        int r2 = helper2(r1, src1);
        int r3 = helper3(r2, src2);
        
        /* Prevent dead code elimination */
        sink += r3;
        
        /* Modify source slightly to prevent loop invariant removal */
        src1 ^= 1;
        src2 ^= 2;
        src3 ^= 3;
    }
    
    return sink != 0;
}
