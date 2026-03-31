/* test_mcf.c - Program to stress GCC's Min-Cost Flow solver */
/* Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -fdump-rtl-bbro-slim test_mcf.c -o test_mcf */

#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define REG_VAR(reg) register
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define REG_VAR(reg)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

static int verbose = 0;

/* Pattern A: Function targeting ENTRY/EXIT blocks with irreducible region */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    /* Label the first block to encourage ENTRY_BLOCK identification */
    start_label:
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (i % 2 == 0) {
            if (i % 3 == 0) {
                if (i % 5 == 0) {
                    a0 = i * 3;
                    a1 = i * 5;
                    a2 = i * 7;
                    a3 = i * 11;
                    a4 = i * 13;
                    FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3); FORCE_USE(a4);
                } else {
                    b0 = i * 2;
                    b1 = i * 4;
                    b2 = i * 6;
                    b3 = i * 8;
                    b4 = i * 10;
                    FORCE_USE(b0); FORCE_USE(b1); FORCE_USE(b2); FORCE_USE(b3); FORCE_USE(b4);
                }
            } else {
                /* Create irreducible region with goto */
                if (i % 7 == 0) {
                    goto middle_label;
                }
                a5 = i * 17;
                a6 = i * 19;
                a7 = i * 23;
                FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
            }
        } else {
            middle_label:
            for (j = 0; j < 5; j++) {
                for (k = 0; k < 3; k++) {
                    /* Complex arithmetic to prevent optimization */
                    a8 = (i * j * k) ^ 0x12345678;
                    a9 = (i + j + k) * 0x87654321;
                    b5 = (i - j - k) | 0x55555555;
                    b6 = (i ^ j ^ k) & 0xAAAAAAAA;
                    FORCE_USE(a8); FORCE_USE(a9); FORCE_USE(b5); FORCE_USE(b6);
                    
                    /* Nested continue/break with goto to create complex edges */
                    if ((i + j + k) % 11 == 0) {
                        goto start_label;  /* Back-edge to create loop */
                    }
                    if ((i + j + k) % 13 == 0) {
                        goto exit_label;   /* Early exit */
                    }
                }
                b7 = j * 0x11111111;
                b8 = j * 0x22222222;
                FORCE_USE(b7); FORCE_USE(b8);
            }
        }
        
        b9 = result;
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        FORCE_USE(b9);
    }
    
    exit_label:
    return result;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE
int pattern_b_new_indices(int iterations, int seed) {
    static jmp_buf env;
    int result = seed;
    int i;
    
    /* Many scalar variables to increase register pressure */
    int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
    int y0, y1, y2, y3, y4, y5, y6, y7, y8, y9;
    
    if (setjmp(env) == 0) {
        for (i = 0; i < iterations; i++) {
            x0 = i * 1; x1 = i * 2; x2 = i * 3; x3 = i * 4; x4 = i * 5;
            x5 = i * 6; x6 = i * 7; x7 = i * 8; x8 = i * 9; x9 = i * 10;
            
            y0 = x0 ^ 0xAA; y1 = x1 ^ 0xBB; y2 = x2 ^ 0xCC; y3 = x3 ^ 0xDD; y4 = x4 ^ 0xEE;
            y5 = x5 ^ 0xFF; y6 = x6 ^ 0x11; y7 = x7 ^ 0x22; y8 = x8 ^ 0x33; y9 = x9 ^ 0x44;
            
            FORCE_USE(x0); FORCE_USE(x1); FORCE_USE(x2); FORCE_USE(x3); FORCE_USE(x4);
            FORCE_USE(x5); FORCE_USE(x6); FORCE_USE(x7); FORCE_USE(x8); FORCE_USE(x9);
            FORCE_USE(y0); FORCE_USE(y1); FORCE_USE(y2); FORCE_USE(y3); FORCE_USE(y4);
            FORCE_USE(y5); FORCE_USE(y6); FORCE_USE(y7); FORCE_USE(y8); FORCE_USE(y9);
            
            /* Occasionally trigger longjmp to create exceptional edge */
            if (i % 100 == 99) {
                longjmp(env, 1);
            }
            
            result = (result + x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9) & 0xFFFF;
        }
    } else {
        /* longjmp target - creates exceptional control flow */
        result = (result * 3) & 0xFFFF;
    }
    
    return result;
}

