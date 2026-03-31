/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage testing */
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

/* Global verbose flag - set to 1 to enable debug output */
static volatile int verbose = 0;

/* ============================================
   PATTERN A: ENTRY/EXIT BLOCKS with irreducible region
   ============================================ */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = 0;
    int i, j, k;
    
    /* Many local variables to create register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Create irreducible region using goto */
    if (iterations > 100) {
        goto irreducible_region;
    }
    
    /* Deeply nested if-else chain */
    for (i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            if (i % 5 == 0) {
                for (j = 0; j < 10; j++) {
                    if (j % 2 == 0) {
                        r0 = r1 + r2;
                        r3 = r4 * r5;
                        r6 = r7 - r8;
                        r9 = r10 ^ r11;
                        r12 = r13 | r14;
                        r15 = r0 ^ r3;
                    } else {
                        r1 = r2 + r3;
                        r4 = r5 * r6;
                        r7 = r8 - r9;
                        r10 = r11 ^ r12;
                        r13 = r14 | r15;
                        r0 = r1 ^ r4;
                    }
                    /* Force variable liveness */
                    asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3),
                                       "g"(r4), "g"(r5), "g"(r6), "g"(r7),
                                       "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                                       "g"(r12), "g"(r13), "g"(r14), "g"(r15));
                }
            } else if (i % 5 == 1) {
                r0 = r1 * r2 + r3;
                r4 = r5 - r6 * r7;
                r8 = r9 ^ r10 | r11;
                r12 = r13 + r14 - r15;
            }
        } else if (i % 3 == 1) {
            for (k = 0; k < 5; k++) {
                r0 += k;
                r1 -= k;
                r2 *= (k + 1);
                r3 ^= k;
            }
        }
        
irreducible_region:
        /* This creates an irreducible loop */
        if (i % 7 == 0) {
            r0 = r1;
            goto middle_of_loop;
        } else if (i % 7 == 1) {
            r1 = r2;
            goto end_of_loop;
        }
        
middle_of_loop:
        r2 = r3;
        if (i % 2 == 0) {
            continue;
        }
        
end_of_loop:
        r3 = r4;
        result += r0 + r1 + r2 + r3;
    }
    
    return result + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
}

/* ============================================
   PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp/longjmp
   ============================================ */
static jmp_buf jump_buffer;
static volatile int jump_counter = 0;

NOINLINE USED
int pattern_b_new_exit_entry(int depth, int max_depth) {
    volatile int result = 0;
    int i, j;
    
    /* Many scalar variables */
    int s0 = depth, s1 = depth * 2, s2 = depth * 3, s3 = depth * 4;
    int s4 = depth * 5, s5 = depth * 6, s6 = depth * 7, s7 = depth * 8;
    int s8 = depth * 9, s9 = depth * 10, s10 = depth * 11, s11 = depth * 12;
    int s12 = depth * 13, s13 = depth * 14, s14 = depth * 15, s15 = depth * 16;
    
    if (depth >= max_depth) {
        return s0 + s1;
    }
    
    /* setjmp creates exceptional control flow */
    int ret = setjmp(jump_buffer);
    if (ret == 0) {
        /* Normal path */
        for (i = 0; i < 100; i++) {
            /* Complex arithmetic to create many basic blocks */
            switch (i % 13) {
                case 0: s0 = s1 + s2; s3 = s4 * s5; break;
                case 1: s1 = s2 - s3; s4 = s5 / (s6 + 1); break;
                case 2: s2 = s3 ^ s4; s5 = s6 | s7; break;
                case 3: s3 = s4 & s5; s6 = s7 << 2; break;
                case 4: s4 = s5 >> 1; s7 = s8 + s9; break;
                case 5: s5 = s6 * s7; s8 = s9 - s10; break;
                case 6: s6 = s7 / 2; s9 = s10 ^ s11; break;
                case 7: s7 = s8 | s9; s10 = s11 & s12; break;
                case 8: s8 = s9 << 3; s11 = s12 + s13; break;
                case 9: s9 = s10 >> 2; s12 = s13 * s14; break;
                case 10: s10 = s11 - s12; s13 = s14 / 3; break;
                case 11: s11 = s12 ^ s13; s14 = s15 | s0; break;
                case 12: s12 = s13 & s14; s15 = s0 << 1; break;
            }
            
            /* Force liveness */
            asm volatile("" : : "g"(s0), "g"(s1), "g"(s2), "g"(s3),
                               "g"(s4), "g"(s5), "g"(s6), "g"(s7),
                               "g"(s8), "g"(s9), "g"(s10), "g"(s11),
                               "g"(s12), "g"(s13), "g"(s14), "g"(s15));
            
            /* Occasionally longjmp to create exceptional edge */
            if (i == 50 && jump_counter < 3) {
                jump_counter++;
                longjmp(jump_buffer, 1);
            }
        }
        
        /* Recursive call for deeper CFG */
        result = pattern_b_new_exit_entry(depth + 1, max_depth);
    } else {
        /* longjmp return path */
        for (j = 0; j < 20; j++) {
            s0 += s1;
            s1 -= s2;
            s2 *= s3;
            s3 ^= s4;
            s4 |= s5;
            s5 &= s6;
            s6 <<= 1;
            s7 >>= 1;
        }
    }
    
    return result + s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
}

