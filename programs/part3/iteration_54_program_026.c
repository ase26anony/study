/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage */
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

/* Pattern A: Deeply nested if-else with irreducible region for ENTRY/EXIT blocks */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Label the first basic block to encourage ENTRY_BLOCK identification */
    start_label:
    
    for (i = 0; i < iterations; i++) {
        /* Deep nested if-else chain */
        if (r0 & 1) {
            if (r1 & 2) {
                for (j = 0; j < 5; j++) {
                    if (r2 & (1 << j)) {
                        r3 += r4 * j;
                        /* Create irreducible region with goto */
                        if (r3 > 1000) goto irreducible_label;
                    } else {
                        r5 -= r6 / (j + 1);
                    }
                }
            } else if (r7 < r8) {
                r9 = r10 ^ r11;
                /* Another goto creating complex control flow */
                if (r9 == 0) goto start_label;
            }
        } else {
            /* Switch inside else to create more basic blocks */
            switch (r12 & 3) {
                case 0: r13 = r14 + r15; break;
                case 1: r14 = r13 - r15; break;
                case 2: r15 = r13 * r14; break;
                case 3: r13 = r14 / (r15 ? r15 : 1); break;
            }
        }
        
        irreducible_label:
        /* Complex arithmetic using all variables */
        r0 = r1 + r2 - r3 * r4 / (r5 ? r5 : 1);
        r1 = r2 ^ r3 | r4 & r5;
        r2 = r3 * r4 - r5 + r6;
        r3 = r4 / (r6 ? r6 : 1) + r7 - r8;
        r4 = r5 | r6 & r7 ^ r8;
        r5 = r6 + r7 * r8 - r9;
        r6 = r7 - r8 + r9 * r10;
        
        /* Force all variables to be considered live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        
        /* Break to different points based on condition */
        if (result > 1000000) {
            break;
        } else if (result < -1000000) {
            continue;
        }
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                  r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    }
    
    return result;
}

/* Pattern B: setjmp/longjmp for exceptional edges requiring NEW_EXIT/NEW_ENTRY */
NOINLINE
int pattern_b_new_indices(int iterations) {
    jmp_buf env;
    volatile int result = 0;
    int i, j;
    
    /* Many scalar variables to increase register pressure */
    int s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5, s5 = 6, s6 = 7, s7 = 8;
    int s8 = 9, s9 = 10, s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16;
    
    if (setjmp(env) == 0) {
        /* Normal execution path */
        for (i = 0; i < iterations; i++) {
            /* Complex loop with many computations */
            for (j = 0; j < 100; j++) {
                s0 = s1 + s2;
                s1 = s2 * s3 - s4;
                s2 = s3 / (s5 ? s5 : 1) + s6;
                s3 = s4 ^ s5 | s6 & s7;
                s4 = s5 + s6 - s7 * s8;
                s5 = s6 | s7 & s8 ^ s9;
                s6 = s7 - s8 + s9 * s10;
                s7 = s8 / (s10 ? s10 : 1) - s11;
                
                result += s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7;
                
                /* Force all variables live */
                FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10);
                FORCE_USE(s11); FORCE_USE(s12); FORCE_USE(s13);
                FORCE_USE(s14); FORCE_USE(s15);
                
                /* Occasionally longjmp to create exceptional edge */
                if ((i * j) % 137 == 0) {
                    longjmp(env, 1);
                }
            }
        }
    } else {
        /* longjmp target - different execution path */
        for (i = 0; i < 50; i++) {
            s8 = s9 * s10 - s11;
            s9 = s10 + s11 / (s12 ? s12 : 1);
            s10 = s11 ^ s12 | s13;
            s11 = s12 - s13 * s14 + s15;
            result -= s8 + s9 + s10 + s11;
        }
    }
    
    return result;
}

