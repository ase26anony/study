/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to cover the debug dumping code
 * for fixup graph edges with NEW_EXIT and NEW_ENTRY nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of MCF debugging code */
#ifdef MCF_DEBUG
/* Already defined by compiler flag */
#else
/* Ensure we define it if not passed via command line */
#define MCF_DEBUG 1
#endif

/* Function with complex register pressure to trigger fixup graph creation */
__attribute__((noinline))
void test_ira_conflict(int iterations) {
    /* Declare many variables with overlapping live ranges */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across loop iterations */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 5; inner++) {
            /* Complex computation keeping many values live */
            f = g + h + inner;
            g = h + i + outer;
            h = i + j + a;
            i = j + k + b;
            j = k + l + c;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
            
            /* More computations to extend live ranges */
            k = l + m + d;
            l = m + n + e;
            m = n + o + f;
            n = o + p + g;
            o = p + q + h;
        }
        
        /* Additional computations in outer loop */
        p = q + r + i;
        q = r + s + j;
        r = s + t + k;
        s = t + a + l;
        t = a + b + m;
        
        /* Another asm statement with different clobbers */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r" (a)
            : "r" (b), "r" (c)
            : "cc"
        );
    }
    
    /* Force all variables to be used to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Second test function with different register pressure pattern */
__attribute__((noinline))
void test_ira_conflict2(int seed) {
    /* Variables with alternating live ranges */
    int v1 = seed, v2 = seed * 2, v3 = seed * 3;
    int v4, v5, v6, v7, v8, v9, v10;
    
    /* Unrolled loop to create many temporary values */
    for (int i = 0; i < 100; i++) {
        v4 = v1 + v2;
        v5 = v2 + v3;
        v6 = v3 + v4;
        v7 = v4 + v5;
        v8 = v5 + v6;
        v9 = v6 + v7;
        v10 = v7 + v8;
        
        /* Chain computations to extend live ranges */
        v1 = v8 + v9;
        v2 = v9 + v10;
        v3 = v10 + v1;
        
        /* Conditional that forces both branches to be compiled */
        if (i & 1) {
            asm volatile ("nop" : : : "r0", "r1", "r2", "r3");
            v4 = v1 * 2;
            v5 = v2 * 3;
        } else {
            asm volatile ("nop" : : : "r4", "r5", "r6", "r7");
            v4 = v1 / 2;
            v5 = v2 / 3;
        }
    }
    
    /* Use all variables */
    volatile int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    (void)sum;
}

/* Third test: Function with switch statement for control flow complexity */
__attribute__((noinline))
void test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    switch (mode % 5) {
        case 0:
            x1 = x2 + x3;
            x4 = x5 + x6;
            asm volatile ("" : : : "r8", "r9", "r10");
            break;
        case 1:
            x2 = x3 + x4;
            x5 = x6 + x7;
            asm volatile ("" : : : "r11", "r12");
            break;
        case 2:
            x3 = x4 + x5;
            x6 = x7 + x8;
            asm volatile ("" : : : "r13", "r14", "r15");
            break;
        case 3:
            x4 = x5 + x6;
            x7 = x8 + x9;
            /* Clobber many registers */
            asm volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
            break;
        case 4:
            x5 = x6 + x7;
            x8 = x9 + x10;
            /* Different clobber set */
            asm volatile ("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            break;
    }
    
    /* Complex expression using all variables */
    volatile int total = x1 * x2 + x3 * x4 + x5 * x6 + x7 * x8 + x9 * x10;
    (void)total;
}

/* Main function to drive different test cases */
int main() {
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    test_ira_conflict(10);
    test_ira_conflict2(42);
    test_ira_conflict3(7);
    test_ira_conflict3(13);
    test_ira_conflict(5);
    
    return 0;
}
