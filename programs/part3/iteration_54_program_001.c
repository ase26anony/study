/* test_mcf.c - Comprehensive test for GCC MCF pass special node coverage */
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

#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

static int verbose = 0;

/* Pattern A: Deep nesting with irreducible regions for ENTRY/EXIT blocks */
HOT NOINLINE
int pattern_a_irreducible(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    
    /* Create complex entry block with many variables */
    int a0 = seed * 1, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    int a12 = seed * 13, a13 = seed * 14, a14 = seed * 15, a15 = seed * 16;
    
    /* Label to create irreducible region */
    irreducible_start:
    
    for (i = 0; i < iterations; i++) {
        /* Deep if-else chain creating many basic blocks */
        if (i % 3 == 0) {
            a0 = a1 + a2;
            a1 = a3 * a4;
            a2 = a5 - a6;
            if (i % 7 == 0) {
                a3 = a7 ^ a8;
                a4 = a9 | a10;
                goto irreducible_mid;  /* Create irreducible region */
            } else {
                a5 = a11 & a12;
                a6 = a13 << 2;
            }
        } else if (i % 3 == 1) {
            a7 = a14 >> 1;
            a8 = a15 + i;
            for (j = 0; j < 5; j++) {
                a9 = a0 * j;
                a10 = a1 + j;
                if (j % 2 == 0) {
                    a11 = a2 - j;
                    continue;  /* Complex loop control */
                }
                a12 = a3 ^ j;
            }
        } else {
            a13 = a4 | i;
            a14 = a5 & i;
            irreducible_mid:
            a15 = a6 << (i & 3);
            if (i % 11 == 0) {
                goto irreducible_start;  /* Back edge creating irreducible region */
            }
        }
        
        /* More arithmetic to increase register pressure */
        a0 = a0 + a1 - a2 + a3 - a4 + a5 - a6;
        a7 = a7 * a8 / (a9 + 1);
        a10 = a10 ^ a11 ^ a12 ^ a13;
        a14 = (a14 << 2) | (a15 >> 2);
        a15 = a0 * a7 + a14;
        
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
        FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
        FORCE_USE(a8); FORCE_USE(a9); FORCE_USE(a10); FORCE_USE(a11);
        FORCE_USE(a12); FORCE_USE(a13); FORCE_USE(a14); FORCE_USE(a15);
    }
    
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + 
             a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
    return result & 0xFFFF;
}

/* Pattern B: setjmp/longjmp with many variables for NEW_EXIT/NEW_ENTRY */
NOINLINE
int pattern_b_setjmp(int iterations, int seed) {
    jmp_buf env;
    int result = seed;
    volatile int should_jump = 0;
    
    /* Many scalar variables to pressure registers */
    int b0 = seed, b1 = seed*2, b2 = seed*3, b3 = seed*4;
    int b4 = seed*5, b5 = seed*6, b6 = seed*7, b7 = seed*8;
    int b8 = seed*9, b9 = seed*10, b10 = seed*11, b11 = seed*12;
    int b12 = seed*13, b13 = seed*14, b14 = seed*15, b15 = seed*16;
    int b16 = seed*17, b17 = seed*18, b18 = seed*19, b19 = seed*20;
    int b20 = seed*21, b21 = seed*22, b22 = seed*23, b23 = seed*24;
    
    if (setjmp(env) == 0) {
        /* Normal execution path */
        for (int i = 0; i < iterations; i++) {
            /* Complex arithmetic on all variables */
            b0 = b1 + b2; b1 = b3 * b4; b2 = b5 - b6;
            b3 = b7 ^ b8; b4 = b9 | b10; b5 = b11 & b12;
            b6 = b13 << 1; b7 = b14 >> 2; b8 = b15 + b16;
            b9 = b17 * b18; b10 = b19 - b20; b11 = b21 ^ b22;
            b12 = b23 | b0; b13 = b1 & b2; b14 = b3 << 3;
            b15 = b4 >> 1; b16 = b5 + b6; b17 = b7 * b8;
            b18 = b9 - b10; b19 = b11 ^ b12; b20 = b13 | b14;
            b21 = b15 & b16; b22 = b17 << 2; b23 = b18 >> 2;
            
            /* Force all variables live */
            FORCE_USE(b0); FORCE_USE(b1); FORCE_USE(b2); FORCE_USE(b3);
            FORCE_USE(b4); FORCE_USE(b5); FORCE_USE(b6); FORCE_USE(b7);
            FORCE_USE(b8); FORCE_USE(b9); FORCE_USE(b10); FORCE_USE(b11);
            FORCE_USE(b12); FORCE_USE(b13); FORCE_USE(b14); FORCE_USE(b15);
            FORCE_USE(b16); FORCE_USE(b17); FORCE_USE(b18); FORCE_USE(b19);
            FORCE_USE(b20); FORCE_USE(b21); FORCE_USE(b22); FORCE_USE(b23);
            
            /* Occasionally trigger longjmp */
            if (i > iterations/2 && (i % 13 == 0)) {
                should_jump = 1;
                longjmp(env, 1);
            }
        }
    } else {
        /* longjmp target - exceptional edge */
        b0 = b23; b1 = b22; b2 = b21; b3 = b20;
        b4 = b19; b5 = b18; b6 = b17; b7 = b16;
        b8 = b15; b9 = b14; b10 = b13; b11 = b12;
    }
    
    result = b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 + b11 +
             b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 + b21 + b22 + b23;
    return result & 0xFFFFFF;
}

