/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define REGISTER_VAR(name, reg) register int name asm(reg)
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define REGISTER_VAR(name, reg) int name
#endif

/* Global verbose flag for debug output */
static volatile int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = 0;
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    int i = seed * 9, j = seed * 10, k = seed * 11, l = seed * 12;
    int m = seed * 13, n = seed * 14, o = seed * 15, p = seed * 16;
    
    /* Create irreducible region with goto */
    if (iterations > 100) goto irreducible_label;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Deep if-else chain creating many basic blocks */
        if (a % 2 == 0) {
            a = (a * 3 + 1) % 1000;
            if (b % 3 == 0) {
                b = (b * 5 + 2) % 1000;
                if (c % 4 == 0) {
                    c = (c * 7 + 3) % 1000;
                    if (d % 5 == 0) {
                        d = (d * 11 + 4) % 1000;
                    } else {
                        d = (d * 13 + 5) % 1000;
                    }
                } else {
                    c = (c * 17 + 6) % 1000;
                }
            } else {
                b = (b * 19 + 7) % 1000;
            }
        } else {
            a = (a * 23 + 8) % 1000;
        }
        
        /* Nested loops with continue/break to different points */
        for (int i1 = 0; i1 < 5; i1++) {
            for (int j1 = 0; j1 < 5; j1++) {
                if ((i1 * j1) % 7 == seed % 7) {
                    continue;
                }
                if ((i1 + j1) > 8) {
                    break;
                }
                e = (e + i1 * j1) % 1000;
            }
        }
        
        /* Use all variables to prevent elimination */
        result += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
        FORCE_USE(result);
    }
    
    irreducible_label:
    /* Create back-edge to entry */
    if (result < 0) {
        /* This should never happen but creates CFG edge */
        return pattern_a_entry_exit(1, seed);
    }
    
    return result % 1000;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int jump_counter = 0;

NOINLINE
int pattern_b_new_indices(int depth, int max_depth) {
    int r0 = depth * 1, r1 = depth * 2, r2 = depth * 3, r3 = depth * 4;
    int r4 = depth * 5, r5 = depth * 6, r6 = depth * 7, r7 = depth * 8;
    int r8 = depth * 9, r9 = depth * 10, r10 = depth * 11, r11 = depth * 12;
    int r12 = depth * 13, r13 = depth * 14, r14 = depth * 15, r15 = depth * 16;
    
    if (depth >= max_depth) {
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
               r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    }
    
    /* setjmp creates exceptional control flow edges */
    if (setjmp(jump_buffer) == 0) {
        /* Normal path - complex arithmetic */
        for (int i = 0; i < 10; i++) {
            r0 = (r0 * 3 + r1) % 1000;
            r1 = (r1 * 5 + r2) % 1000;
            r2 = (r2 * 7 + r3) % 1000;
            r3 = (r3 * 11 + r4) % 1000;
            r4 = (r4 * 13 + r5) % 1000;
            r5 = (r5 * 17 + r6) % 1000;
            r6 = (r6 * 19 + r7) % 1000;
            r7 = (r7 * 23 + r8) % 1000;
            
            /* Force all variables live */
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
            FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
            
            if (i == 5 && jump_counter++ < 3) {
                /* longjmp creates exceptional exit edge */
                longjmp(jump_buffer, 1);
            }
        }
        
        /* Recursive call */
        int child_result = pattern_b_new_indices(depth + 1, max_depth);
        return (r0 + r1 + r2 + r3 + child_result) % 1000;
    } else {
        /* longjmp target - different computation */
        for (int i = 0; i < 5; i++) {
            r8 = (r8 * 29 + r9) % 1000;
            r9 = (r9 * 31 + r10) % 1000;
            r10 = (r10 * 37 + r11) % 1000;
            r11 = (r11 * 41 + r12) % 1000;
            r12 = (r12 * 43 + r13) % 1000;
            r13 = (r13 * 47 + r14) % 1000;
            r14 = (r14 * 53 + r15) % 1000;
            r15 = (r15 * 59 + r0) % 1000;
        }
        return (r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15) % 1000;
    }
}

