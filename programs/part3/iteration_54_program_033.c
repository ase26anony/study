/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
/* Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf.c */
/* Then run: ./test_mcf_gen && gcc -O2 -fprofile-use -fprofile-arcs -ftest-coverage test_mcf.c */

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define USED __attribute__((used))
#define KEEP(var) asm volatile("" : : "g"(var))
#else
#define NOINLINE
#define HOT
#define USED
#define KEEP(var) (void)(var)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS with irreducible region ========== */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create complex entry block with many variables */
    int a0 = seed * 1, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    int a12 = seed * 13, a13 = seed * 14, a14 = seed * 15, a15 = seed * 16;
    
    /* Labeled block to encourage ENTRY_BLOCK identification */
    hot_entry_label:
    if (verbose) printf("Pattern A hot entry\n");
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain creating many basic blocks */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    a0 = a1 + a2;
                    a3 = a4 - a5;
                } else {
                    a6 = a7 * a8;
                    a9 = a10 / (a11 ? a11 : 1);
                }
            } else {
                if (i & 8) {
                    a12 = a13 ^ a14;
                    a15 = a0 | a1;
                } else {
                    a2 = a3 & a4;
                    a5 = a6 << 2;
                }
            }
        } else {
            if (i & 16) {
                a7 = a8 >> 1;
                a9 = a10 % (a11 ? a11 : 1);
            } else {
                a12 = ~a13;
                a14 = a15 * 3;
            }
        }
        
        /* Irreducible region using goto */
        if (i % 7 == 0) {
            goto irreducible_region;
        }
        
        /* Loop with continue to different points */
        for (j = 0; j < 5; j++) {
            if ((i + j) % 3 == 0) {
                continue;
            }
            
            /* Another level of nesting */
            for (k = 0; k < 3; k++) {
                if ((i + j + k) % 2 == 0) {
                    a0 += k;
                    continue;
                } else {
                    a1 -= k;
                    break;
                }
            }
            
            if (j == 2) {
                goto hot_entry_label; /* Back-edge to entry */
            }
        }
        
        irreducible_region:
        /* Complex arithmetic to keep variables live */
        a0 = a1 * a2 + a3;
        a4 = a5 - a6 * a7;
        a8 = a9 ^ a10 | a11;
        a12 = a13 & a14 + a15;
        
        KEEP(a0); KEEP(a1); KEEP(a2); KEEP(a3);
        KEEP(a4); KEEP(a5); KEEP(a6); KEEP(a7);
        KEEP(a8); KEEP(a9); KEEP(a10); KEEP(a11);
        KEEP(a12); KEEP(a13); KEEP(a14); KEEP(a15);
    }
    
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + 
             a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
    KEEP(result);
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp/longjmp ========== */
static jmp_buf jump_buffer;
static volatile int jump_counter = 0;

NOINLINE USED
int helper_b1(int x) { return x * 3 + 1; }
NOINLINE USED
int helper_b2(int x) { return x / 2 - 1; }
NOINLINE USED
int helper_b3(int x) { return x ^ 0x55AA; }