/* Pattern C: Vector operations with large switch */
NOINLINE
int pattern_c_vector_switch(int iterations, int seed) {
    int result = seed;
    
#ifdef __GNUC__
    v4si v0 = {seed, seed+1, seed+2, seed+3};
    v4si v1 = {seed+4, seed+5, seed+6, seed+7};
    v4si v2 = {seed+8, seed+9, seed+10, seed+11};
    v4si v3 = {seed+12, seed+13, seed+14, seed+15};
#endif
    
    /* Many scalar variables for register pressure */
    int c0 = seed, c1 = seed*2, c2 = seed*3, c3 = seed*4;
    int c4 = seed*5, c5 = seed*6, c6 = seed*7, c7 = seed*8;
    int c8 = seed*9, c9 = seed*10, c10 = seed*11, c11 = seed*12;
    int c12 = seed*13, c13 = seed*14, c14 = seed*15, c15 = seed*16;
    
    for (int i = 0; i < iterations; i++) {
        int switch_val = (i * seed) % 35;  /* 35 cases to exceed 30 */
        
        switch (switch_val) {
            case 0:
                c0 = c1 + c2; c1 = c3 * c4;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                if (i % 3 == 0) continue;
                break;
            case 1:
                c2 = c5 - c6; c3 = c7 ^ c8;
#ifdef __GNUC__
                v1 = v2 * v3;
#endif
                if (i % 5 == 0) break;
                /* fall through */
            case 2:
                c4 = c9 | c10; c5 = c11 & c12;
                c6 = c13 << 1; c7 = c14 >> 2;
                break;
            case 3:
                c8 = c15 + c0; c9 = c1 * c2;
#ifdef __GNUC__
                v2 = v3 - v0;
#endif
                if (i % 7 == 0) continue;
                break;
            case 4:
                c10 = c3 ^ c4; c11 = c5 | c6;
                c12 = c7 & c8; c13 = c9 << 3;
                break;
            case 5:
                c14 = c10 >> 1; c15 = c11 + c12;
#ifdef __GNUC__
                v3 = v0 ^ v1;
#endif
                break;
            case 6:
                c0 = c13 * c14; c1 = c15 - c0;
                c2 = c1 ^ c2; c3 = c3 | c4;
                break;
            case 7:
                c4 = c5 & c6; c5 = c7 << 2;
#ifdef __GNUC__
                v0 = v1 + v2 + v3;
#endif
                if (i % 11 == 0) break;
                /* fall through */
            case 8:
                c6 = c8 >> 2; c7 = c9 + c10;
                c8 = c11 * c12; c9 = c13 - c14;
                break;
            case 9:
                c10 = c15 ^ c0; c11 = c1 | c2;
#ifdef __GNUC__
                v1 = v2 * v0;
#endif
                break;
            case 10:
                c12 = c3 & c4; c13 = c5 << 1;
                c14 = c6 >> 3; c15 = c7 + c8;
                break;
            case 11:
                c0 = c9 * c10; c1 = c11 - c12;
#ifdef __GNUC__
                v2 = v3 - v1;
#endif
                if (i % 13 == 0) continue;
                break;
            case 12:
                c2 = c13 ^ c14; c3 = c15 | c0;
                c4 = c1 & c2; c5 = c3 << 2;
                break;
            case 13:
                c6 = c4 >> 1; c7 = c5 + c6;
#ifdef __GNUC__
                v3 = v0 ^ v2;
#endif
                break;
            case 14:
                c8 = c7 * c8; c9 = c9 - c10;
                c10 = c11 ^ c12; c11 = c13 | c14;
                break;
            case 15:
                c12 = c15 & c0; c13 = c1 << 3;
#ifdef __GNUC__
                v0 = v1 + v3;
#endif
                break;
            case 16:
                c14 = c2 >> 2; c15 = c3 + c4;
                c0 = c5 * c6; c1 = c7 - c8;
                break;
            case 17:
                c2 = c9 ^ c10; c3 = c11 | c12;
#ifdef __GNUC__
                v1 = v2 * v3;
#endif
                if (i % 17 == 0) break;
                /* fall through */
            case 18:
                c4 = c13 & c14; c5 = c15 << 1;
                c6 = c0 >> 2; c7 = c1 + c2;
                break;
            case 19:
                c8 = c3 * c4; c9 = c5 - c6;
#ifdef __GNUC__
                v2 = v0 - v1;
#endif
                break;
            case 20:
                c10 = c7 ^ c8; c11 = c9 | c0;
                c12 = c1 & c2; c13 = c3 << 2;
                break;
            case 21:
                c14 = c4 >> 1; c15 = c5 + c6;
#ifdef __GNUC__
                v3 = v1 ^ v2;
#endif
                break;
            case 22:
                c0 = c7 * c8; c1 = c9 - c10;
                c2 = c11 ^ c12; c3 = c13 | c14;
                break;
            case 23:
                c4 = c15 & c0; c5 = c1 << 3;
#ifdef __GNUC__
                v0 = v2 + v3;
#endif
                break;
            case 24:
                c6 = c2 >> 2; c7 = c3 + c4;
                c8 = c5 * c6; c9 = c7 - c8;
                break;
            case 25:
                c10 = c9 ^ c0; c11 = c1 | c2;
#ifdef __GNUC__
                v1 = v3 * v0;
#endif
                if (i % 19 == 0) continue;
                break;
            case 26:
                c12 = c3 & c4; c13 = c5 << 1;
                c14 = c6 >> 3; c15 = c7 + c8;
                break;
            case 27:
                c0 = c9 * c10; c1 = c11 - c12;
#ifdef __GNUC__
                v2 = v0 - v3;
#endif
                break;
            case 28:
                c2 = c13 ^ c14; c3 = c15 | c0;
                c4 = c1 & c2; c5 = c3 << 2;
                break;
            case 29:
                c6 = c4 >> 1; c7 = c5 + c6;
#ifdef __GNUC__
                v3 = v1 ^ v0;
#endif
                break;
            case 30:
                c8 = c7 * c8; c9 = c9 - c10;
                c10 = c11 ^ c12; c11 = c13 | c14;
                break;
            case 31:
                c12 = c15 & c0; c13 = c1 << 3;
#ifdef __GNUC__
                v0 = v1 + v2 + v3;
#endif
                break;
            case 32:
                c14 = c2 >> 2; c15 = c3 + c4;
                c0 = c5 * c6; c1 = c7 - c8;
                break;
            case 33:
                c2 = c9 ^ c10; c3 = c11 | c12;
#ifdef __GNUC__
                v1 = v2 * v0;
#endif
                break;
            case 34:
                c4 = c13 & c14; c5 = c15 << 1;
                c6 = c0 >> 2; c7 = c1 + c2;
                break;
            default:
                c8 = c3 * c4; c9 = c5 - c6;
                break;
        }
        
        /* Force all scalars live */
        FORCE_USE(c0); FORCE_USE(c1); FORCE_USE(c2); FORCE_USE(c3);
        FORCE_USE(c4); FORCE_USE(c5); FORCE_USE(c6); FORCE_USE(c7);
        FORCE_USE(c8); FORCE_USE(c9); FORCE_USE(c10); FORCE_USE(c11);
        FORCE_USE(c12); FORCE_USE(c13); FORCE_USE(c14); FORCE_USE(c15);
        
#ifdef __GNUC__
        /* Force all vectors live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    }
    
    result = c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + 
             c8 + c9 + c10 + c11 + c12 + c13 + c14 + c15;
#ifdef __GNUC__
    result += v0[0] + v0[1] + v0[2] + v0[3];
#endif
    return result & 0xFFFFFF;
}

/* Pattern D: Explicit register variables with artificial conflicts */
NOINLINE
int helper_dummy1(int x, int y) { return x ^ y; }
NOINLINE
int helper_dummy2(int x, int y) { return x | y; }
NOINLINE
int helper_dummy3(int x, int y) { return x & y; }
NOINLINE
int helper_dummy4(int x, int y) { return x + y; }

NOINLINE
int pattern_d_register_conflict(int iterations, int seed) {
    /* Explicit register variables for artificial conflicts */
#ifdef __GNUC__
    register int r0 asm ("r10") = seed;
    register int r1 asm ("r11") = seed * 2;
    register int r2 asm ("r12") = seed * 3;
    register int r3 asm ("r13") = seed * 4;
#else
    int r0 = seed, r1 = seed * 2, r2 = seed * 3, r3 = seed * 4;
#endif
    
    int d0 = seed, d1 = seed*5, d2 = seed*6, d3 = seed*7;
    int d4 = seed*8, d5 = seed*9, d6 = seed*10, d7 = seed*11;
    int d8 = seed*12, d9 = seed*13, d10 = seed*14, d11 = seed*15;
    int d12 = seed*16, d13 = seed*17, d14 = seed*18, d15 = seed*19;
    
    for (int i = 0; i < iterations; i++) {
        /* Force register variable usage with calls that clobber registers */
        r0 = helper_dummy1(r0, d0);
        r1 = helper_dummy2(r1, d1);
        r2 = helper_dummy3(r2, d2);
        r3 = helper_dummy4(r3, d3);
        
        /* Complex arithmetic creating many temporary values */
        d0 = d1 + d2; d1 = d3 * d4; d2 = d5 - d6;
        d3 = d7 ^ d8; d4 = d9 | d10; d5 = d11 & d12;
        d6 = d13 << 1; d7 = d14 >> 2; d8 = d15 + r0;
        d9 = r1 * r2; d10 = r3 - d0; d11 = d1 ^ d2;
        d12 = d3 | d4; d13 = d5 & d6; d14 = d7 << 3;
        d15 = d8 >> 1; 
        
        /* More calls to split live ranges */
        r0 = helper_dummy1(d9, d10);
        r1 = helper_dummy2(d11, d12);
        r2 = helper_dummy3(d13, d14);
        r3 = helper_dummy4(d15, r0);
        
        /* Force all variables live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(d0); FORCE_USE(d1); FORCE_USE(d2); FORCE_USE(d3);
        FORCE_USE(d4); FORCE_USE(d5); FORCE_USE(d6); FORCE_USE(d7);
        FORCE_USE(d8); FORCE_USE(d9); FORCE_USE(d10); FORCE_USE(d11);
        FORCE_USE(d12); FORCE_USE(d13); FORCE_USE(d14); FORCE_USE(d15);
        
        /* Switch-like dispatch with continue/break to different cases */
        if (i % 4 == 0) {
            d0 = r1 + r2;
            if (i % 8 == 0) continue;
        } else if (i % 4 == 1) {
            d1 = r3 * r0;
            if (i % 9 == 0) break;
        } else if (i % 4 == 2) {
            d2 = r1 ^ r2;
            if (i % 7 == 0) continue;
        } else {
            d3 = r3 | r0;
        }
    }
    
    int result = r0 + r1 + r2 + r3 + d0 + d1 + d2 + d3 + d4 + d5 + 
                 d6 + d7 + d8 + d9 + d10 + d11 + d12 + d13 + d14 + d15;
    return result & 0xFFFFFF;
}

/* Main function with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total = 0;  /* volatile to prevent optimization */
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Check CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        use_avx2 = 1;
        if (verbose) printf("AVX2 supported, using vector patterns\n");
    }
#endif
    
    /* Run all patterns multiple times to generate profile data */
    for (int run = 0; run < 10; run++) {
        int seed = run * 12345 + 6789;
        
        /* Pattern A: Irreducible regions */
        total += pattern_a_irreducible(iterations / 10, seed);
        
        /* Pattern B: setjmp/longjmp */
        total += pattern_b_setjmp(iterations / 20, seed + 1);
        
        /* Pattern C: Vector + switch */
        total += pattern_c_vector_switch(iterations / 5, seed + 2);
        
        /* Pattern D: Register conflicts */
        total += pattern_d_register_conflict(iterations / 8, seed + 3);
        
        if (verbose && run % 3 == 0) {
            printf("Run %d complete, total so far: %d\n", run, total);
        }
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    /* Return non-zero to ensure execution */
    return (total != 0) ? 0 : 1;
}