/* ========== PATTERN C: MIXED PRESSURE with VECTOR OPS ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int selector, int iterations) {
    /* Scalar variables */
    int s0 = selector, s1 = selector * 2, s2 = selector * 3, s3 = selector * 4;
    int s4 = selector * 5, s5 = selector * 6, s6 = selector * 7, s7 = selector * 8;
    int s8 = selector * 9, s9 = selector * 10, s10 = selector * 11, s11 = selector * 12;
    int s12 = selector * 13, s13 = selector * 14, s14 = selector * 15, s15 = selector * 16;
    
    /* Vector variables */
#ifdef __GNUC__
    v4si v0 = {selector, selector + 1, selector + 2, selector + 3};
    v4si v1 = {selector * 2, selector * 3, selector * 4, selector * 5};
    v4si v2 = {selector * 6, selector * 7, selector * 8, selector * 9};
    v4si v3 = {selector * 10, selector * 11, selector * 12, selector * 13};
#endif
    
    int result = 0;
    
    /* Large switch with 30+ cases */
    for (int iter = 0; iter < iterations; iter++) {
        switch ((selector + iter) % 35) {
            case 0:
                s0 = (s0 * 3 + 1) % 1000;
                s1 = (s1 * 5 + 2) % 1000;
#ifdef __GNUC__
                v0 = v0 + v1;
#endif
                result += s0 + s1;
                break;
            case 1:
                s2 = (s2 * 7 + 3) % 1000;
                s3 = (s3 * 11 + 4) % 1000;
#ifdef __GNUC__
                v1 = v1 * v2;
#endif
                result += s2 + s3;
                break;
            case 2:
                s4 = (s4 * 13 + 5) % 1000;
                s5 = (s5 * 17 + 6) % 1000;
                result += s4 + s5;
                break;
            case 3:
                s6 = (s6 * 19 + 7) % 1000;
                s7 = (s7 * 23 + 8) % 1000;
#ifdef __GNUC__
                v2 = v2 - v0;
#endif
                result += s6 + s7;
                break;
            /* 31 more cases follow... */
            case 4: s8 = (s8 * 29 + 9) % 1000; result += s8; break;
            case 5: s9 = (s9 * 31 + 10) % 1000; result += s9; break;
            case 6: s10 = (s10 * 37 + 11) % 1000; result += s10; break;
            case 7: s11 = (s11 * 41 + 12) % 1000; result += s11; break;
            case 8: s12 = (s12 * 43 + 13) % 1000; result += s12; break;
            case 9: s13 = (s13 * 47 + 14) % 1000; result += s13; break;
            case 10: s14 = (s14 * 53 + 15) % 1000; result += s14; break;
            case 11: s15 = (s15 * 59 + 16) % 1000; result += s15; break;
            case 12: s0 = (s0 + s1) % 1000; result += s0; break;
            case 13: s1 = (s1 + s2) % 1000; result += s1; break;
            case 14: s2 = (s2 + s3) % 1000; result += s2; break;
            case 15: s3 = (s3 + s4) % 1000; result += s3; break;
            case 16: s4 = (s4 + s5) % 1000; result += s4; break;
            case 17: s5 = (s5 + s6) % 1000; result += s5; break;
            case 18: s6 = (s6 + s7) % 1000; result += s6; break;
            case 19: s7 = (s7 + s8) % 1000; result += s7; break;
            case 20: s8 = (s8 + s9) % 1000; result += s8; break;
            case 21: s9 = (s9 + s10) % 1000; result += s9; break;
            case 22: s10 = (s10 + s11) % 1000; result += s10; break;
            case 23: s11 = (s11 + s12) % 1000; result += s11; break;
            case 24: s12 = (s12 + s13) % 1000; result += s12; break;
            case 25: s13 = (s13 + s14) % 1000; result += s13; break;
            case 26: s14 = (s14 + s15) % 1000; result += s14; break;
            case 27: s15 = (s15 + s0) % 1000; result += s15; break;
            case 28: s0 = (s0 * 2) % 1000; result += s0; break;
            case 29: s1 = (s1 * 2) % 1000; result += s1; break;
            case 30: s2 = (s2 * 2) % 1000; result += s2; break;
            case 31: s3 = (s3 * 2) % 1000; result += s3; break;
            case 32: s4 = (s4 * 2) % 1000; result += s4; break;
            case 33: s5 = (s5 * 2) % 1000; result += s5; break;
            case 34:
                s6 = (s6 * 2) % 1000;
                s7 = (s7 * 2) % 1000;
#ifdef __GNUC__
                v3 = v0 + v1 + v2;
#endif
                result += s6 + s7;
                break;
        }
        
        /* Loop control with continue to different switch cases */
        if (iter % 7 == 0) {
            continue;
        }
        if (iter % 13 == 0) {
            selector = (selector + 1) % 35;
            continue;
        }
        
        /* Force all scalars live */
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
        
#ifdef __GNUC__
        /* Force all vectors live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    }
    
    return result % 1000;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE void dummy1(int x) { FORCE_USE(x); }
NOINLINE void dummy2(int x, int y) { FORCE_USE(x); FORCE_USE(y); }
NOINLINE void dummy3(int x, int y, int z) { FORCE_USE(x); FORCE_USE(y); FORCE_USE(z); }

NOINLINE
int pattern_d_register_conflict(int param) {
    /* Explicit register variables creating conflicts */
    REGISTER_VAR(r10_var, "r10") = param * 2;
    REGISTER_VAR(r11_var, "r11") = param * 3;
    
    int local1 = param * 4;
    int local2 = param * 5;
    int local3 = param * 6;
    int local4 = param * 7;
    int local5 = param * 8;
    int local6 = param * 9;
    int local7 = param * 10;
    int local8 = param * 11;
    
    /* Force register variable usage across calls */
    dummy1(r10_var);
    r10_var = r10_var * 3 + 1;
    
    dummy2(r11_var, local1);
    r11_var = r11_var * 5 + 2;
    
    /* Complex arithmetic spreading live ranges */
    for (int i = 0; i < 20; i++) {
        switch (i % 8) {
            case 0:
                local1 = (local1 + r10_var) % 1000;
                dummy1(local1);
                break;
            case 1:
                local2 = (local2 + r11_var) % 1000;
                dummy2(local2, r10_var);
                break;
            case 2:
                local3 = (local3 * r10_var) % 1000;
                dummy3(local3, local1, local2);
                break;
            case 3:
                local4 = (local4 * r11_var) % 1000;
                dummy1(local4);
                break;
            case 4:
                local5 = (local5 + local1 + r10_var) % 1000;
                dummy2(local5, local3);
                break;
            case 5:
                local6 = (local6 + local2 + r11_var) % 1000;
                dummy3(local6, local4, local5);
                break;
            case 6:
                local7 = (local7 * local3 + r10_var) % 1000;
                dummy1(local7);
                break;
            case 7:
                local8 = (local8 * local4 + r11_var) % 1000;
                dummy2(local8, local7);
                r10_var = (r10_var + 1) % 100;
                r11_var = (r11_var + 1) % 100;
                break;
        }
        
        /* Force all variables live periodically */
        if (i % 5 == 0) {
            FORCE_USE(r10_var); FORCE_USE(r11_var);
            FORCE_USE(local1); FORCE_USE(local2); FORCE_USE(local3); FORCE_USE(local4);
            FORCE_USE(local5); FORCE_USE(local6); FORCE_USE(local7); FORCE_USE(local8);
        }
    }
    
    return (r10_var + r11_var + local1 + local2 + local3 + 
            local4 + local5 + local6 + local7 + local8) % 1000;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    volatile int total_result = 0;
    int iterations = 1000;
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %d\n", use_avx2);
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - targets ENTRY/EXIT blocks */
        total_result += pattern_a_entry_exit(i % 100 + 1, i);
        
        /* Pattern B - targets NEW_EXIT/NEW_ENTRY */
        if (i % 10 == 0) {
            jump_counter = 0;
            total_result += pattern_b_new_indices(0, 5);
        }
        
        /* Pattern C - mixed pressure */
        total_result += pattern_c_mixed_pressure(i, 50);
        
        /* Pattern D - register conflicts */
        total_result += pattern_d_register_conflict(i);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            FORCE_USE(total_result);
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result % 1000);
    }
    
    return total_result % 1000;
}