/* ============================================
   PATTERN C: MIXED PRESSURE with vector operations
   ============================================ */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int mode, int iterations) {
    volatile int result = 0;
    int i;
    
    /* Scalar variables */
    int a0 = mode, a1 = mode + 1, a2 = mode + 2, a3 = mode + 3;
    int a4 = mode + 4, a5 = mode + 5, a6 = mode + 6, a7 = mode + 7;
    int a8 = mode + 8, a9 = mode + 9, a10 = mode + 10, a11 = mode + 11;
    int a12 = mode + 12, a13 = mode + 13, a14 = mode + 14, a15 = mode + 15;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {mode, mode + 1, mode + 2, mode + 3};
    v4si v1 = {mode + 4, mode + 5, mode + 6, mode + 7};
    v4si v2 = {mode + 8, mode + 9, mode + 10, mode + 11};
    v4si v3 = {mode + 12, mode + 13, mode + 14, mode + 15};
#endif
    
    /* Large switch statement with 30+ cases */
    for (i = 0; i < iterations; i++) {
        switch (i % 35) {
            case 0:
                a0 = a1 + a2;
                a3 = a4 * a5;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                break;
            case 1:
                a1 = a2 - a3;
                a4 = a5 / (a6 + 1);
#ifdef __GNUC__
                v1 = v2 - v3;
#endif
                break;
            case 2:
                a2 = a3 ^ a4;
                a5 = a6 | a7;
#ifdef __GNUC__
                v2 = v0 ^ v1;
#endif
                break;
            case 3:
                a3 = a4 & a5;
                a6 = a7 << 2;
#ifdef __GNUC__
                v3 = v1 & v2;
#endif
                break;
            case 4:
                a4 = a5 >> 1;
                a7 = a8 + a9;
#ifdef __GNUC__
                v0 = v1 >> 1;
#endif
                break;
            case 5:
                a5 = a6 * a7;
                a8 = a9 - a10;
#ifdef __GNUC__
                v1 = v2 * v3;
#endif
                break;
            case 6:
                a6 = a7 / 2;
                a9 = a10 ^ a11;
#ifdef __GNUC__
                v2 = v3 / 2;
#endif
                break;
            case 7:
                a7 = a8 | a9;
                a10 = a11 & a12;
#ifdef __GNUC__
                v3 = v0 | v1;
#endif
                break;
            case 8:
                a8 = a9 << 3;
                a11 = a12 + a13;
#ifdef __GNUC__
                v0 = v1 << 2;
#endif
                break;
            case 9:
                a9 = a10 >> 2;
                a12 = a13 * a14;
#ifdef __GNUC__
                v1 = v2 >> 1;
#endif
                break;
            case 10:
                a10 = a11 - a12;
                a13 = a14 / 3;
#ifdef __GNUC__
                v2 = v3 - v0;
#endif
                break;
            case 11:
                a11 = a12 ^ a13;
                a14 = a15 | a0;
#ifdef __GNUC__
                v3 = v0 ^ v1;
#endif
                break;
            case 12:
                a12 = a13 & a14;
                a15 = a0 << 1;
#ifdef __GNUC__
                v0 = v1 & v2;
#endif
                break;
            case 13:
                a0 = a1 * a2 + a3;
                a4 = a5 - a6 * a7;
                break;
            case 14:
                a1 = a2 / (a3 + 1) + a4;
                a5 = a6 | a7 & a8;
                break;
            case 15:
                a2 = (a3 ^ a4) | a5;
                a6 = a7 << (a8 % 4);
                break;
            case 16:
                a3 = a4 & (a5 | a6);
                a7 = a8 >> (a9 % 4);
                break;
            case 17:
                a4 = a5 + a6 * a7;
                a8 = a9 - a10 / 2;
                break;
            case 18:
                a5 = a6 ^ (a7 & a8);
                a9 = a10 | a11;
                break;
            case 19:
                a6 = a7 << 1;
                a10 = a11 >> 2;
                break;
            case 20:
                a7 = a8 + a9 + a10;
                a11 = a12 * a13;
                break;
            case 21:
                a8 = a9 - a10 * a11;
                a12 = a13 / (a14 + 1);
                break;
            case 22:
                a9 = a10 ^ a11 ^ a12;
                a13 = a14 | a15;
                break;
            case 23:
                a10 = a11 & a12 & a13;
                a14 = a15 << 3;
                break;
            case 24:
                a11 = a12 >> 1;
                a15 = a0 + a1;
                break;
            case 25:
                a12 = a13 * a14 * a15;
                a0 = a1 - a2;
                break;
            case 26:
                a13 = a14 / 2;
                a1 = a2 ^ a3;
                break;
            case 27:
                a14 = a15 | a0 | a1;
                a2 = a3 & a4;
                break;
            case 28:
                a15 = a0 << 2;
                a3 = a4 + a5;
                break;
            case 29:
                a0 = a1 >> 1;
                a4 = a5 * a6;
                break;
            case 30:
                a1 = a2 - a3 - a4;
                a5 = a6 / 3;
                break;
            case 31:
                a2 = a3 ^ a4 ^ a5;
                a6 = a7 | a8;
                break;
            case 32:
                a3 = a4 & a5 & a6;
                a7 = a8 << 1;
                break;
            case 33:
                a4 = a5 >> 2;
                a8 = a9 + a10;
                break;
            case 34:
                a5 = a6 * a7 + a8;
                a9 = a10 - a11 * a12;
                break;
        }
        
        /* Force liveness of all variables */
        asm volatile("" : : "g"(a0), "g"(a1), "g"(a2), "g"(a3),
                           "g"(a4), "g"(a5), "g"(a6), "g"(a7),
                           "g"(a8), "g"(a9), "g"(a10), "g"(a11),
                           "g"(a12), "g"(a13), "g"(a14), "g"(a15));
#ifdef __GNUC__
        asm volatile("" : : "g"(v0), "g"(v1), "g"(v2), "g"(v3));
#endif
        
        /* Complex loop control */
        if (i % 7 == 0) {
            continue;
        } else if (i % 13 == 0) {
            i += 2;
            continue;
        } else if (i % 17 == 0) {
            break;
        }
        
        result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
    }
    
    return result;
}

