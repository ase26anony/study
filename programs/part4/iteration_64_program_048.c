/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a GCC built with --enable-checking (MCF_DEBUG defined)
 */

/* Force use of priority-based register allocator which uses MCF solver */
#define FORCE_PRIORITY_ALLOC __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers (16 GPRs, some reserved) */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Clobber many ARM registers to increase pressure */
#define CLOBBER_REGS asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

/* Main high-pressure function */
ARM_TARGET FORCE_PRIORITY_ALLOC
void high_pressure_function(void) {
    /* Declare many integer variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with volatile values to prevent constant propagation */
    volatile int seed = 42;
    v0 = seed + 0;
    v1 = seed + 1;
    v2 = seed + 2;
    v3 = seed + 3;
    v4 = seed + 4;
    v5 = seed + 5;
    v6 = seed + 6;
    v7 = seed + 7;
    v8 = seed + 8;
    v9 = seed + 9;
    v10 = seed + 10;
    v11 = seed + 11;
    v12 = seed + 12;
    v13 = seed + 13;
    v14 = seed + 14;
    v15 = seed + 15;
    
    /* Complex control flow with many live ranges */
    /* First block: all variables live */
    CLOBBER_REGS;
    v0 = v1 + v2;
    v3 = v4 * v5;
    v6 = v7 - v8;
    v9 = v10 ^ v11;
    v12 = v13 | v14;
    v15 = v0 & v3;
    
    /* Branch point creating different live ranges */
    if (v0 > 100) {
        /* Subset 1 live across this path */
        CLOBBER_REGS;
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 ^ v12;
        v13 = v14 | v15;
        v0 = v1 & v4;
        
        /* Nested condition for more complexity */
        if (v1 < 50) {
            CLOBBER_REGS;
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 - v10;
            v11 = v12 ^ v13;
            v14 = v15 | v0;
            v1 = v2 & v5;
        } else {
            CLOBBER_REGS;
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 ^ v14;
            v15 = v0 | v1;
            v2 = v3 & v6;
        }
        
        /* Rejoin with more computations */
        v4 = v5 + v6;
        v7 = v8 * v9;
        v10 = v11 - v12;
    } else {
        /* Subset 2 live across alternative path */
        CLOBBER_REGS;
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v0;
        v1 = v2 & v5;
        
        /* Another nested condition */
        if (v2 > 75) {
            CLOBBER_REGS;
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 ^ v14;
            v15 = v0 | v1;
            v2 = v3 & v6;
        } else {
            CLOBBER_REGS;
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            v13 = v14 ^ v15;
            v0 = v1 | v2;
            v3 = v4 & v7;
        }
        
        /* More computations on this path */
        v5 = v6 + v7;
        v8 = v9 * v10;
        v11 = v12 - v13;
    }
    
    /* Final convergence point - all variables become live again */
    CLOBBER_REGS;
    v12 = v13 + v14;
    v15 = v0 * v1;
    v2 = v3 - v4;
    v5 = v6 ^ v7;
    v8 = v9 | v10;
    v11 = v12 & v15;
    
    /* Force all variables to be used to prevent dead code elimination */
    volatile int sink;
    sink = v0; sink = v1; sink = v2; sink = v3;
    sink = v4; sink = v5; sink = v6; sink = v7;
    sink = v8; sink = v9; sink = v10; sink = v11;
    sink = v12; sink = v13; sink = v14; sink = v15;
}

/* Additional high-pressure function with switch statement */
ARM_TARGET FORCE_PRIORITY_ALLOC
void switch_pressure_function(int selector) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    volatile int seed = selector;
    
    a = seed * 1; b = seed * 2; c = seed * 3; d = seed * 4;
    e = seed * 5; f = seed * 6; g = seed * 7; h = seed * 8;
    i = seed * 9; j = seed * 10; k = seed * 11; l = seed * 12;
    m = seed * 13; n = seed * 14; o = seed * 15; p = seed * 16;
    
    /* Switch creates complex control flow graph */
    switch (selector & 7) {
        case 0:
            CLOBBER_REGS;
            a = b + c; d = e * f; g = h - i;
            j = k ^ l; m = n | o; p = a & d;
            break;
        case 1:
            CLOBBER_REGS;
            b = c + d; e = f * g; h = i - j;
            k = l ^ m; n = o | p; a = b & e;
            break;
        case 2:
            CLOBBER_REGS;
            c = d + e; f = g * h; i = j - k;
            l = m ^ n; o = p | a; b = c & f;
            break;
        case 3:
            CLOBBER_REGS;
            d = e + f; g = h * i; j = k - l;
            m = n ^ o; p = a | b; c = d & g;
            break;
        case 4:
            CLOBBER_REGS;
            e = f + g; h = i * j; k = l - m;
            n = o ^ p; a = b | c; d = e & h;
            break;
        case 5:
            CLOBBER_REGS;
            f = g + h; i = j * k; l = m - n;
            o = p ^ a; b = c | d; e = f & i;
            break;
        case 6:
            CLOBBER_REGS;
            g = h + i; j = k * l; m = n - o;
            p = a ^ b; c = d | e; f = g & j;
            break;
        default:
            CLOBBER_REGS;
            h = i + j; k = l * m; n = o - p;
            a = b ^ c; d = e | f; g = h & k;
            break;
    }
    
    /* Post-switch computations keep variables live */
    a = b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    
    volatile int sink;
    sink = a; sink = b; sink = c; sink = d;
    sink = e; sink = f; sink = g; sink = h;
    sink = i; sink = j; sink = k; sink = l;
    sink = m; sink = n; sink = o; sink = p;
}

/* Main function exists only to make compilation work */
int main(void) {
    high_pressure_function();
    switch_pressure_function(3);
    return 0;
}
