/* test-mcf-dump-fixup-edge.c
 * 
 * This test is designed to trigger the uncovered debug dumping code in GCC's
 * min-cost flow solver (mcf.cc) when compiled with a GCC built with internal
 * checking enabled (--enable-checking). The uncovered lines print special node
 * labels (NEW_EXIT, NEW_ENTRY) for artificial source/sink nodes added during
 * fixup graph construction.
 *
 * The strategy is to create a function with extremely high register pressure
 * across complex control flow, targeting an architecture with few registers,
 * using the priority-based IRA algorithm, and clobbering registers via inline
 * assembly to force the allocator to build a fixup graph requiring additional
 * nodes.
 *
 * Coverage is achieved at compile-time when GCC's IRA phase runs with
 * MCF_DEBUG defined and dumps the fixup graph.
 */

/* Force the priority-based allocator for this function.
 * This algorithm uses the min-cost flow solver we want to exercise.
 */
#define ATTR_OPT __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARMv7-a, which has only 14 general-purpose integer registers (r0-r12, lr).
 * This makes register pressure acute.
 */
#define ATTR_TARGET __attribute__((target("arch=armv7-a")))

/* Combine attributes for the high-pressure function */
#define HIGH_PRESSURE ATTR_OPT ATTR_TARGET

/* Volatile assembly to clobber many ARM registers and memory.
 * This increases perceived register pressure and prevents optimizations
 * that could simplify liveness.
 */
#define CLOBBER_MANY_ARM_REGS \
    asm volatile("" : : : "memory", \
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
                 "r8", "r9", "r10", "r11", "r12", "r14")

/* The main high-pressure function.
 * It uses many integer variables simultaneously live across a switch statement
 * with multiple basic blocks. Different subsets of variables are used in each
 * case to create complex interference.
 */
HIGH_PRESSURE void high_pressure_func(int selector) {
    /* Declare many integer variables (at least 14) to exceed ARM's register count.
     * All are initialized to make them live from the start.
     */
    int v0 = selector;
    int v1 = v0 + 1;
    int v2 = v1 * 2;
    int v3 = v2 - v0;
    int v4 = v3 ^ v1;
    int v5 = v4 | v2;
    int v6 = v5 & v3;
    int v7 = v6 + v4;
    int v8 = v7 * v5;
    int v9 = v8 - v6;
    int v10 = v9 ^ v7;
    int v11 = v10 | v8;
    int v12 = v11 & v9;
    int v13 = v12 + v10;
    int v14 = v13 * v11;
    int v15 = v14 - v12;
    
    /* Clobber registers early to force spills/reloads */
    CLOBBER_MANY_ARM_REGS;
    
    /* Complex switch with multiple cases, each using a different subset of
     * variables. This creates many live ranges across basic blocks.
     */
    switch (selector & 0x7) {
        case 0:
            v0 = v1 + v2;
            v3 = v4 * v5;
            v6 = v7 ^ v8;
            v9 = v10 | v11;
            v12 = v13 & v14;
            v15 = v0 + v3;
            break;
        case 1:
            v1 = v2 - v3;
            v4 = v5 ^ v6;
            v7 = v8 | v9;
            v10 = v11 & v12;
            v13 = v14 + v15;
            v0 = v1 * v4;
            break;
        case 2:
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 ^ v10;
            v11 = v12 | v13;
            v14 = v15 & v0;
            v1 = v2 - v5;
            break;
        case 3:
            v3 = v4 - v5;
            v6 = v7 ^ v8;
            v9 = v10 | v11;
            v12 = v13 & v14;
            v15 = v0 + v1;
            v2 = v3 * v6;
            break;
        case 4:
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 ^ v12;
            v13 = v14 | v15;
            v0 = v1 & v2;
            v3 = v4 - v7;
            break;
        case 5:
            v5 = v6 - v7;
            v8 = v9 ^ v10;
            v11 = v12 | v13;
            v14 = v15 & v0;
            v1 = v2 + v3;
            v4 = v5 * v8;
            break;
        case 6:
            v6 = v7 + v8;
            v9 = v10 * v11;
            v12 = v13 ^ v14;
            v15 = v0 | v1;
            v2 = v3 & v4;
            v5 = v6 - v9;
            break;
        default: /* case 7 */
            v7 = v8 - v9;
            v10 = v11 ^ v12;
            v13 = v14 | v15;
            v0 = v1 & v2;
            v3 = v4 + v5;
            v6 = v7 * v10;
            break;
    }
    
    /* More computations using all variables to keep them live */
    v0 = v1 + v2 + v3 + v4;
    v5 = v6 * v7 * v8 * v9;
    v10 = v11 ^ v12 ^ v13 ^ v14;
    v15 = v0 | v5 | v10;
    
    /* Final clobber to prevent dead-code elimination */
    CLOBBER_MANY_ARM_REGS;
    
    /* Use all variables in a volatile asm to ensure they appear live */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                       "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                       "r"(v8), "r"(v9), "r"(v10), "r"(v11),
                       "r"(v12), "r"(v13), "r"(v14), "r"(v15));
}

/* A second function with different control flow to increase variety.
 * Uses nested if-else chains instead of a switch.
 */
HIGH_PRESSURE void high_pressure_func2(int x, int y) {
    int a = x;
    int b = y;
    int c = a + b;
    int d = a - b;
    int e = a * b;
    int f = a ^ b;
    int g = a | b;
    int h = a & b;
    int i = c + d;
    int j = e * f;
    int k = g ^ h;
    int l = i | j;
    int m = k & l;
    int n = m + c;
    int o = n * d;
    int p = o ^ e;
    
    CLOBBER_MANY_ARM_REGS;
    
    /* Nested if-else chain creating many basic blocks */
    if (x > 0) {
        a = b + c;
        d = e * f;
        g = h ^ i;
        if (y < 0) {
            j = k | l;
            m = n & o;
            p = a + d;
        } else {
            j = k & l;
            m = n | o;
            p = a - d;
        }
        g = j + m;
    } else if (x < 0) {
        b = c - d;
        e = f ^ g;
        h = i | j;
        if (y > 10) {
            k = l & m;
            n = o + p;
            a = b * e;
        } else {
            k = l | m;
            n = o - p;
            a = b / 2;
        }
        h = k + n;
    } else {
        c = d + e;
        f = g * h;
        i = j ^ k;
        l = m | n;
        o = p & a;
        b = c + f;
    }
    
    /* More uses */
    a = b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    
    CLOBBER_MANY_ARM_REGS;
    asm volatile("" : : "r"(a));
}

/* Trivial main to make the file compilable.
 * The coverage happens at compile-time, not runtime.
 */
int main() {
    return 0;
}
