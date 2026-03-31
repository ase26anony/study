/* reorg_delay_slot.c
 * Designed to trigger delay slot filling logic in GCC's reorg.cc
 * Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -S reorg_delay_slot.c
 * For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -S reorg_delay_slot.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static volatile int sink;

/* Main function with carefully constructed control flow */
int main(int argc, char *argv[]) {
    /* Use argc to create runtime-dependent values */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Local variables - keep them in registers where possible */
    int a = base + 1;
    int b = base * 2;
    int c = base / 3;
    int d = base - 10;
    int e = base + 100;
    
    /* Prevent constant propagation */
    if (argc > 2) {
        a += atoi(argv[2]);
    }
    
    /* Loop to create scheduling context */
    for (int i = 0; i < (argc > 1 ? 100 : 200); ++i) {
        /* Mix of operations to create register pressure */
        int temp = a * b + c;
        
        /* KEY CONSTRUCT: Conditional jump with potential delay slot candidate */
        if ((i % 7) == (argc & 3)) {  /* Runtime-dependent condition */
            /* Jump to label where candidate instruction resides */
            goto delay_candidate_label;
        }
        
        /* Alternative path */
        a = b + c;
        continue;
        
        /* TARGET LABEL: Placed immediately before simple, safe instruction */
        delay_candidate_label:
        /* Candidate instruction for delay slot:
           - Simple arithmetic (no trapping)
           - Uses different variables than jump condition
           - Not a jump or complex sequence */
        d = e + 1;  /* Simple register-to-register operation */
        
        /* Continue with other operations */
        b = c * temp;
        c = d - a;
    }
    
    /* Additional control flow to prevent tail merging */
    if (argc > 3) {
        for (int j = 0; j < 50; ++j) {
            a += j;
            if (a & 1) {
                b ^= c;
            } else {
                d |= e;
            }
        }
    }
    
    /* Create observable side effect */
    int result = a + b + c + d + e;
    sink = result;  /* Use volatile to force computation */
    
    /* Return value based on modified variables */
    return result & 0xFF;
}

/* Additional function to increase compilation complexity */
int helper(int x, int y) {
    int z = x * y;
    
    /* Another delay slot opportunity */
    if (x > y) {
        goto helper_label;
    }
    
    z = y - x;
    return z;
    
    helper_label:
    /* Another candidate instruction */
    x = y + 5;
    return x + z;
}
