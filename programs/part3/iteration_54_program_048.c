/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage testing */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define COLD
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE static int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex function with irreducible region to force ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Irreducible region using goto */
    if (iterations > 0) {
        goto middle_of_function;
    }
    
start_label:
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (r0 & 1) {
            if (r1 & 2) {
                if (r2 & 4) {
                    r3 = r3 * r4 + r5;
                } else {
                    r6 = r6 ^ r7 | r8;
                }
            } else {
                for (j = 0; j < 3; j++) {
                    r9 = (r9 << j) | (r10 >> j);
                    if (j == 1) continue;
                    r11 += r12 - r13;
                }
            }
        } else {
            switch (r0 % 5) {
                case 0: r14 = r15 * 3; break;
                case 1: r15 = r14 / 2; break;
                case 2: r0 = r1 ^ r2; break;
                case 3: r1 = r3 | r4; break;
                case 4: r2 = r5 & r6; break;
            }
        }
        
        /* Complex loop with break to different points */
        for (k = 0; k < 10; k++) {
            if (k == i % 10) {
                if (r0 > r1) goto middle_of_function;
                if (r2 < r3) break;
            }
            r4 = r5 + r6 * k;
        }
        
        if (i % 7 == 0) {
            goto start_label;
        }
        
        middle_of_function:
        r7 = r8 * r9 - r10;
        r8 = r11 ^ r12 | r13;
    }
    
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    return result % 1000;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int jump_counter = 0;

NOINLINE static int helper_b1(int x) { return x * 3 + 7; }
NOINLINE static int helper_b2(int x) { return x / 2 - 5; }
NOINLINE static int helper_b3(int x) { return x ^ 0xABCD; }

