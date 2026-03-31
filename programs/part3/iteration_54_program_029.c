/* test_mcf.c - Comprehensive test to trigger MCF special node printing */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define VECTOR_TYPE __attribute__((vector_size(16)))
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define VECTOR_TYPE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex irreducible CFG to force ENTRY_BLOCK and EXIT_BLOCK special nodes */
    volatile int result = seed;
    int i, j, k;
    
    /* Create irreducible region with goto */
    if (iterations > 0) {
        goto middle_of_function;
    }
    
    /* Multiple basic blocks with high register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Deeply nested if-else chain */
    for (i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            if (i % 3 == 0) {
                if (i % 5 == 0) {
                    r0 = r1 + r2;
                    r3 = r4 - r5;
                    goto middle_of_function;
                } else {
                    r6 = r7 * r8;
                    r9 = r10 / (r11 ? r11 : 1);
                }
            } else {
                for (j = 0; j < 5; j++) {
                    r12 += r13;
                    r14 -= r15;
                    if (j == 3) break;
                }
            }
        } else {
            switch (i % 7) {
                case 0: r0++; break;
                case 1: r1--; break;
                case 2: r2 *= 2; break;
                case 3: r3 /= 2; break;
                case 4: r4 = r5 ^ r6; break;
                case 5: r7 = r8 | r9; break;
                case 6: r10 = r11 & r12; break;
            }
        }
        
    middle_of_function:
        /* Irreducible region entry point */
        k = i * 2;
        if (k > 100) {
            r0 = r1 + r2 + r3 + r4 + r5;
            goto loop_end;
        } else {
            r6 = r7 + r8 + r9 + r10 + r11;
        }
        
        /* More arithmetic to increase register pressure */
        r12 = r13 * r14;
        r15 = r0 ^ r1 ^ r2 ^ r3;
        
    loop_end:
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + 
                  r10 + r11 + r12 + r13 + r14 + r15;
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
NOINLINE
int pattern_b_new_exit_entry(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Many scalar variables to pressure registers */
    int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    int a5 = seed+5, a6 = seed+6, a7 = seed+7, a8 = seed+8, a9 = seed+9;
    int b0 = seed+10, b1 = seed+11, b2 = seed+12, b3 = seed+13, b4 = seed+14;
    int b5 = seed+15, b6 = seed+16, b7 = seed+17, b8 = seed+18, b9 = seed+19;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal execution path */
        for (i = 0; i < iterations; i++) {
            /* Complex arithmetic using all variables */
            a0 = a1 + a2;
            a3 = a4 - a5;
            a6 = a7 * a8;
            a9 = b0 / (b1 ? b1 : 1);
            
            b2 = b3 ^ b4;
            b5 = b6 | b7;
            b8 = b9 & a0;
            
            /* Conditional longjmp creates exceptional edge */
            if (i == iterations / 2) {
                longjmp(jump_buffer, 1);
            }
            
            /* More arithmetic to keep variables live */
            a1 = a2 + a3 + a4;
            a5 = a6 - a7 - a8;
            b1 = b2 * b3 * b4;
            b6 = b7 / (b8 ? b8 : 2);
            
            result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
                     b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9;
                     
            FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3); FORCE_USE(a4);
            FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7); FORCE_USE(a8); FORCE_USE(a9);
            FORCE_USE(b0); FORCE_USE(b1); FORCE_USE(b2); FORCE_USE(b3); FORCE_USE(b4);
            FORCE_USE(b5); FORCE_USE(b6); FORCE_USE(b7); FORCE_USE(b8); FORCE_USE(b9);
        }
    } else {
        /* longjmp target - different execution path */
        result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
                b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9;
    }
    
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
NOINLINE
int pattern_c_mixed_pressure(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Vector type for SIMD pressure */
    typedef int v4si VECTOR_TYPE;
    
    /* Many scalar and vector variables */
    int s0 = seed, s1 = seed+1, s2 = seed+2, s3 = seed+3, s4 = seed+4;
    int s5 = seed+5, s6 = seed+6, s7 = seed+7, s8 = seed+8, s9 = seed+9;
    v4si v0 = {seed, seed+1, seed+2, seed+3};
    v4si v1 = {seed+4, seed+5, seed+6, seed+7};
    v4si v2 = {seed+8, seed+9, seed+10, seed+11};
    v4si v3 = {seed+12, seed+13, seed+14, seed+15};
    
    /* Large switch statement with 30+ cases */
    for (i = 0; i < iterations; i++) {
        switch (i % 35) {
            case 0: s0 = s1 + s2; v0 = v1 + v2; break;
            case 1: s1 = s2 - s3; v1 = v2 - v3; break;
            case 2: s2 = s3 * s4; v2 = v3 * v0; break;
            case 3: s3 = s4 / (s5 ? s5 : 1); v3 = v0 / (v1 ? v1 : (v4si){1,1,1,1}); break;
            case 4: s4 = s5 ^ s6; v0 = v1 ^ v2; break;
            case 5: s5 = s6 | s7; v1 = v2 | v3; break;
            case 6: s6 = s7 & s8; v2 = v3 & v0; break;
            case 7: s7 = s8 << 1; v3 = v0 << 1; break;
            case 8: s8 = s9 >> 2; v0 = v1 >> 2; break;
            case 9: s9 = s0 + s1; v1 = v2 + v3; break;
            case 10: s0 = s1 - s2; v2 = v3 - v0; break;
            case 11: s1 = s2 * s3; v3 = v0 * v1; break;
            case 12: s2 = s3 / (s4 ? s4 : 1); v0 = v1 / (v2 ? v2 : (v4si){1,1,1,1}); break;
            case 13: s3 = s4 ^ s5; v1 = v2 ^ v3; break;
            case 14: s4 = s5 | s6; v2 = v3 | v0; break;
            case 15: s5 = s6 & s7; v3 = v0 & v1; break;
            case 16: s6 = s7 << 2; v0 = v1 << 2; break;
            case 17: s7 = s8 >> 1; v1 = v2 >> 1; break;
            case 18: s8 = s9 + s0; v2 = v3 + v0; break;
            case 19: s9 = s0 - s1; v3 = v0 - v1; break;
            case 20: s0 = s1 * s2; v0 = v1 * v2; break;
            case 21: s1 = s2 / (s3 ? s3 : 1); v1 = v2 / (v3 ? v3 : (v4si){1,1,1,1}); break;
            case 22: s2 = s3 ^ s4; v2 = v3 ^ v0; break;
            case 23: s3 = s4 | s5; v3 = v0 | v1; break;
            case 24: s4 = s5 & s6; v0 = v1 & v2; break;
            case 25: s5 = s6 << 3; v1 = v2 << 3; break;
            case 26: s6 = s7 >> 3; v2 = v3 >> 3; break;
            case 27: s7 = s8 + s9; v3 = v0 + v1; break;
            case 28: s8 = s9 - s0; v0 = v1 - v2; break;
            case 29: s9 = s0 * s1; v1 = v2 * v3; break;
            case 30: s0 = s1 / (s2 ? s2 : 1); v2 = v3 / (v0 ? v0 : (v4si){1,1,1,1}); break;
            case 31: s1 = s2 ^ s3; v3 = v0 ^ v1; break;
            case 32: s2 = s3 | s4; v0 = v1 | v2; break;
            case 33: s3 = s4 & s5; v1 = v2 & v3; break;
            case 34: s4 = s5 << 4; v2 = v3 << 4; break;
        }
        
        /* Use continue to create complex control flow */
        if (i % 10 == 0) continue;
        
        /* More arithmetic mixing scalars and vectors */
        int* v0p = (int*)&v0;
        int* v1p = (int*)&v1;
        s5 = v0p[0] + v0p[1] + v0p[2] + v0p[3];
        s6 = v1p[0] + v1p[1] + v1p[2] + v1p[3];
        
        result += s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9;
        
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3); FORCE_USE(s4);
        FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7); FORCE_USE(s8); FORCE_USE(s9);
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
    }
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE void dummy1(register int x asm ("r10"), register int y asm ("r11")) {
    FORCE_USE(x); FORCE_USE(y);
}