HOT NOINLINE USED
int pattern_b_new_nodes(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Many scalar variables to pressure registers */
    int b0 = seed, b1 = seed + 1, b2 = seed + 2, b3 = seed + 3;
    int b4 = seed + 4, b5 = seed + 5, b6 = seed + 6, b7 = seed + 7;
    int b8 = seed + 8, b9 = seed + 9, b10 = seed + 10, b11 = seed + 11;
    int b12 = seed + 12, b13 = seed + 13, b14 = seed + 14, b15 = seed + 15;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal execution path */
        for (i = 0; i < iterations; i++) {
            /* Complex arithmetic across all variables */
            b0 = helper_b1(b0) + b1;
            b1 = helper_b2(b1) - b2;
            b2 = helper_b3(b2) ^ b3;
            b3 = b4 * b5 + b6;
            b4 = b7 / (b8 ? b8 : 1) - b9;
            b5 = b10 << (b11 & 3);
            b6 = b12 >> (b13 % 4);
            b7 = b14 | b15;
            b8 = b0 & b1;
            b9 = b2 + b3 * b4;
            b10 = b5 - b6 / (b7 ? b7 : 1);
            b11 = b8 ^ b9 | b10;
            b12 = b11 & b12 + b13;
            b13 = b14 * 3 - b15;
            b14 = helper_b1(b0) ^ helper_b2(b1);
            b15 = helper_b3(b2) | helper_b1(b3);
            
            /* Force register pressure */
            KEEP(b0); KEEP(b1); KEEP(b2); KEEP(b3);
            KEEP(b4); KEEP(b5); KEEP(b6); KEEP(b7);
            KEEP(b8); KEEP(b9); KEEP(b10); KEEP(b11);
            KEEP(b12); KEEP(b13); KEEP(b14); KEEP(b15);
            
            /* Occasionally trigger longjmp to create exceptional edge */
            if (i % 13 == 0 && jump_counter < 5) {
                jump_counter++;
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* longjmp target - different execution path */
        b0 = b0 ^ 0xFFFF;
        b1 = b1 * 2;
        b2 = b2 + 1000;
        b3 = b3 - 500;
    }
    
    result = b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + 
             b8 + b9 + b10 + b11 + b12 + b13 + b14 + b15;
    KEEP(result);
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector operations ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

HOT NOINLINE USED
int pattern_c_mixed_pressure(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Scalar variables */
    int c0 = seed, c1 = seed * 2, c2 = seed * 3, c3 = seed * 4;
    int c4 = seed * 5, c5 = seed * 6, c6 = seed * 7, c7 = seed * 8;
    int c8 = seed * 9, c9 = seed * 10, c10 = seed * 11, c11 = seed * 12;
    int c12 = seed * 13, c13 = seed * 14, c14 = seed * 15, c15 = seed * 16;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v1 = {seed + 4, seed + 5, seed + 6, seed + 7};
    v4si v2 = {seed + 8, seed + 9, seed + 10, seed + 11};
    v4si v3 = {seed + 12, seed + 13, seed + 14, seed + 15};
#endif
    
    /* Large switch statement with 30+ cases */
    for (i = 0; i < iterations; i++) {
        switch (i % 35) {  /* 35 cases to exceed 30 */
            case 0:
                c0 = c1 + c2; c3 = c4 - c5;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                if (i % 3 == 0) continue;
                break;
            case 1:
                c6 = c7 * c8; c9 = c10 / (c11 ? c11 : 1);
#ifdef __GNUC__
                v1 = v2 * v3;
#endif
                if (i % 5 == 0) break;
                continue;
            case 2:
                c12 = c13 ^ c14; c15 = c0 | c1;
#ifdef __GNUC__
                v2 = v3 ^ v0;
#endif
                break;
            case 3:
                c2 = c3 & c4; c5 = c6 << 2;
#ifdef __GNUC__
                v3 = v0 & v1;
#endif
                if (i % 7 == 0) continue;
                break;
            case 4:
                c7 = c8 >> 1; c9 = c10 % (c11 ? c11 : 1);
#ifdef __GNUC__
                v0 = v1 >> 2;
#endif
                break;
            case 5:
                c12 = ~c13; c14 = c15 * 3;
#ifdef __GNUC__
                v1 = ~v2;
#endif
                break;
            /* 29 more cases with unique arithmetic patterns */
            case 6: c0 = c1 * 3 + c2; c3 = c4 / 2 - c5; break;
            case 7: c6 = c7 ^ 0xAA + c8; c9 = c10 | 0x55 - c11; break;
            case 8: c12 = c13 << (c14 & 3); c15 = c0 >> (c1 % 4); break;
            case 9: c2 = c3 + c4 * c5; c6 = c7 - c8 / (c9 ? c9 : 1); break;
            case 10: c10 = c11 & c12 | c13; c14 = c15 ^ c0 + c1; break;
            case 11: c3 = c4 * 5 - c5; c6 = c7 / 3 + c8; break;
            case 12: c9 = c10 ^ c11 & c12; c13 = c14 | c15 - c0; break;
            case 13: c1 = c2 << 1; c3 = c4 >> 2; c5 = c6 * 7; break;
            case 14: c7 = c8 / 5; c9 = c10 % 11; c11 = c12 + 13; break;
            case 15: c13 = c14 - 17; c15 = c0 ^ 19; c1 = c2 | 23; break;
            case 16: c3 = c4 & 29; c5 = c6 * 31; c7 = c8 / 37; break;
            case 17: c9 = c10 % 41; c11 = c12 + 43; c13 = c14 - 47; break;
            case 18: c15 = c0 ^ 53; c1 = c2 | 59; c3 = c4 & 61; break;
            case 19: c5 = c6 * 67; c7 = c8 / 71; c9 = c10 % 73; break;
            case 20: c11 = c12 + 79; c13 = c14 - 83; c15 = c0 ^ 89; break;
            case 21: c1 = c2 | 97; c3 = c4 & 101; c5 = c6 * 103; break;
            case 22: c7 = c8 / 107; c9 = c10 % 109; c11 = c12 + 113; break;
            case 23: c13 = c14 - 127; c15 = c0 ^ 131; c1 = c2 | 137; break;
            case 24: c3 = c4 & 139; c5 = c6 * 149; c7 = c8 / 151; break;
            case 25: c9 = c10 % 157; c11 = c12 + 163; c13 = c14 - 167; break;
            case 26: c15 = c0 ^ 173; c1 = c2 | 179; c3 = c4 & 181; break;
            case 27: c5 = c6 * 191; c7 = c8 / 193; c9 = c10 % 197; break;
            case 28: c11 = c12 + 199; c13 = c14 - 211; c15 = c0 ^ 223; break;
            case 29: c1 = c2 | 227; c3 = c4 & 229; c5 = c6 * 233; break;
            case 30: c7 = c8 / 239; c9 = c10 % 241; c11 = c12 + 251; break;
            case 31: c13 = c14 - 257; c15 = c0 ^ 263; c1 = c2 | 269; break;
            case 32: c3 = c4 & 271; c5 = c6 * 277; c7 = c8 / 281; break;
            case 33: c9 = c10 % 283; c11 = c12 + 293; c13 = c14 - 307; break;
            case 34:
                c15 = c0 ^ 311; c1 = c2 | 313; c3 = c4 & 317;
                if (i % 11 == 0) break;
                continue;
        }
        
        /* Mix vector and scalar operations */
#ifdef __GNUC__
        v0 = v0 + v1 * v2;
        v3 = v3 - v0 / (v1 + 1);
        /* Extract elements to scalars */
        c0 += v0[0]; c1 += v0[1]; c2 += v0[2]; c3 += v0[3];
        c4 += v1[0]; c5 += v1[1]; c6 += v1[2]; c7 += v1[3];
#endif
        
        KEEP(c0); KEEP(c1); KEEP(c2); KEEP(c3);
        KEEP(c4); KEEP(c5); KEEP(c6); KEEP(c7);
        KEEP(c8); KEEP(c9); KEEP(c10); KEEP(c11);
        KEEP(c12); KEEP(c13); KEEP(c14); KEEP(c15);
#ifdef __GNUC__
        KEEP(v0); KEEP(v1); KEEP(v2); KEEP(v3);
#endif
    }
    
    result = c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + 
             c8 + c9 + c10 + c11 + c12 + c13 + c14 + c15;
    KEEP(result);
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT with register variables ========== */
NOINLINE USED int dummy1(int x) { return x + 1; }
NOINLINE USED int dummy2(int x) { return x * 2; }
NOINLINE USED int dummy3(int x) { return x ^ 0xFF; }
NOINLINE USED int dummy4(int x) { return x - 5; }
NOINLINE USED int dummy5(int x) { return x / 3; }

HOT NOINLINE USED
int pattern_d_register_conflict(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
#ifdef __GNUC__
    /* Explicit register variables creating artificial conflicts */
    register int r0 asm ("r10") = seed;
    register int r1 asm ("r11") = seed + 1;
    register int r2 asm ("r12") = seed + 2;
    register int r3 asm ("r13") = seed + 3;
    register int r4 asm ("r14") = seed + 4;
    register int r5 asm ("r15") = seed + 5;
#else
    int r0 = seed, r1 = seed + 1, r2 = seed + 2;
    int r3 = seed + 3, r4 = seed + 4, r5 = seed + 5;
#endif
    
    int d0 = seed * 2, d1 = seed * 3, d2 = seed * 4, d3 = seed * 5;
    int d4 = seed * 6, d5 = seed * 7, d6 = seed * 8, d7 = seed * 9;
    int d8 = seed * 10, d9 = seed * 11, d10 = seed * 12, d11 = seed * 13;
    int d12 = seed * 14, d13 = seed * 15, d14 = seed * 16, d15 = seed * 17;
    
    for (i = 0; i < iterations; i++) {
        /* Force conflicts by using register variables in complex expressions */
        r0 = dummy1(r0) + r1;
        r1 = dummy2(r1) - r2;
        r2 = dummy3(r2) ^ r3;
        r3 = dummy4(r3) | r4;
        r4 = dummy5(r4) & r5;
        r5 = dummy1(r5) * r0;
        
        /* Mix with regular variables */
        d0 = r0 + d1;
        d1 = r1 - d2;
        d2 = r2 ^ d3;
        d3 = r3 | d4;
        d4 = r4 & d5;
        d5 = r5 * d6;
        
        /* More complex operations */
        d6 = dummy2(d6) + dummy3(d7);
        d7 = dummy4(d7) - dummy5(d8);
        d8 = dummy1(d8) ^ dummy2(d9);
        d9 = dummy3(d9) | dummy4(d10);
        d10 = dummy5(d10) & dummy1(d11);
        d11 = dummy2(d11) * dummy3(d12);
        d12 = dummy4(d12) / (dummy5(d13) ? dummy5(d13) : 1);
        d13 = dummy1(d13) % (dummy2(d14) ? dummy2(d14) : 1);
        d14 = dummy3(d14) << (dummy4(d15) & 3);
        d15 = dummy5(d15) >> (dummy1(d0) % 4);
        
        /* Force all variables to be considered live */
        KEEP(r0); KEEP(r1); KEEP(r2); KEEP(r3); KEEP(r4); KEEP(r5);
        KEEP(d0); KEEP(d1); KEEP(d2); KEEP(d3); KEEP(d4); KEEP(d5);
        KEEP(d6); KEEP(d7); KEEP(d8); KEEP(d9); KEEP(d10); KEEP(d11);
        KEEP(d12); KEEP(d13); KEEP(d14); KEEP(d15);
        
        /* Conditional that creates divergent paths */
        if (i % 17 == 0) {
            /* Call dummy functions in different order */
            r0 = dummy5(r0);
            r1 = dummy4(r1);
            r2 = dummy3(r2);
            r3 = dummy2(r3);
            r4 = dummy1(r4);
            r5 = dummy5(r5);
        } else if (i % 19 == 0) {
            d0 = dummy1(d0);
            d1 = dummy2(d1);
            d2 = dummy3(d2);
            d3 = dummy4(d3);
            d4 = dummy5(d4);
            d5 = dummy1(d5);
        }
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + 
             d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + 
             d8 + d9 + d10 + d11 + d12 + d13 + d14 + d15;
    KEEP(result);
    return result;
}

/* ========== MAIN FUNCTION with PGO support ========== */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations = 1000;
    
    if (argc > 1) {
        verbose = atoi(argv[1]);
        if (argc > 2) {
            iterations = atoi(argv[2]);
        }
    }
    
    /* Check for AVX2 to engage target-specific register allocation */
    int has_avx2 = 0;
#ifdef __GNUC__
    has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", has_avx2 ? "yes" : "no");
    }
#endif
    
    /* Run each pattern multiple times with different seeds */
    for (i = 0; i < 10; i++) {
        if (verbose) printf("Iteration %d:\n", i);
        
        int seed = i * 12345 + 6789;
        
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(iterations / 10, seed);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        total += pattern_b_new_nodes(iterations / 20, seed + 1);
        
        /* Pattern C - Mixed pressure with vectors */
        total += pattern_c_mixed_pressure(iterations / 5, seed + 2);
        
        /* Pattern D - Register conflicts */
        total += pattern_d_register_conflict(iterations / 8, seed + 3);
        
        KEEP(total);
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    /* Ensure result is used */
    KEEP(total);
    return total != 0 ? 0 : 1;
}
