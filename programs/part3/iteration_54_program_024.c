/* test_mcf.c - Complex CFG generator for GCC MCF pass testing */
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define USED __attribute__((used))
#else
#define NOINLINE
#define HOT
#define USED
#endif

/* Global to prevent optimization */
volatile int global_sink = 0;
static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex function with irreducible region to force ENTRY/EXIT block creation */
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables for register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Irreducible region using goto */
    if (iterations > 0) {
        goto middle_of_loop;
    }
    
start_loop:
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (r0 & 1) {
            if (r1 & 2) {
                for (j = 0; j < 5; j++) {
                    if (r2 & (1 << j)) {
                        r3 += r4 * j;
                        if (r3 > 1000) {
                            r5 = r6 ^ r7;
                            goto middle_of_loop;
                        }
                    } else {
                        r8 = r9 - r10;
                        continue;
                    }
                }
            } else if (r11 < r12) {
                r13 = r14 | r15;
                break;
            }
        } else {
            r0 = r1 * r2;
        }
        
        /* Another level of nesting */
        switch (r0 % 7) {
            case 0: r1 = r2 + r3; break;
            case 1: r2 = r3 - r4; break;
            case 2: r3 = r4 * r5; break;
            case 3: r4 = r5 / (r6 ? r6 : 1); break;
            case 4: r5 = r6 ^ r7; break;
            case 5: r6 = r7 | r8; break;
            case 6: r7 = r8 & r9; break;
        }
        
