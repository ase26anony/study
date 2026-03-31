/* test_mcf.c - Complex CFG generator to trigger MCF special node printing */
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

/* ==================== PATTERN A: ENTRY/EXIT BLOCKS ==================== */
/* Creates irreducible region with goto to force ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    int r0 = seed, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10, r11 = 11;
    int r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    int result = 0;
    
    /* Label to create irreducible region */
    irreducible_region:
    
    for (int i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (r0 & 1) {
            if (r1 & 2) {
                r2 = r3 * r4;
                if (r2 > 100) {
                    r5 = r6 - r7;
                    goto skip_middle; /* Create irreducible flow */
                } else {
                    r8 = r9 ^ r10;
                }
            } else {
                r11 = r12 | r13;
                if (r11 < 50) {
                    goto irreducible_region; /* Back edge to label */
                }
            }
            r14 = r15 << 2;
        } else {
            if (r3 & 4) {
                r4 = r5 / (r6 ? r6 : 1);
                if (r4 == 0) {
                    continue; /* Complex loop control */
                }
            }
            r7 = r8 % (r9 ? r9 : 1);
        }
        
        skip_middle:
        
        /* Another level of nesting */
        switch (r0 % 5) {
            case 0: r0 = r1 + r2; break;
            case 1: r1 = r2 - r3; break;
            case 2: r2 = r3 * r4; break;
            case 3: r3 = r4 / (r5 ? r5 : 1); break;
            case 4: goto irreducible_region; /* More irreducibility */
        }
        
        /* Force all variables live */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
                          "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                          "g"(r12), "g"(r13), "g"(r14), "g"(r15));
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    return result;
}

/* ==================== PATTERN B: NEW_EXIT/NEW_ENTRY ==================== */
/* Uses setjmp/longjmp for exceptional edges */
static jmp_buf jump_buffer;
NOINLINE USED
int pattern_b_new_indices(int depth, int max_depth) {
    volatile int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6;
    volatile int v6 = 7, v7 = 8, v8 = 9, v9 = 10, v10 = 11, v11 = 12;
    volatile int v12 = 13, v13 = 14, v14 = 15, v15 = 16;
    int sum = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal path with many live variables */
        for (int i = 0; i < 100; i++) {
            v0 = v1 + v2;
            v1 = v3 * v4;
            v2 = v5 ^ v6;
            v3 = v7 | v8;
            v4 = v9 & v10;
            v5 = v11 << v12;
            v6 = v13 >> v14;
            v7 = v15 % (v0 ? v0 : 1);
            
            /* Complex condition that might trigger longjmp */
            if (depth < max_depth && (i % 17) == 0) {
                pattern_b_new_indices(depth + 1, max_depth);
            }
            
            /* Force all variables live across potential longjmp */
            asm volatile("" : : "g"(v0), "g"(v1), "g"(v2), "g"(v3), "g"(v4), "g"(v5),
                              "g"(v6), "g"(v7), "g"(v8), "g"(v9), "g"(v10), "g"(v11),
                              "g"(v12), "g"(v13), "g"(v14), "g"(v15));
        }
    } else {
        /* longjmp target - different live ranges */
        v0 = v15;
        v1 = v14;
        v2 = v13;
    }
    
    /* Trigger longjmp from some calls */
    if (depth > 0 && (v0 % 7) == 0) {
        longjmp(jump_buffer, 1);
    }
    
    sum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    return sum;
}

/* ==================== PATTERN C: MIXED PRESSURE ==================== */
/* Combines vector operations with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int selector, int iterations) {
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5, r5 = 6, r6 = 7, r7 = 8;
    int r8 = 9, r9 = 10, r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    
#ifdef __GNUC__
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch ((selector + i) % 35) {
            case 0: r0 = r1 + r2; r1 = r3 * r4; break;
            case 1: r2 = r3 - r4; r3 = r5 ^ r6; break;
            case 2: r4 = r5 | r6; r5 = r7 & r8; break;
            case 3: r6 = r7 << 1; r7 = r8 >> 2; break;
            case 4: r8 = r9 % 7; r9 = r10 * 3; break;
            case 5: r10 = r11 + r12; r11 = r13 - r14; break;
            case 6: r12 = r13 * r14; r13 = r15 / 2; break;
            case 7: r14 = r0 ^ r1; r15 = r2 | r3; break;
            case 8: r0 = r4 & r5; r1 = r6 << 3; break;
            case 9: r2 = r7 >> 1; r3 = r8 % 5; break;
            case 10: r4 = r9 * 11; r5 = r10 + 17; break;
            case 11: r6 = r11 - 19; r7 = r12 * 23; break;
            case 12: r8 = r13 ^ 29; r9 = r14 | 31; break;
            case 13: r10 = r15 & 37; r11 = r0 << 2; break;
            case 14: r12 = r1 >> 1; r13 = r2 % 3; break;
            case 15: r14 = r3 * 5; r15 = r4 + 7; break;
            case 16: r0 = r5 - 11; r1 = r6 * 13; break;
            case 17: r2 = r7 ^ 17; r3 = r8 | 19; break;
            case 18: r4 = r9 & 23; r5 = r10 << 1; break;
            case 19: r6 = r11 >> 2; r7 = r12 % 7; break;
            case 20: r8 = r13 * 11; r9 = r14 + 13; break;
            case 21: r10 = r15 - 17; r11 = r0 * 19; break;
            case 22: r12 = r1 ^ 23; r13 = r2 | 29; break;
            case 23: r14 = r3 & 31; r15 = r4 << 3; break;
            case 24: r0 = r5 >> 1; r1 = r6 % 2; break;
            case 25: r2 = r7 * 3; r3 = r8 + 5; break;
            case 26: r4 = r9 - 7; r5 = r10 * 11; break;
            case 27: r6 = r11 ^ 13; r7 = r12 | 17; break;
            case 28: r8 = r13 & 19; r9 = r14 << 2; break;
            case 29: r10 = r15 >> 1; r11 = r0 % 3; break;
            case 30: r12 = r1 * 5; r13 = r2 + 7; break;
            case 31: r14 = r3 - 11; r15 = r4 * 13; break;
            case 32: r0 = r5 ^ 17; r1 = r6 | 19; break;
            case 33: r2 = r7 & 23; r3 = r8 << 1; break;
            case 34: r4 = r9 >> 2; r5 = r10 % 5; break;
        }
        
#ifdef __GNUC__
        /* Vector operations mixed in */
        if (i % 3 == 0) {
            vec0 = vec0 + vec1;
            vec1 = vec1 * vec2;
            vec2 = vec2 - vec3;
            vec3 = vec3 & vec0;
            
            /* Extract elements to scalar */
            r0 += vec0[0];
            r1 += vec1[1];
            r2 += vec2[2];
            r3 += vec3[3];
        }