/* ============================================
   PATTERN D: ARTIFICIAL CONFLICT with register variables
   ============================================ */
#ifdef __GNUC__
/* Dummy helper functions */
NOINLINE int helper1(int x) { return x * 2; }
NOINLINE int helper2(int x) { return x + 1; }
NOINLINE int helper3(int x) { return x ^ 0x55; }
NOINLINE int helper4(int x) { return x | 0xAA; }
NOINLINE int helper5(int x) { return x & 0xFF; }
#endif

NOINLINE USED
int pattern_d_artificial_conflict(int value) {
    volatile int result = 0;
    int i;
    
#ifdef __GNUC__
    /* Explicit register variables creating conflicts */
    register int r0 asm ("r10") = value;
    register int r1 asm ("r11") = value + 1;
    register int r2 asm ("r12") = value + 2;
    register int r3 asm ("r13") = value + 3;
    /* Note: r14 and r15 might be reserved on some architectures */
    register int r4 asm ("r8") = value + 4;
    register int r5 asm ("r9") = value + 5;
#else
    int r0 = value, r1 = value + 1, r2 = value + 2;
    int r3 = value + 3, r4 = value + 4, r5 = value + 5;
#endif
    
    /* Additional local variables */
    int v6 = value + 6, v7 = value + 7, v8 = value + 8, v9 = value + 9;
    int v10 = value + 10, v11 = value + 11, v12 = value + 12, v13 = value + 13;
    int v14 = value + 14, v15 = value + 15;
    
    /* Complex control flow with calls to noinline functions */
    for (i = 0; i < 100; i++) {
        switch (i % 20) {
            case 0:
                r0 = helper1(r1);
                r2 = helper2(r3);
                break;
            case 1:
                r1 = helper3(r2);
                r3 = helper4(r0);
                break;
            case 2:
                r2 = helper5(r1);
                r0 = helper1(r3);
                break;
            case 3:
                r3 = helper2(r0);
                r1 = helper3(r2);
                break;
            case 4:
                r4 = helper4(r1);
                r5 = helper5(r0);
                break;
            case 5:
                v6 = helper1(r2);
                v7 = helper2(r3);
                break;
            case 6:
                v8 = helper3(r4);
                v9 = helper4(r5);
                break;
            case 7:
                v10 = helper5(v6);
                v11 = helper1(v7);
                break;
            case 8:
                v12 = helper2(v8);
                v13 = helper3(v9);
                break;
            case 9:
                v14 = helper4(v10);
                v15 = helper5(v11);
                break;
            case 10:
                r0 = v12 + v13;
                r1 = v14 - v15;
                break;
            case 11:
                r2 = v6 * v7;
                r3 = v8 / (v9 + 1);
                break;
            case 12:
                r4 = v10 ^ v11;
                r5 = v12 | v13;
                break;
            case 13:
                v6 = r0 & r1;
                v7 = r2 << 1;
                break;
            case 14:
                v8 = r3 >> 2;
                v9 = r4 + r5;
                break;
            case 15:
                v10 = r0 - r1;
                v11 = r2 * r3;
                break;
            case 16:
                v12 = r4 ^ r5;
                v13 = r0 | r1;
                break;
            case 17:
                v14 = r2 & r3;
                v15 = r4 << 3;
                break;
            case 18:
                r0 = v6 >> 1;
                r1 = v7 + v8;
                break;
            case 19:
                r2 = v9 - v10;
                r3 = v11 * v12;
                break;
        }
        
        /* Force liveness */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3),
                           "g"(r4), "g"(r5), "g"(v6), "g"(v7),
                           "g"(v8), "g"(v9), "g"(v10), "g"(v11),
                           "g"(v12), "g"(v13), "g"(v14), "g"(v15));
        
        /* Artificial loop exit conditions */
        if (i == 50) {
            result += r0 + r1 + r2 + r3 + r4 + r5;
            continue;
        }
        
        result += v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    }
    
    return result;
}

/* ============================================
   MAIN FUNCTION with profile feedback
   ============================================ */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    int i, iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF coverage test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        total_result += pattern_a_entry_exit(i % 50 + 10, i);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 10 == 0) {
            jump_counter = 0;
            total_result += pattern_b_new_exit_entry(0, 3);
        }
        
        /* Pattern C - Mixed pressure */
        total_result += pattern_c_mixed_pressure(i % 10, i % 20 + 5);
        
        /* Pattern D - Artificial conflict */
        total_result += pattern_d_artificial_conflict(i);
        
        /* Prevent optimization */
        asm volatile("" : "+g"(total_result));
    }
    
    if (verbose) {
        printf("Total result: %d\n", total_result);
    }
    
    return total_result != 0 ? 0 : 1;
}