middle_of_loop:
        /* More computations */
        r8 = r9 << (r10 & 3);
        r9 = r10 >> (r11 & 3);
        
        if (i & 1) {
            goto start_loop;
        }
        
        /* Prevent dead code elimination */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), 
                     "g"(r4), "g"(r5), "g"(r6), "g"(r7),
                     "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                     "g"(r12), "g"(r13), "g"(r14), "g"(r15));
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
NOINLINE USED
int pattern_b_new_exit_entry(int depth, int value) {
    volatile int result = value;
    int i, j;
    
    /* Many local variables */
    int v0 = value, v1 = value + 1, v2 = value + 2, v3 = value + 3;
    int v4 = value + 4, v5 = value + 5, v6 = value + 6, v7 = value + 7;
    int v8 = value + 8, v9 = value + 9, v10 = value + 10, v11 = value + 11;
    int v12 = value + 12, v13 = value + 13, v14 = value + 14, v15 = value + 15;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal execution path */
        for (i = 0; i < depth; i++) {
            /* Complex loop with many variables */
            v0 = v1 * v2 + v3;
            v1 = v2 - v3 * v4;
            v2 = v3 / (v4 ? v4 : 1) + v5;
            v3 = v4 ^ v5 | v6;
            v4 = v5 & v6 << v7;
            v5 = v6 >> v8 + v9;
            v6 = v7 * v8 - v9;
            v7 = v8 + v9 / (v10 ? v10 : 1);
            v8 = v9 ^ v10 & v11;
            v9 = v10 | v11 << v12;
            v10 = v11 >> v13 + v14;
            v11 = v12 * v13 - v14;
            v12 = v13 + v14 * v15;
            v13 = v14 - v15 / (v0 ? v0 : 1);
            v14 = v15 ^ v0 | v1;
            v15 = v0 & v1 + v2;
            
            /* Create exceptional edge with longjmp */
            if (v0 > 1000000 && i > depth/2) {
                longjmp(jump_buffer, 1);
            }
            
            /* Prevent optimization */
            asm volatile("" : : "g"(v0), "g"(v1), "g"(v2), "g"(v3),
                         "g"(v4), "g"(v5), "g"(v6), "g"(v7),
                         "g"(v8), "g"(v9), "g"(v10), "g"(v11),
                         "g"(v12), "g"(v13), "g"(v14), "g"(v15));
        }
    } else {
        /* longjmp target - different execution path */
        for (j = 0; j < 10; j++) {
            v0 = v1 + v2;
            v1 = v2 - v3;
            v2 = v3 * v4;
            v3 = v4 ^ v5;
            /* ... continue with all variables ... */
        }
    }
    
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
             v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int selector, int iterations) {
    volatile int result = 0;
    int i;
    
    /* Scalar variables */
    int s0 = selector, s1 = selector + 1, s2 = selector + 2, s3 = selector + 3;
    int s4 = selector + 4, s5 = selector + 5, s6 = selector + 6, s7 = selector + 7;
    int s8 = selector + 8, s9 = selector + 9, s10 = selector + 10, s11 = selector + 11;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {selector, selector+1, selector+2, selector+3};
    v4si v1 = {selector+4, selector+5, selector+6, selector+7};
    v4si v2 = {selector+8, selector+9, selector+10, selector+11};
    v4si v3 = {selector+12, selector+13, selector+14, selector+15};
#endif
    
    /* Large switch statement with 30+ cases */
    for (i = 0; i < iterations; i++) {
        switch ((s0 + i) % 35) {
            case 0:
                s0 = s1 + s2;
                s1 = s2 * s3;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                break;
            case 1:
                s2 = s3 - s4;
                s3 = s4 / (s5 ? s5 : 1);
#ifdef __GNUC__
                v1 = v2 - v3;
#endif
                break;
            case 2:
                s4 = s5 ^ s6;
                s5 = s6 | s7;
#ifdef __GNUC__
                v2 = v3 * v0;
#endif
                break;
            case 3:
                s6 = s7 & s8;
                s7 = s8 << (s9 & 3);
#ifdef __GNUC__
                v3 = v0 & v1;
#endif
                break;
            case 4:
                s8 = s9 >> (s10 & 3);
                s9 = s10 + s11;
#ifdef __GNUC__
                v0 = v1 | v2;
#endif
                break;
            case 5:
                s10 = s11 * s0;
                s11 = s0 - s1;
#ifdef __GNUC__
                v1 = v2 ^ v3;
#endif
                break;
            /* 29 more cases with unique arithmetic patterns */
            case 6: s0 = s1 * s2 + s3; s1 = s4 - s5; break;
            case 7: s2 = s3 / (s6 ? s6 : 1); s3 = s7 ^ s8; break;
            case 8: s4 = s5 | s9; s5 = s10 & s11; break;
            case 9: s6 = s7 << 1; s7 = s8 >> 2; break;
            case 10: s8 = s9 + s0 * 3; s9 = s1 - s2 / 4; break;
            case 11: s10 = s11 ^ s3; s11 = s4 | s5; break;
            case 12: s0 = s1 & s6; s1 = s7 << (s8 & 1); break;
            case 13: s2 = s9 >> (s10 & 2); s3 = s11 + s0; break;
            case 14: s4 = s1 * s2 - s3; s5 = s4 / (s5 ? s5 : 2); break;
            case 15: s6 = s7 ^ s8 | s9; s7 = s10 & s11 << 1; break;
            case 16: s8 = s0 >> (s1 & 3) + s2; s9 = s3 * s4 - s5; break;
            case 17: s10 = s6 + s7 / (s8 ? s8 : 1); s11 = s9 ^ s10; break;
            case 18: s0 = s11 | s0 & s1; s1 = s2 << s3; break;
            case 19: s2 = s4 >> s5 + s6; s3 = s7 * s8 ^ s9; break;
            case 20: s4 = s10 - s11 * s0; s5 = s1 / (s2 ? s2 : 3); break;
            case 21: s6 = s3 | s4 & s5; s7 = s6 << (s7 & 2); break;
            case 22: s8 = s8 >> (s9 & 1) - s10; s9 = s11 + s0 * s1; break;
            case 23: s10 = s2 - s3 / (s4 ? s4 : 4); s11 = s5 ^ s6 | s7; break;
            case 24: s0 = s8 & s9 << s10; s1 = s11 >> s0 + s1; break;
            case 25: s2 = s2 * s3 - s4; s3 = s5 / (s6 ? s6 : 5); break;
            case 26: s4 = s7 ^ s8 & s9; s5 = s10 | s11 << s0; break;
            case 27: s6 = s1 >> s2 * s3; s7 = s4 - s5 + s6; break;
            case 28: s8 = s7 / (s8 ? s8 : 6) ^ s9; s9 = s10 & s11 | s0; break;
            case 29: s10 = s1 << s2 - s3; s11 = s4 * s5 / (s6 ? s6 : 7); break;
            case 30: s0 = s7 ^ s8 | s9 & s10; s1 = s11 << s0 >> s1; break;
            case 31: s2 = s2 + s3 * s4 - s5; s3 = s6 / (s7 ? s7 : 8) + s8; break;
            case 32: s4 = s9 ^ s10 & s11 | s0; s5 = s1 << (s2 & 3) >> s3; break;
            case 33: s6 = s4 + s5 - s6 * s7; s7 = s8 / (s9 ? s9 : 9) ^ s10; break;
            case 34:
                s8 = s11 & s0 | s1 << s2;
                s9 = s3 >> s4 + s5 * s6;
                /* Use continue to create complex back-edge */
                if (i < iterations - 1) continue;
                break;
        }
        
        /* Prevent optimization for all variables */
        asm volatile("" : : "g"(s0), "g"(s1), "g"(s2), "g"(s3),
                     "g"(s4), "g"(s5), "g"(s6), "g"(s7),
                     "g"(s8), "g"(s9), "g"(s10), "g"(s11));
#ifdef __GNUC__
        asm volatile("" : : "g"(v0), "g"(v1), "g"(v2), "g"(v3));
#endif
    }
    
#ifdef __GNUC__
    /* Extract results from vectors */
    int v0_0 = v0[0], v0_1 = v0[1], v0_2 = v0[2], v0_3 = v0[3];
    int v1_0 = v1[0], v1_1 = v1[1], v1_2 = v1[2], v1_3 = v1[3];
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11 +
             v0_0 + v0_1 + v0_2 + v0_3 + v1_0 + v1_1 + v1_2 + v1_3;
#else
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11;
#endif
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
/* Dummy helper functions */
NOINLINE int helper1(register int a asm ("r10"), register int b asm ("r11")) {
    return a * b + 1;
}

NOINLINE int helper2(register int a asm ("r10"), register int b asm ("r12")) {
    return a - b * 2;
}

NOINLINE int helper3(register int a asm ("r11"), register int b asm ("r12")) {
    return a | b & 3;
}
#endif

NOINLINE USED
int pattern_d_artificial_conflict(int x, int y) {
    volatile int result = 0;
    int i;
    
#ifdef __GNUC__
    /* Explicit register variables creating conflicts */
    register int r10_var asm ("r10") = x;
    register int r11_var asm ("r11") = y;
    register int r12_var asm ("r12") = x + y;
    
    /* Many other local variables */
    int v0 = x, v1 = y, v2 = x * y, v3 = x + y;
    int v4 = x - y, v5 = x ^ y, v6 = x | y, v7 = x & y;
    int v8 = x << 1, v9 = y >> 2, v10 = x * 3, v11 = y / 4;
    
    /* Complex loop with register conflicts */
    for (i = 0; i < 100; i++) {
        /* Call helpers with conflicting register assignments */
        v0 = helper1(r10_var, r11_var);
        v1 = helper2(r10_var, r12_var);
        v2 = helper3(r11_var, r12_var);
        
        /* Modify register variables */
        r10_var = v0 + v1;
        r11_var = v1 - v2;
        r12_var = v2 * v0;
        
        /* More computations */
        v3 = v0 * v1 + v2;
        v4 = v1 - v2 * v3;
        v5 = v2 / (v3 ? v3 : 1) + v4;
        v6 = v3 ^ v4 | v5;
        v7 = v4 & v5 << v6;
        v8 = v5 >> v7 + v8;
        v9 = v6 * v7 - v8;
        v10 = v7 + v8 / (v9 ? v9 : 1);
        v11 = v8 ^ v9 & v10;
        
        /* Force all variables live */
        asm volatile("" : : "g"(v0), "g"(v1), "g"(v2), "g"(v3),
                     "g"(v4), "g"(v5), "g"(v6), "g"(v7),
                     "g"(v8), "g"(v9), "g"(v10), "g"(v11),
                     "g"(r10_var), "g"(r11_var), "g"(r12_var));
    }
    
    result = r10_var + r11_var + r12_var + v0 + v1 + v2 + v3 + v4 + 
             v5 + v6 + v7 + v8 + v9 + v10 + v11;
#else
    result = x + y;
#endif
    
    return result;
}

/* ========== MAIN FUNCTION with PGO support ========== */
int main(int argc, char *argv[]) {
    int i, total = 0;
    
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %d\n", use_avx2);
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < 100; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(i % 20 + 5, i * 3);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        total += pattern_b_new_exit_entry(i % 10 + 3, i * 7 + 1);
        
        /* Pattern C - Mixed pressure */
        total += pattern_c_mixed_pressure(i % 50, i % 15 + 10);
        
        /* Pattern D - Artificial conflict */
        total += pattern_d_artificial_conflict(i * 2, i * 3 + 1);
        
        /* Prevent loop optimization */
        if (i % 37 == 0) {
            asm volatile("" : : "g"(total));
        }
    }
    
    /* Store result in global volatile to prevent optimization */
    global_sink = total;
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return 0;
}
