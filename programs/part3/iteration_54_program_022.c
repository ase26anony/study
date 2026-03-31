/* test_mcf.c - Comprehensive test to trigger MCF special node printing */
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

/* Global flag for debug output */
static volatile int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex function with irreducible region to force ENTRY/EXIT block creation */
    int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Deeply nested if-else chain */
    if (iterations > 0) {
        if (iterations > 10) {
            if (iterations > 20) {
                /* Irreducible region using goto */
                if (seed % 3 == 0) {
                    goto irreducible_label_1;
                } else if (seed % 3 == 1) {
                    goto irreducible_label_2;
                }
            } else {
                for (i = 0; i < iterations; i++) {
                    r0 += r1 * r2;
                    r3 -= r4 / (r5 + 1);
                    if (i % 7 == 0) continue;
                    r6 ^= r7 | r8;
                }
            }
        } else {
            while (iterations-- > 0) {
                r9 = r10 * r11 - r12;
                r13 = (r14 << 2) | (r15 >> 3);
                if (iterations % 5 == 0) break;
            }
        }
    }
    
    /* Irreducible region labels */
irreducible_label_1:
    r0 = r1 + r2 * r3;
    if (seed % 2 == 0) goto irreducible_label_3;
    
irreducible_label_2:
    r4 = r5 - r6 / r7;
    if (seed % 2 == 1) goto irreducible_label_1;
    
irreducible_label_3:
    r8 = r9 ^ r10 | r11;
    
    /* Complex loop with continue to different points */
    for (i = 0; i < iterations * 2; i++) {
        switch (i % 8) {
            case 0: r0 += r1; continue;
            case 1: r2 *= r3; continue;
            case 2: r4 -= r5; if (i % 3 == 0) continue; break;
            case 3: r6 ^= r7; goto irreducible_label_2;
            case 4: r8 |= r9; continue;
            case 5: r10 &= r11; break;
            case 6: r12 <<= r13; continue;
            case 7: r14 >>= r15; break;
        }
        r0 ^= r2;
    }
    
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    return result + r0 + r1 + r2 + r3;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int jump_counter = 0;