/* Pattern C: Mixed pressure with vector operations and large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations, int seed) {
    int result = seed;
    int i;
    
    /* Many scalar variables */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0, v1, v2, v3, v4;
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Initialize scalars */
        r0 = i * 1; r1 = i * 2; r2 = i * 3; r3 = i * 4; r4 = i * 5;
        r5 = i * 6; r6 = i * 7; r7 = i * 8; r8 = i * 9; r9 = i * 10;
        r10 = i * 11; r11 = i * 12; r12 = i * 13; r13 = i * 14; r14 = i * 15; r15 = i * 16;
        
#ifdef __GNUC__
        /* Initialize vectors */
        v0 = (v4si){r0, r1, r2, r3};
        v1 = (v4si){r4, r5, r6, r7};
        v2 = (v4si){r8, r9, r10, r11};
        v3 = (v4si){r12, r13, r14, r15};
        v4 = v0 + v1 + v2 + v3;
#endif
        
        /* Large switch statement with 30+ cases */
        switch (i % 35) {
            case 0: r0 = r1 + r2; r1 = r3 * r4; break;
            case 1: r2 = r5 - r6; r3 = r7 / (r8 ? r8 : 1); break;
            case 2: r4 = r9 ^ r10; r5 = r11 | r12; break;
            case 3: r6 = r13 & r14; r7 = r15 << 2; break;
            case 4: r8 = r0 >> 1; r9 = r1 + 1; break;
            case 5: r10 = r2 * 3; r11 = r3 % 7; break;
            case 6: r12 = r4 ^ 0xFF; r13 = r5 | 0xAA; break;
            case 7: r14 = r6 & 0x55; r15 = r7 << 3; break;
            case 8: r0 = r8 >> 2; r1 = r9 + 2; break;
            case 9: r2 = r10 * 5; r3 = r11 % 11; break;
            case 10: r4 = r12 ^ 0xCC; r5 = r13 | 0x33; break;
            case 11: r6 = r14 & 0x66; r7 = r15 << 1; break;
            case 12: r8 = r0 >> 3; r9 = r1 + 3; break;
            case 13: r10 = r2 * 7; r11 = r3 % 13; break;
            case 14: r12 = r4 ^ 0x99; r13 = r5 | 0x44; break;
            case 15: r14 = r6 & 0x22; r15 = r7 << 4; break;
            case 16: r0 = r8 >> 4; r1 = r9 + 4; break;
            case 17: r2 = r10 * 11; r3 = r11 % 17; break;
            case 18: r4 = r12 ^ 0x77; r5 = r13 | 0x88; break;
            case 19: r6 = r14 & 0x11; r7 = r15 << 2; break;
            case 20: r8 = r0 >> 5; r9 = r1 + 5; break;
            case 21: r10 = r2 * 13; r11 = r3 % 19; break;
            case 22: r12 = r4 ^ 0x55; r13 = r5 | 0xAA; break;
            case 23: r14 = r6 & 0xBB; r15 = r7 << 3; break;
            case 24: r0 = r8 >> 6; r1 = r9 + 6; break;
            case 25: r2 = r10 * 17; r3 = r11 % 23; break;
            case 26: r4 = r12 ^ 0x33; r5 = r13 | 0xCC; break;
            case 27: r6 = r14 & 0xDD; r7 = r15 << 1; break;
            case 28: r8 = r0 >> 7; r9 = r1 + 7; break;
            case 29: r10 = r2 * 19; r11 = r3 % 29; break;
            case 30: r12 = r4 ^ 0x11; r13 = r5 | 0xEE; break;
            case 31: r14 = r6 & 0xFF; r15 = r7 << 5; break;
            case 32: r0 = r8 >> 8; r1 = r9 + 8; break;
            case 33: r2 = r10 * 23; r3 = r11 % 31; break;
            case 34: r4 = r12 ^ 0x22; r5 = r13 | 0xDD; break;
        }
        
        /* Force all variables to be considered live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3); FORCE_USE(r4);
        FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7); FORCE_USE(r8); FORCE_USE(r9);
        FORCE_USE(r10); FORCE_USE(r11); FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        
#ifdef __GNUC__
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3); FORCE_USE(v4);
#endif
        
        result = (result + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + 
                  r10 + r11 + r12 + r13 + r14 + r15) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Pattern D: Artificial register conflicts */