/* Pattern C: Vector operations combined with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations, int selector) {
    volatile int result = 0;
    int i;
    
    /* Many integer variables */
    int i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7, i7 = 8;
    int i8 = 9, i9 = 10, i10 = 11, i11 = 12, i12 = 13, i13 = 14, i14 = 15, i15 = 16;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {1, 2, 3, 4};
    v4si v1 = {5, 6, 7, 8};
    v4si v2 = {9, 10, 11, 12};
    v4si v3 = {13, 14, 15, 16};
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Large switch statement with 30+ cases */
        switch ((selector + i) % 35) {
            case 0:
                i0 = i1 + i2; i1 = i2 * i3; i2 = i3 - i4;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                break;
            case 1:
                i3 = i4 / (i5 ? i5 : 1); i4 = i5 ^ i6; i5 = i6 | i7;
#ifdef __GNUC__
                v1 = v2 - v3;
#endif
                break;
            case 2:
                i6 = i7 & i8; i7 = i8 + i9; i8 = i9 * i10;
                break;
            case 3:
                i9 = i10 - i11; i10 = i11 / (i12 ? i12 : 1); i11 = i12 ^ i13;
#ifdef __GNUC__
                v2 = v3 * v0;
#endif
                break;
            case 4:
                i12 = i13 | i14; i13 = i14 & i15; i14 = i15 + i0;
                break;
            case 5:
                i15 = i0 * i1; i0 = i1 - i2; i1 = i2 / (i3 ? i3 : 1);
#ifdef __GNUC__
                v3 = v0 + v1;
#endif
                break;
            /* 29 more cases... */
            case 6: i2 = i3 ^ i4; i3 = i4 | i5; i4 = i5 & i6; break;
            case 7: i5 = i6 + i7; i6 = i7 * i8; i7 = i8 - i9; break;
            case 8: i8 = i9 / (i10 ? i10 : 1); i9 = i10 ^ i11; i10 = i11 | i12; break;
            case 9: i11 = i12 & i13; i12 = i13 + i14; i13 = i14 * i15; break;
            case 10: i14 = i15 - i0; i15 = i0 / (i1 ? i1 : 1); i0 = i1 ^ i2; break;
            case 11: i1 = i2 | i3; i2 = i3 & i4; i3 = i4 + i5; break;
            case 12: i4 = i5 * i6; i5 = i6 - i7; i6 = i7 / (i8 ? i8 : 1); break;
            case 13: i7 = i8 ^ i9; i8 = i9 | i10; i9 = i10 & i11; break;
            case 14: i10 = i11 + i12; i11 = i12 * i13; i12 = i13 - i14; break;
            case 15: i13 = i14 / (i15 ? i15 : 1); i14 = i15 ^ i0; i15 = i0 | i1; break;
            case 16: i0 = i1 & i2; i1 = i2 + i3; i2 = i3 * i4; break;
            case 17: i3 = i4 - i5; i4 = i5 / (i6 ? i6 : 1); i5 = i6 ^ i7; break;
            case 18: i6 = i7 | i8; i7 = i8 & i9; i8 = i9 + i10; break;
            case 19: i9 = i10 * i11; i10 = i11 - i12; i11 = i12 / (i13 ? i13 : 1); break;
            case 20: i12 = i13 ^ i14; i13 = i14 | i15; i14 = i15 & i0; break;
            case 21: i15 = i0 + i1; i0 = i1 * i2; i1 = i2 - i3; break;
            case 22: i2 = i3 / (i4 ? i4 : 1); i3 = i4 ^ i5; i4 = i5 | i6; break;
            case 23: i5 = i6 & i7; i6 = i7 + i8; i7 = i8 * i9; break;
            case 24: i8 = i9 - i10; i9 = i10 / (i11 ? i11 : 1); i10 = i11 ^ i12; break;
            case 25: i11 = i12 | i13; i12 = i13 & i14; i13 = i14 + i15; break;
            case 26: i14 = i15 * i0; i15 = i0 - i1; i0 = i1 / (i2 ? i2 : 1); break;
            case 27: i1 = i2 ^ i3; i2 = i3 | i4; i3 = i4 & i5; break;
            case 28: i4 = i5 + i6; i5 = i6 * i7; i6 = i7 - i8; break;
            case 29: i7 = i8 / (i9 ? i9 : 1); i8 = i9 ^ i10; i9 = i10 | i11; break;
            case 30: i10 = i11 & i12; i11 = i12 + i13; i12 = i13 * i14; break;
            case 31: i13 = i14 - i15; i14 = i15 / (i0 ? i0 : 1); i15 = i0 ^ i1; break;
            case 32: i0 = i1 | i2; i1 = i2 & i3; i2 = i3 + i4; break;
            case 33: i3 = i4 * i5; i4 = i5 - i6; i5 = i6 / (i7 ? i7 : 1); break;
            case 34: i6 = i7 ^ i8; i7 = i8 | i9; i8 = i9 & i10; break;
        }
        
        /* Force all integer variables live */
        FORCE_USE(i0); FORCE_USE(i1); FORCE_USE(i2); FORCE_USE(i3);
        FORCE_USE(i4); FORCE_USE(i5); FORCE_USE(i6); FORCE_USE(i7);
        FORCE_USE(i8); FORCE_USE(i9); FORCE_USE(i10); FORCE_USE(i11);
        FORCE_USE(i12); FORCE_USE(i13); FORCE_USE(i14); FORCE_USE(i15);
        