NOINLINE
int pattern_b_new_nodes(int depth, int value) {
    int result = value;
    int vars[32];
    
    /* Initialize many variables */
    for (int i = 0; i < 32; i++) {
        vars[i] = value + i * 7;
    }
    
    /* Complex arithmetic across many variables */
    for (int i = 0; i < depth; i++) {
        vars[0] = vars[1] * vars[2] - vars[3];
        vars[4] = vars[5] + vars[6] / (vars[7] + 1);
        vars[8] = vars[9] ^ vars[10] | vars[11];
        vars[12] = vars[13] << (vars[14] & 3);
        vars[16] = vars[17] - vars[18] * vars[19];
        
        /* setjmp/longjmp creates exceptional edges */
        if (i == depth / 2) {
            if (setjmp(jump_buffer) == 0) {
                /* First time through */
                for (int j = 20; j < 32; j++) {
                    vars[j] = vars[j-1] + vars[j-2];
                }
            } else {
                /* After longjmp */
                jump_counter++;
                vars[0] += 1000;
            }
        }
        
        /* Trigger longjmp under certain conditions */
        if (i > depth / 2 && vars[0] % 777 == 0) {
            longjmp(jump_buffer, 1);
        }
        
        /* More arithmetic */
        vars[1] = vars[2] + vars[3] - vars[4];
        vars[5] = vars[6] * vars[7] / (vars[8] + 1);
        vars[9] = vars[10] | vars[11] & vars[12];
    }
    
    /* Force all variables live */
    for (int i = 0; i < 32; i++) {
        FORCE_USE(vars[i]);
    }
    
    return result + vars[0] + vars[31];
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int mode, int iterations) {
    int result = mode;
    
    /* Many scalar variables */
    int s0 = mode, s1 = mode + 1, s2 = mode + 2, s3 = mode + 3;
    int s4 = mode + 4, s5 = mode + 5, s6 = mode + 6, s7 = mode + 7;
    int s8 = mode + 8, s9 = mode + 9, s10 = mode + 10, s11 = mode + 11;
    int s12 = mode + 12, s13 = mode + 13, s14 = mode + 14, s15 = mode + 15;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {mode, mode+1, mode+2, mode+3};
    v4si v1 = {mode+4, mode+5, mode+6, mode+7};
    v4si v2 = {mode+8, mode+9, mode+10, mode+11};
    v4si v3 = {mode+12, mode+13, mode+14, mode+15};
#endif
    
    /* Large switch statement with 30+ cases */
    for (int i = 0; i < iterations; i++) {
        int case_id = (mode + i) % 35;
        
        switch (case_id) {
            case 0:
                s0 = s1 + s2 * s3;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                break;
            case 1:
                s4 = s5 - s6 / (s7 + 1);
#ifdef __GNUC__
                v1 = v2 - v3;
#endif
                break;
            case 2:
                s8 = s9 ^ s10 | s11;
#ifdef __GNUC__
                v2 = v3 * v0;
#endif
                break;
            case 3:
                s12 = s13 << (s14 & 3);
#ifdef __GNUC__
                v3 = v0 & v1;
#endif
                break;
            case 4:
                s15 = s0 * s1 - s2;
#ifdef __GNUC__
                v0 = v1 | v2;
#endif
                break;
            case 5:
                s1 = s2 + s3 - s4;
                break;
            case 6:
                s5 = s6 * s7 / (s8 + 1);
                break;
            case 7:
                s9 = s10 | s11 & s12;
                break;
            case 8:
                s13 = s14 << 1 | s15 >> 1;
                break;
            case 9:
                s0 = s1 ^ s2 + s3;
                break;
            case 10:
                s4 = s5 - s6 * s7;
                break;
            case 11:
                s8 = s9 / (s10 + 1);
                break;
            case 12:
                s11 = s12 << 2;
                break;
            case 13:
                s13 = s14 >> 1;
                break;
            case 14:
                s15 = s0 & s1 | s2;
                break;
            case 15:
                s3 = s4 + s5 * s6;
                break;
            case 16:
                s7 = s8 - s9 / s10;
                break;
            case 17:
                s11 = s12 ^ s13;
                break;
            case 18:
                s14 = s15 << 3;
                break;
            case 19:
                s0 = s1 >> 2;
                break;
            case 20:
                s2 = s3 + s4 - s5;
                break;
            case 21:
                s6 = s7 * s8;
                break;
            case 22:
                s9 = s10 / (s11 + 1);
                break;
            case 23:
                s12 = s13 | s14;
                break;
            case 24:
                s15 = s0 & s1;
                break;
            case 25:
                s2 = s3 ^ s4;
                break;
            case 26:
                s5 = s6 << 1;
                break;
            case 27:
                s7 = s8 >> 2;
                break;
            case 28:
                s9 = s10 + s11 * s12;
                break;
            case 29:
                s13 = s14 - s15;
                break;
            case 30:
                s0 = s1 / (s2 + 1);
                break;
            case 31:
                s3 = s4 | s5 & s6;
                break;
            case 32:
                s7 = s8 ^ s9;
                break;
            case 33:
                s10 = s11 << (s12 & 3);
                break;
            case 34:
                s13 = s14 * s15 - s0;
                break;
        }
        
        /* Use continue/break to create complex control flow */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
        
#ifdef __GNUC__
        /* Vector operations between cases */
        if (i % 3 == 0) {
            v0 = v0 + v1;
            v2 = v2 - v3;
        }
#endif
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
    
    return result + s0 + s15;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
/* Dummy helpers for artificial conflicts */
NOINLINE COLD int helper1(register int a asm ("r10"), register int b asm ("r11")) {
    FORCE_USE(a); FORCE_USE(b);
    return a * b + 7;
}

NOINLINE COLD int helper2(register int x asm ("r10"), register int y asm ("r12")) {
    FORCE_USE(x); FORCE_USE(y);
    return x - y * 3;
}

NOINLINE COLD int helper3(register int p asm ("r11"), register int q asm ("r13")) {
    FORCE_USE(p); FORCE_USE(q);
    return p ^ q | 0xFF;
}
#endif

NOINLINE
int pattern_d_artificial_conflict(int base, int count) {
    int result = base;
    
#ifdef __GNUC__
    /* Explicit register variables with conflicts */
    register int r10_var asm ("r10") = base;
    register int r11_var asm ("r11") = base + 1;
    register int r12_var asm ("r12") = base + 2;
    register int r13_var asm ("r13") = base + 3;
    
    /* Many local variables to increase pressure */
    int locals[20];
    for (int i = 0; i < 20; i++) {
        locals[i] = base + i * 5;
    }
    
    /* Complex loop with register-conflicting calls */
    for (int i = 0; i < count; i++) {
        /* Artificial conflicts by calling helpers with different register args */
        switch (i % 7) {
            case 0:
                r10_var = helper1(r10_var, r11_var);
                locals[0] += r10_var;
                break;
            case 1:
                r11_var = helper2(r10_var, r12_var);
                locals[1] -= r11_var;
                break;
            case 2:
                r12_var = helper3(r11_var, r13_var);
                locals[2] ^= r12_var;
                break;
            case 3:
                r10_var = helper1(r12_var, r13_var);
                locals[3] |= r10_var;
                break;
            case 4:
                r11_var = helper2(r13_var, r10_var);
                locals[4] &= r11_var;
                break;
            case 5:
                r12_var = helper3(r10_var, r11_var);
                locals[5] <<= (r12_var & 3);
                break;
            case 6:
                r13_var = helper1(r11_var, r12_var);
                locals[6] >>= (r13_var & 3);
                break;
        }
        
        /* More arithmetic creating live range splits */
        locals[7] = locals[0] + locals[1] - locals[2];
        locals[8] = locals[3] * locals[4] / (locals[5] + 1);
        locals[9] = locals[6] ^ locals[7] | locals[8];
        
        if (i % 11 == 0) {
            r10_var = locals[9] + locals[10];
            r11_var = locals[11] - locals[12];
        }
        
        if (i % 13 == 0) {
            r12_var = locals[13] * locals[14];
            r13_var = locals[15] ^ locals[16];
        }
    }
    
    /* Force all variables live */
    FORCE_USE(r10_var); FORCE_USE(r11_var); FORCE_USE(r12_var); FORCE_USE(r13_var);
    for (int i = 0; i < 20; i++) {
        FORCE_USE(locals[i]);
    }
    
    result = r10_var + r11_var + r12_var + r13_var;
#endif
    
    return result;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call all pattern functions multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(i % 50 + 10, i * 3);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 37 == 0) {
            jump_counter = 0;
            total += pattern_b_new_nodes(i % 20 + 5, i * 7);
        }
        
        /* Pattern C - Mixed pressure */
        total += pattern_c_mixed_pressure(i % 10, i % 30 + 5);
        
        /* Pattern D - Artificial conflict */
#ifdef __GNUC__
        total += pattern_d_artificial_conflict(i, i % 40 + 3);
#endif
        
        /* Prevent optimization */
        if (total > 1000000) total = total % 1000000;
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
        printf("Jump counter: %d\n", jump_counter);
    }
    
    /* Ensure result is used */
    FORCE_USE(total);
    
    return total == 0 ? 1 : 0;
}