static int pattern_b_new_indices(int depth, int max_depth) {
    /* Function using setjmp/longjmp to create exceptional edges */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int iter = 0; iter < depth * 10; iter++) {
            /* Complex arithmetic on all variables */
            a = helper_b1(iter) + b;
            b = helper_b2(a) ^ c;
            c = helper_b3(b) | d;
            d = a * b - c;
            e = d >> (iter % 16);
            f = e << (iter % 8);
            g = f + helper_b1(e);
            h = g - helper_b2(f);
            i = h * helper_b3(g);
            j = i / (iter % 7 + 1);
            k = j ^ iter;
            l = k | (iter * 3);
            m = l & 0xFF;
            n = m + iter;
            o = n - depth;
            p = o * iter;
            
            /* Occasionally longjmp to create abnormal edge */
            if (iter % 23 == 0 && jump_counter < 3) {
                jump_counter++;
                longjmp(jump_buffer, 1);
            }
            
            /* Nested loop with continue to different cases */
            for (int inner = 0; inner < 5; inner++) {
                if (inner == iter % 5) continue;
                a += inner;
                b -= inner;
                c *= inner;
                d ^= inner;
            }
        }
    } else {
        /* After longjmp */
        if (depth < max_depth) {
            return pattern_b_new_indices(depth + 1, max_depth);
        }
    }
    
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
    FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
    FORCE_USE(i); FORCE_USE(j); FORCE_USE(k); FORCE_USE(l);
    FORCE_USE(m); FORCE_USE(n); FORCE_USE(o); FORCE_USE(p);
    
    return (a + b + c + d + e + f + g + h + 
            i + j + k + l + m + n + o + p) & 0xFFFF;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE static int pattern_c_mixed_pressure(int selector, int iterations) {
    /* Large switch with vector operations */
    int r0 = selector, r1 = selector + 1, r2 = selector + 2, r3 = selector + 3;
    int r4 = selector + 4, r5 = selector + 5, r6 = selector + 6, r7 = selector + 7;
    int r8 = selector + 8, r9 = selector + 9, r10 = selector + 10, r11 = selector + 11;
    int r12 = selector + 12, r13 = selector + 13, r14 = selector + 14, r15 = selector + 15;
    
#ifdef __GNUC__
    v4si v0 = {r0, r1, r2, r3};
    v4si v1 = {r4, r5, r6, r7};
    v4si v2 = {r8, r9, r10, r11};
    v4si v3 = {r12, r13, r14, r15};
#endif
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Large switch with 30+ cases */
        switch ((selector + iter) % 35) {
            case 0:
                r0 = r1 * r2 + r3;
                r1 = r4 ^ r5 | r6;
#ifdef __GNUC__
                v0 = v0 + v1;
#endif
                if (iter % 3 == 0) continue;
                break;
            case 1:
                r2 = r7 - r8 * r9;
                r3 = r10 & r11 | r12;
#ifdef __GNUC__
                v1 = v1 * v2;
#endif
                if (iter % 5 == 0) break;
                /* fall through */
            case 2:
                r4 = r13 / (r14 + 1);
                r5 = r15 << (iter % 8);
#ifdef __GNUC__
                v2 = v2 - v3;
#endif
                break;
            case 3:
                r6 = r0 ^ r1;
                r7 = r2 | r3;
#ifdef __GNUC__
                v3 = v3 & v0;
#endif
                if (iter % 7 == 0) continue;
                break;
            case 4:
                r8 = r4 * 3 + r5;
                r9 = r6 / 2 - r7;
#ifdef __GNUC__
                v0 = v0 | v1;
#endif
                break;
            case 5:
                r10 = r8 << 1;
                r11 = r9 >> 2;
#ifdef __GNUC__
                v1 = v1 ^ v2;
#endif
                break;
            case 6:
                r12 = r10 + r11 * r0;
                r13 = r1 - r2 / r3;
#ifdef __GNUC__
                v2 = v2 + v3;
#endif
                if (iter % 11 == 0) break;
                /* fall through */
            case 7:
                r14 = r4 & 0xFF;
                r15 = r5 | 0x55;
#ifdef __GNUC__
                v3 = v3 * v0;
#endif
                break;
            case 8:
                r0 = r6 ^ r7;
                r1 = r8 & r9;
#ifdef __GNUC__
                v0 = v0 - v1;
#endif
                break;
            case 9:
                r2 = r10 * r11;
                r3 = r12 + r13;
#ifdef __GNUC__
                v1 = v1 | v2;
#endif
                break;
            case 10:
                r4 = r14 << 3;
                r5 = r15 >> 1;
#ifdef __GNUC__
                v2 = v2 ^ v3;
#endif
                if (iter % 13 == 0) continue;
                break;
            /* 25 more cases follow similar pattern */
            case 11: r6 = r0 + r1 * 2; r7 = r2 - r3 / 3; break;
            case 12: r8 = r4 ^ r5; r9 = r6 | r7; break;
            case 13: r10 = r8 * 5; r11 = r9 + 7; break;
            case 14: r12 = r10 & 0xF; r13 = r11 | 0xA; break;
            case 15: r14 = r12 << 2; r15 = r13 >> 1; break;
            case 16: r0 = r14 * r15; r1 = r0 + r1; break;
            case 17: r2 = r2 ^ r3; r3 = r4 | r5; break;
            case 18: r4 = r6 - r7; r5 = r8 * r9; break;
            case 19: r6 = r10 / 2; r7 = r11 + 3; break;
            case 20: r8 = r12 & 0x7; r9 = r13 ^ 0x9; break;
            case 21: r10 = r14 << 1; r11 = r15 >> 3; break;
            case 22: r12 = r0 * 7; r13 = r1 - 5; break;
            case 23: r14 = r2 | r3; r15 = r4 & r5; break;
            case 24: r0 = r6 + r7; r1 = r8 * r9; break;
            case 25: r2 = r10 ^ 0xFF; r3 = r11 | 0xAA; break;
            case 26: r4 = r12 << 4; r5 = r13 >> 2; break;
            case 27: r6 = r14 * 3; r7 = r15 + 9; break;
            case 28: r8 = r0 & r1; r9 = r2 | r3; break;
            case 29: r10 = r4 - r5; r11 = r6 * r7; break;
            case 30: r12 = r8 / 4; r13 = r9 + 1; break;
            case 31: r14 = r10 ^ r11; r15 = r12 & r13; break;
            case 32: r0 = r14 << 3; r1 = r15 >> 2; break;
            case 33: r2 = r0 * 6; r3 = r1 - 8; break;
            case 34:
                r4 = r2 | r3;
                r5 = r4 ^ r5;
                if (iter % 17 == 0) break;
                /* fall through to force edge creation */
            default:
                r6 = iter * 2;
                r7 = iter + 5;
                break;
        }
        
        /* Vector operations mixed with scalar */
#ifdef __GNUC__
        if (iter % 4 == 0) {
            v0 = v0 + v1;
            v1 = v1 * v2;
        } else if (iter % 4 == 1) {
            v2 = v2 - v3;
            v3 = v3 & v0;
        } else if (iter % 4 == 2) {
            v0 = v0 | v1;
            v1 = v1 ^ v2;
        } else {
            v2 = v2 + v3;
            v3 = v3 * v0;
        }
#endif
    }
    
    /* Force all variables live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
#ifdef __GNUC__
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    
    return (r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
            r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15) & 0x7FFF;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
register int conflict_var1 asm ("r10");
register int conflict_var2 asm ("r10");  /* Same register! */
#endif