NOINLINE void dummy2(register int a asm ("r10"), register int b asm ("r12")) {
    FORCE_USE(a); FORCE_USE(b);
}

NOINLINE void dummy3(register int p asm ("r11"), register int q asm ("r12")) {
    FORCE_USE(p); FORCE_USE(q);
}

NOINLINE
int pattern_d_artificial_conflict(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Explicit register variables with conflicts */
    register int x asm ("r10") = seed;
    register int y asm ("r11") = seed + 1;
    register int z asm ("r12") = seed + 2;
    
    /* Many other local variables */
    int a0 = seed+3, a1 = seed+4, a2 = seed+5, a3 = seed+6, a4 = seed+7;
    int a5 = seed+8, a6 = seed+9, a7 = seed+10, a8 = seed+11, a9 = seed+12;
    
    for (i = 0; i < iterations; i++) {
        /* Call dummy functions to force register conflicts */
        if (i % 3 == 0) {
            dummy1(x, y);
            x = a0 + a1;
            y = a2 - a3;
        } else if (i % 3 == 1) {
            dummy2(x, z);
            x = a4 * a5;
            z = a6 / (a7 ? a7 : 1);
        } else {
            dummy3(y, z);
            y = a8 ^ a9;
            z = a0 | a1;
        }
        
        /* Complex arithmetic using all variables */
        a0 = a1 + a2 + x;
        a3 = a4 - a5 - y;
        a6 = a7 * a8 * z;
        a9 = a0 / (a1 ? a1 : 1);
        
        /* Rotate values */
        int temp = a0;
        a0 = a1; a1 = a2; a2 = a3; a3 = a4; a4 = a5;
        a5 = a6; a6 = a7; a7 = a8; a8 = a9; a9 = temp;
        
        result += x + y + z + a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
        
        FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3); FORCE_USE(a4);
        FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7); FORCE_USE(a8); FORCE_USE(a9);
    }
    
    return result;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char** argv) {
    volatile int total = 0;
    int i, iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Check CPU features to engage target-specific register allocation */
    int has_avx2 = 0;
#ifdef __GNUC__
    has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", has_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different seeds */
    for (i = 0; i < 10; i++) {
        if (verbose && i % 10 == 0) {
            printf("Iteration batch %d\n", i);
        }
        
        total += pattern_a_entry_exit(iterations / 10, i * 100);
        total += pattern_b_new_exit_entry(iterations / 10, i * 200 + 1);
        total += pattern_c_mixed_pressure(iterations / 10, i * 300 + 2);
        total += pattern_d_artificial_conflict(iterations / 10, i * 400 + 3);
        
        /* Mix in some conditional calls based on CPU features */
        if (has_avx2) {
            total += pattern_c_mixed_pressure(iterations / 20, i * 500 + 4);
        }
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    /* Prevent optimization of total */
    FORCE_USE(total);
    
    return total != 0 ? 0 : 1;
}