#ifdef __GNUC__
        /* Force all vector variables live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
        
        result += i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + 
                  i8 + i9 + i10 + i11 + i12 + i13 + i14 + i15;
        
        /* Complex loop control */
        if (result > 1000000) {
            continue;
        } else if (result < -1000000) {
            break;
        }
    }
    
    return result;
}

/* Dummy helper functions for Pattern D */
NOINLINE void dummy1(int a, int b, int c) {
    volatile int x = a + b + c;
    FORCE_USE(x);
}

NOINLINE void dummy2(int a, int b, int c, int d) {
    volatile int x = a * b - c + d;
    FORCE_USE(x);
}

NOINLINE void dummy3(int a, int b) {
    volatile int x = a ^ b;
    FORCE_USE(x);
}

/* Pattern D: Explicit register variables for artificial conflicts */
NOINLINE
int pattern_d_register_conflict(int iterations) {
    volatile int result = 0;
    int i;
    
    /* Explicit register variables that may conflict */
#ifdef __GNUC__
    register int x asm("r10") = 1;
    register int y asm("r11") = 2;
    register int z asm("r12") = 3;
    register int w asm("r13") = 4;
#else
    int x = 1, y = 2, z = 3, w = 4;
#endif
    
    /* Many other local variables */
    int a0 = 5, a1 = 6, a2 = 7, a3 = 8, a4 = 9, a5 = 10;
    int a6 = 11, a7 = 12, a8 = 13, a9 = 14, a10 = 15, a11 = 16;
    
    for (i = 0; i < iterations; i++) {
        /* Use register variables in complex ways */
        x = y + z;
        y = z * w - x;
        z = w / (x ? x : 1) + y;
        w = x ^ y | z & w;
        
        /* Call dummy functions that may clobber registers */
        dummy1(x, y, z);
        dummy2(w, a0, a1, a2);
        dummy3(a3, a4);
        
        /* More computations */
        a0 = a1 + a2 - a3;
        a1 = a2 * a3 / (a4 ? a4 : 1);
        a2 = a3 ^ a4 | a5;
        a3 = a4 & a5 + a6;
        a4 = a5 - a6 * a7;
        a5 = a6 / (a7 ? a7 : 1) + a8;
        a6 = a7 | a8 & a9 ^ a10;
        a7 = a8 + a9 - a10 * a11;
        
        /* Force all variables live */
        FORCE_USE(x); FORCE_USE(y); FORCE_USE(z); FORCE_USE(w);
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
        FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
        FORCE_USE(a8); FORCE_USE(a9); FORCE_USE(a10); FORCE_USE(a11);
        
        result += x + y + z + w + a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
        
        /* Complex loop control */
        switch (i % 7) {
            case 0: continue;
            case 1: break;
            case 2: result *= 2; continue;
            case 3: result /= 2; break;
            case 4: x = y; continue;
            case 5: y = z; break;
            case 6: z = w; continue;
        }
    }
    
    return result;
}

/* Main function with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations;
    
    /* Determine iterations based on input or default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    if (argc > 2) {
        verbose = atoi(argv[2]);
    }
    
    if (verbose) {
        printf("Starting MCF coverage test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
#ifdef __GNUC__
    int has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", has_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        /* Pattern A - targeting ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(10 + (i % 5), i);
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY indices */
        total += pattern_b_new_indices(5 + (i % 3));
        
        /* Pattern C - mixed integer/vector pressure */
        total += pattern_c_mixed_pressure(8 + (i % 4), i);
        
        /* Pattern D - register variable conflicts */
        total += pattern_d_register_conflict(6 + (i % 3));
        
        /* Prevent optimization of the loop */
        FORCE_USE(total);
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    return total != 0 ? 0 : 1;
}
