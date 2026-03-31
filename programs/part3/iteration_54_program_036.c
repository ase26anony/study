/* test_mcf.c - Complex CFG generator for MCF pass coverage */
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
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create complex irreducible region with goto */
    if (iterations > 0) {
        int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
        int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
        
        /* Irreducible region created by forward goto */
        if (r0 & 1) goto label1;
        if (r1 & 2) goto label2;
        
        for (i = 0; i < iterations; i++) {
            /* Deeply nested if-else chain */
            if (r0 < 100) {
                if (r1 < 200) {
                    if (r2 < 300) {
                        r3 = r0 * r1 + r2;
                    } else {
                        r3 = r0 * r2 - r1;
                    }
                } else {
                    if (r2 > 150) {
                        r3 = r1 * r2 / (r0 + 1);
                    } else {
                        r3 = r2 * r0 % (r1 + 1);
                    }
                }
            } else {
                if (r1 > 50) {
                    if (r2 > 100) {
                        r3 = (r0 + r1) * r2;
                    } else {
                        r3 = (r0 - r1) * r2;
                    }
                } else {
                    if (r2 < 200) {
                        r3 = r0 * r1 * r2;
                    } else {
                        r3 = r0 + r1 + r2;
                    }
                }
            }
            
            /* More variables to increase pressure */
            r4 = r3 ^ r0;
            r5 = r4 | r1;
            r6 = r5 & r2;
            r7 = r6 + r3;
            
            /* Complex loop with continue to different points */
            if (r7 % 3 == 0) continue;
            if (r7 % 5 == 0) goto label1;
            if (r7 % 7 == 0) goto label2;
            
            /* Additional arithmetic to prevent optimization */
            r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
            r1 = (r1 * 1664525 + 1013904223) & 0x7fffffff;
            r2 = (r2 * 134775813 + 1) & 0x7fffffff;
            
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2);
            FORCE_USE(r3); FORCE_USE(r4); FORCE_USE(r5);
            FORCE_USE(r6); FORCE_USE(r7);
        }
        
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    }
    
    return result;

label1:
    {
        int t0 = seed * 3, t1 = seed * 5, t2 = seed * 7;
        FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2);
        return seed * 11;
    }

label2:
    {
        int t0 = seed * 13, t1 = seed * 17, t2 = seed * 19;
        FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2);
        return seed * 23;
    }
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static int jump_counter = 0;

NOINLINE
int pattern_b_new_indices(int depth, int value) {
    int r0 = value, r1 = value + 1, r2 = value + 2, r3 = value + 3;
    int r4 = value + 4, r5 = value + 5, r6 = value + 6, r7 = value + 7;
    int r8 = value + 8, r9 = value + 9, r10 = value + 10, r11 = value + 11;
    int r12 = value + 12, r13 = value + 13, r14 = value + 14, r15 = value + 15;
    
    if (setjmp(jump_buffer) == 0) {
        /* Complex loop with many variables live across setjmp */
        for (int i = 0; i < depth; i++) {
            /* Heavy arithmetic on all variables */
            r0 = r0 * 3 + r1;
            r1 = r1 * 5 + r2;
            r2 = r2 * 7 + r3;
            r3 = r3 * 11 + r4;
            r4 = r4 * 13 + r5;
            r5 = r5 * 17 + r6;
            r6 = r6 * 19 + r7;
            r7 = r7 * 23 + r8;
            r8 = r8 * 29 + r9;
            r9 = r9 * 31 + r10;
            r10 = r10 * 37 + r11;
            r11 = r11 * 41 + r12;
            r12 = r12 * 43 + r13;
            r13 = r13 * 47 + r14;
            r14 = r14 * 53 + r15;
            r15 = r15 * 59 + r0;
            
            /* Conditional longjmp creates exceptional edge */
            if (jump_counter++ > 100 && (i % 17 == 0)) {
                longjmp(jump_buffer, 1);
            }
            
            /* Force all variables to be considered live */
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
            FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        }
    } else {
        /* Land here after longjmp - different live ranges */
        r0 = r0 ^ r1 ^ r2 ^ r3;
        r4 = r4 | r5 | r6 | r7;
        r8 = r8 & r9 & r10 & r11;
        r12 = r12 + r13 + r14 + r15;
    }
    
    return r0 + r4 + r8 + r12;
}