#endif
        
        /* Complex loop control with continue to different cases */
        if ((i % 7) == 0) {
            selector = (selector * 1103515245 + 12345) & 0x7fffffff;
            if ((selector % 11) == 0) {
                continue; /* Skip to next iteration */
            } else if ((selector % 13) == 0) {
                i++; /* Skip a case */
                continue;
            }
        }
        
        /* Force all scalars live */
        asm volatile("" : : "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
                          "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
                          "g"(r12), "g"(r13), "g"(r14), "g"(r15));
    }
    
#ifdef __GNUC__
    /* Use vector results */
    result = vec0[0] + vec1[1] + vec2[2] + vec3[3];
#endif
    result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    return result;
}

/* ==================== PATTERN D: ARTIFICIAL CONFLICT ==================== */
/* Uses explicit register variables and noinline calls */
#ifdef __GNUC__
register int reg_a asm ("r10");
register int reg_b asm ("r11");
#endif

NOINLINE void dummy_helper1(int a, int b, int c, int d, int e, int f) {
    asm volatile("" : : "g"(a), "g"(b), "g"(c), "g"(d), "g"(e), "g"(f));
}

NOINLINE void dummy_helper2(int a, int b, int c, int d, int e, int f) {
    asm volatile("" : : "g"(a), "g"(b), "g"(c), "g"(d), "g"(e), "g"(f));
}

NOINLINE USED
int pattern_d_register_conflict(int param) {
    int local0 = param * 2;
    int local1 = param * 3;
    int local2 = param * 5;
    int local3 = param * 7;
    int local4 = param * 11;
    int local5 = param * 13;
    int local6 = param * 17;
    int local7 = param * 19;
    int local8 = param * 23;
    int local9 = param * 29;
    int local10 = param * 31;
    int local11 = param * 37;
    
#ifdef __GNUC__
    /* Use explicit register variables in conflicting ways */
    reg_a = local0;
    reg_b = local1;
    
    /* Force spills by using many variables */
    dummy_helper1(local2, local3, local4, local5, local6, local7);
    
    reg_a = local8;
    dummy_helper2(local9, local10, local11, reg_a, reg_b, local0);
    
    reg_b = local2;
    dummy_helper1(local3, local4, local5, reg_a, reg_b, local6);
    
    /* More conflicts */
    int temp = reg_a;
    reg_a = reg_b;
    reg_b = temp;
    
    dummy_helper2(reg_a, reg_b, local7, local8, local9, local10);
#endif
    
    /* Large switch to create many basic blocks */
    switch (param % 20) {
        case 0: local0 += local1; break;
        case 1: local1 += local2; break;
        case 2: local2 += local3; break;
        case 3: local3 += local4; break;
        case 4: local4 += local5; break;
        case 5: local5 += local6; break;
        case 6: local6 += local7; break;
        case 7: local7 += local8; break;
        case 8: local8 += local9; break;
        case 9: local9 += local10; break;
        case 10: local10 += local11; break;
        case 11: local11 += local0; break;
        case 12: local0 -= local1; break;
        case 13: local1 -= local2; break;
        case 14: local2 -= local3; break;
        case 15: local3 -= local4; break;
        case 16: local4 -= local5; break;
        case 17: local5 -= local6; break;
        case 18: local6 -= local7; break;
        case 19: local7 -= local8; break;
    }
    
#ifdef __GNUC__
    /* Final use of register variables */
    asm volatile("" : : "g"(reg_a), "g"(reg_b));
#endif
    
    return local0 + local1 + local2 + local3 + local4 + local5 + local6 + 
           local7 + local8 + local9 + local10 + local11;
}

/* ==================== MAIN DRIVER ==================== */
int main(int argc, char *argv[]) {
    int total = 0;
    
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 supported: %d\n", use_avx2);
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(50 + (i % 10), i * 12345);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 3 == 0) {
            total += pattern_b_new_indices(0, 2 + (i % 3));
        }
        
        /* Pattern C - Mixed pressure with vectors */
        total += pattern_c_mixed_pressure(i, 30 + (i % 20));
        
        /* Pattern D - Register conflicts */
        total += pattern_d_register_conflict(i * 54321);
        
        /* Prevent optimization */
        global_sink = total;
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    return total != 0 ? 0 : 1;
}