NOINLINE static int helper_d1(int x) { return x * 2; }
NOINLINE static int helper_d2(int x) { return x + 3; }
NOINLINE static int helper_d3(int x) { return x ^ 0x1234; }

static int pattern_d_artificial_conflict(int param) {
    int local1 = param, local2 = param * 2, local3 = param + 5;
    int local4 = param - 3, local5 = param ^ 0xAA, local6 = param | 0x55;
    int local7 = param << 2, local8 = param >> 1, local9 = param * 3;
    int local10 = param / 2, local11 = param + 7, local12 = param - 9;
    
#ifdef __GNUC__
    /* Force register conflicts */
    conflict_var1 = local1;
    local2 = helper_d1(conflict_var1);
    
    conflict_var2 = local3;
    local4 = helper_d2(conflict_var2);
    
    /* Switch causing live range splits */
    switch (param % 8) {
        case 0:
            conflict_var1 = local5;
            local6 = helper_d3(conflict_var1);
            break;
        case 1:
            conflict_var2 = local7;
            local8 = helper_d1(conflict_var2);
            break;
        case 2:
            conflict_var1 = local9;
            local10 = helper_d2(conflict_var1);
            break;
        case 3:
            conflict_var2 = local11;
            local12 = helper_d3(conflict_var2);
            break;
        case 4:
            conflict_var1 = local2;
            local3 = helper_d1(conflict_var1);
            break;
        case 5:
            conflict_var2 = local4;
            local5 = helper_d2(conflict_var2);
            break;
        case 6:
            conflict_var1 = local6;
            local7 = helper_d3(conflict_var1);
            break;
        case 7:
            conflict_var2 = local8;
            local9 = helper_d1(conflict_var2);
            break;
    }
#endif
    
    /* Loop with complex flow */
    for (int i = 0; i < param % 100 + 10; i++) {
        if (i % 3 == 0) {
#ifdef __GNUC__
            conflict_var1 = local10 + i;
            local11 = helper_d2(conflict_var1);
#endif
        } else if (i % 3 == 1) {
#ifdef __GNUC__
            conflict_var2 = local12 * i;
            local1 = helper_d3(conflict_var2);
#endif
        } else {
#ifdef __GNUC__
            conflict_var1 = local2 ^ i;
            local3 = helper_d1(conflict_var1);
#endif
        }
        
        /* Nested conditionals */
        if (i % 5 == 0) {
            local4 = helper_d1(local5);
            local6 = helper_d2(local7);
        } else if (i % 5 == 1) {
            local8 = helper_d3(local9);
            local10 = helper_d1(local11);
        } else if (i % 5 == 2) {
            local12 = helper_d2(local1);
            local2 = helper_d3(local3);
        } else if (i % 5 == 3) {
            local4 = helper_d1(local5);
            local6 = helper_d2(local7);
        } else {
            local8 = helper_d3(local9);
            local10 = helper_d1(local11);
        }
    }
    
    FORCE_USE(local1); FORCE_USE(local2); FORCE_USE(local3);
    FORCE_USE(local4); FORCE_USE(local5); FORCE_USE(local6);
    FORCE_USE(local7); FORCE_USE(local8); FORCE_USE(local9);
    FORCE_USE(local10); FORCE_USE(local11); FORCE_USE(local12);
    
    return (local1 + local2 + local3 + local4 + local5 + local6 +
            local7 + local8 + local9 + local10 + local11 + local12) & 0xFFF;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    int i, iterations = 100;
    
    if (argc > 1) {
        verbose = 1;
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    if (verbose) {
        printf("Starting MCF coverage test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call all pattern functions multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        int seed = i * 12345 + 6789;
        
        /* Pattern A - ENTRY/EXIT blocks */
        total_result ^= pattern_a_entry_exit(i % 50 + 10, seed);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 7 == 0) {
            jump_counter = 0;
            total_result += pattern_b_new_indices(1, 3);
        }
        
        /* Pattern C - Mixed pressure with vector ops */
        total_result |= pattern_c_mixed_pressure(i % 100, i % 20 + 5);
        
        /* Pattern D - Artificial conflict */
        if (i % 3 == 0) {
            total_result &= pattern_d_artificial_conflict(i % 64 + 8);
        }
        
        /* Alternate between different calling patterns */
        if (i % 11 == 0) {
            /* Call pattern A twice to create different edge frequencies */
            total_result ^= pattern_a_entry_exit(i % 30 + 5, seed ^ 0x5A5A);
        }
        
        if (i % 13 == 0) {
            /* Deep recursion for pattern B */
            jump_counter = 0;
            total_result += pattern_b_new_indices(0, 4);
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    /* Ensure result is used */
    FORCE_USE(total_result);
    
    return total_result != 0 ? 0 : 1;
}