NOINLINE
void dummy_helper1(int a, int b, int c, int d) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
}

NOINLINE
void dummy_helper2(int a, int b, int c, int d, int e, int f) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d); FORCE_USE(e); FORCE_USE(f);
}

NOINLINE
int pattern_d_register_conflict(int iterations, int seed) {
    int result = seed;
    int i;
    
    /* Explicit register variables to create conflicts */
#ifdef __GNUC__
    register int x asm ("r10");
    register int y asm ("r11");
    register int z asm ("r12");
    register int w asm ("r13");
#else
    int x, y, z, w;
#endif
    
    /* Many other variables that will conflict */
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    x = seed * 1;
    y = seed * 2;
    z = seed * 3;
    w = seed * 4;
    
    for (i = 0; i < iterations; i++) {
        /* Use register variables in complex ways */
        a0 = x + y;
        a1 = z - w;
        a2 = x * y;
        a3 = z / (w ? w : 1);
        a4 = x ^ y;
        a5 = z | w;
        a6 = x & y;
        a7 = z << 2;
        a8 = x >> 1;
        a9 = y + z;
        
        /* Call dummy functions to force spills/restores */
        dummy_helper1(x, y, z, w);
        dummy_helper2(a0, a1, a2, a3, a4, a5);
        
        /* More computations creating live-range splits */
        b0 = a0 * a1;
        b1 = a2 + a3;
        b2 = a4 ^ a5;
        b3 = a6 | a7;
        b4 = a8 & a9;
        b5 = b0 - b1;
        b6 = b2 * b3;
        b7 = b4 / (b5 ? b5 : 1);
        b8 = b6 ^ b7;
        b9 = b8 << 1;
        
        /* Update register variables */
        x = (x + a0) & 0xFF;
        y = (y + a1) & 0xFF;
        z = (z + a2) & 0xFF;
        w = (w + a3) & 0xFF;
        
        /* Force all variables live */
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3); FORCE_USE(a4);
        FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7); FORCE_USE(a8); FORCE_USE(a9);
        FORCE_USE(b0); FORCE_USE(b1); FORCE_USE(b2); FORCE_USE(b3); FORCE_USE(b4);
        FORCE_USE(b5); FORCE_USE(b6); FORCE_USE(b7); FORCE_USE(b8); FORCE_USE(b9);
        FORCE_USE(x); FORCE_USE(y); FORCE_USE(z); FORCE_USE(w);
        
        result = (result + x + y + z + w + 
                  a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
                  b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Main driver that stresses all patterns */
int main(int argc, char *argv[]) {
    volatile int total = 0;  /* volatile to prevent optimization */
    int i, iterations;
    
    /* Determine iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    } else {
        iterations = 1000;
    }
    
    if (verbose) {
        printf("Running MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        if (verbose) printf("AVX2 supported - engaging vector register pressure\n");
    }
#endif
    
    /* Stress all patterns multiple times */
    for (i = 0; i < 10; i++) {
        total += pattern_a_entry_exit(iterations / 10, i * 12345);
        total += pattern_b_new_indices(iterations / 10, i * 23456);
        total += pattern_c_mixed_pressure(iterations / 10, i * 34567);
        total += pattern_d_register_conflict(iterations / 10, i * 45678);
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return total & 0xFF;  /* Return non-zero to indicate success */
}