/* ========== PATTERN C: MIXED PRESSURE with VECTOR OPS ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int selector, int iterations) {
    int result = 0;
    
    /* Scalar variables */
    int s0 = selector, s1 = selector + 1, s2 = selector + 2, s3 = selector + 3;
    int s4 = selector + 4, s5 = selector + 5, s6 = selector + 6, s7 = selector + 7;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {selector, selector+1, selector+2, selector+3};
    v4si v1 = {selector+4, selector+5, selector+6, selector+7};
    v4si v2 = {selector+8, selector+9, selector+10, selector+11};
    v4si v3 = {selector+12, selector+13, selector+14, selector+15};
#endif
    
    /* Large switch statement with 30+ cases */
    for (int i = 0; i < iterations; i++) {
        switch ((s0 + i) % 35) {
            case 0:
                s0 = s1 * s2 + s3;
                s1 = s2 * s3 - s4;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                break;
            case 1:
                s2 = s3 * s4 / (s5 + 1);
                s3 = s4 * s5 % (s6 + 1);
#ifdef __GNUC__
                v1 = v2 - v0;
#endif
                break;
            case 2:
                s4 = s5 ^ s6 ^ s7;
                s5 = s6 | s7 | s0;
#ifdef __GNUC__
                v2 = v0 * v1;
#endif
                break;
            case 3:
                s6 = s7 + s0 + s1;
                s7 = s0 - s1 + s2;
#ifdef __GNUC__
                v3 = v1 + v2 + v0;
#endif
                break;
            case 4:
                s0 = s1 << (s2 & 3);
                s1 = s2 >> (s3 & 3);
#ifdef __GNUC__
                v0 = v2 << 1;
#endif
                break;
            case 5:
                s2 = ~s3 & s4;
                s3 = s4 | ~s5;
#ifdef __GNUC__
                v1 = v3 & v0;
#endif
                break;
            /* 29 more similar cases with unique arithmetic */
            case 6: s4 = s5 * 3 + s6; s5 = s6 * 5 - s7; break;
            case 7: s6 = s7 * 7 ^ s0; s7 = s0 * 11 | s1; break;
            case 8: s0 = s1 + s2 * 13; s1 = s2 + s3 * 17; break;
            case 9: s2 = s3 - s4 * 19; s3 = s4 - s5 * 23; break;
            case 10: s4 = s5 / (s6 + 1); s5 = s6 / (s7 + 1); break;
            case 11: s6 = s7 % (s0 + 1); s7 = s0 % (s1 + 1); break;
            case 12: s0 = s1 & s2 & s3; s1 = s2 | s3 | s4; break;
            case 13: s2 = s3 ^ s4 ^ s5; s3 = s4 ^ s5 ^ s6; break;
            case 14: s4 = s5 << 2; s5 = s6 >> 2; break;
            case 15: s6 = s7 << 1; s7 = s0 >> 1; break;
            case 16: s0 = ~s1; s1 = ~s2; break;
            case 17: s2 = s3 * s4 * s5; s3 = s4 * s5 * s6; break;
            case 18: s4 = s5 + s6 + s7; s5 = s6 + s7 + s0; break;
            case 19: s6 = s7 - s0 - s1; s7 = s0 - s1 - s2; break;
            case 20: s0 = (s1 + s2) * s3; s1 = (s2 + s3) * s4; break;
            case 21: s2 = (s3 - s4) * s5; s3 = (s4 - s5) * s6; break;
            case 22: s4 = s5 * 1103515245; s5 = s6 * 1664525; break;
            case 23: s6 = s7 * 134775813; s7 = s0 * 214013; break;
            case 24: s0 = s1 & 0x55555555; s1 = s2 & 0xaaaaaaaa; break;
            case 25: s2 = s3 | 0x55555555; s3 = s4 | 0xaaaaaaaa; break;
            case 26: s4 = s5 ^ 0xffffffff; s5 = s6 ^ 0x00000000; break;
            case 27: s6 = (s7 << 16) | (s7 >> 16); s7 = (s0 << 8) | (s0 >> 24); break;
            case 28: s0 = s1 + (s2 << 1); s1 = s2 + (s3 << 2); break;
            case 29: s2 = s3 - (s4 << 1); s3 = s4 - (s5 << 2); break;
            case 30: s4 = s5 * s6 / 2; s5 = s6 * s7 / 4; break;
            case 31: s6 = s7 + s0 * 2; s7 = s0 + s1 * 3; break;
            case 32: s0 = s1 - s2 * 4; s1 = s2 - s3 * 5; break;
            case 33: s2 = s3 ^ s4 ^ s5 ^ s6; s3 = s4 ^ s5 ^ s6 ^ s7; break;
            case 34:
                s4 = (s5 + s6) * (s7 - s0);
                s5 = (s6 + s7) * (s0 - s1);
                /* Use continue to create back-edge to different case */
                if ((i % 7) == 0) continue;
                break;
        }
        
        /* Force all scalars live */
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        
#ifdef __GNUC__
        /* Force all vectors live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    }
    
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7;
#ifdef __GNUC__
    result += v0[0] + v1[1] + v2[2] + v3[3];
#endif
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
register int reg_var1 asm ("r10");
register int reg_var2 asm ("r11");
#else
int reg_var1, reg_var2;
#endif

NOINLINE void dummy_helper1(int x) {
    FORCE_USE(x);
}

NOINLINE void dummy_helper2(int x, int y) {
    FORCE_USE(x); FORCE_USE(y);
}

NOINLINE void dummy_helper3(int x, int y, int z) {
    FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
}

NOINLINE
int pattern_d_artificial_conflict(int n) {
    int local1 = n, local2 = n * 2, local3 = n * 3;
    int local4 = n * 4, local5 = n * 5, local6 = n * 6;
    
    /* Use register variables conflicting with calls */
    reg_var1 = local1 + local2;
    dummy_helper1(reg_var1);
    
    reg_var2 = local3 + local4;
    dummy_helper2(reg_var1, reg_var2);
    
    /* Force spilling by using many temporaries around calls */
    int temp1 = reg_var1 * local5;
    int temp2 = reg_var2 * local6;
    int temp3 = temp1 + temp2;
    int temp4 = temp1 - temp2;
    int temp5 = temp3 * temp4;
    int temp6 = temp3 / (temp4 + 1);
    
    dummy_helper3(temp1, temp2, temp3);
    
    /* More conflicts */
    reg_var1 = temp4 + temp5;
    reg_var2 = temp5 + temp6;
    
    dummy_helper1(reg_var1);
    dummy_helper2(reg_var1, reg_var2);
    
    /* Complex loop with register variable usage */
    for (int i = 0; i < n % 100; i++) {
        int t1 = reg_var1 + i;
        int t2 = reg_var2 - i;
        int t3 = t1 * t2;
        int t4 = t1 / (t2 + 1);
        
        reg_var1 = t3 + local1;
        reg_var2 = t4 + local2;
        
        dummy_helper3(t1, t2, t3);
        
        FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3); FORCE_USE(t4);
        FORCE_USE(reg_var1); FORCE_USE(reg_var2);
    }
    
    return reg_var1 + reg_var2 + local1 + local2 + local3 + local4 + local5 + local6;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char **argv) {
    volatile int total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 100000) iterations = 100000;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU feature detection to engage target-specific RA */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - targets ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(i % 50 + 10, i * 3 + 1);
        
        /* Pattern B - targets NEW_EXIT/NEW_ENTRY indices */
        if (i % 23 == 0) {
            jump_counter = 0;
            total += pattern_b_new_indices(i % 20 + 5, i * 5 + 2);
        }
        
        /* Pattern C - mixed pressure with vectors */
        total += pattern_c_mixed_pressure(i % 100, i % 10 + 1);
        
        /* Pattern D - artificial register conflicts */
        if (i % 37 == 0) {
            total += pattern_d_artificial_conflict(i % 200 + 50);
        }
        
        /* Prevent loop unrolling */
        if (i % 256 == 0) {
            FORCE_USE(total);
        }
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return total != 0 ? 0 : 1;
}
