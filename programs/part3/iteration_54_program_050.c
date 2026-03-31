/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
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

/* ========== PATTERN A: ENTRY/EXIT blocks with irreducible region ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Deeply nested if-else chain to create complex CFG */
    if (iterations > 0) {
        if (seed % 2 == 0) {
            for (i = 0; i < iterations; i++) {
                if (i % 3 == 0) {
                    r0 = r1 + r2;
                    r3 = r4 * r5;
                    goto label_a;  /* Create irreducible region */
                } else if (i % 3 == 1) {
                    r6 = r7 - r8;
                    r9 = r10 / (r11 ? r11 : 1);
                    goto label_b;
                } else {
                    r12 = r13 | r14;
                    r15 = r0 ^ r1;
                }
                
                /* Back-edge with goto to create loop with multiple entry points */
                if (i % 5 == 0) {
                    continue;
                }
                
            label_a:
                r2 = r3 + r4;
                if (i % 7 == 0) {
                    goto label_c;
                }
                
            label_b:
                r5 = r6 * r7;
                if (i % 11 == 0) {
                    break;
                }
                
            label_c:
                r8 = r9 - r10;
                result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
            }
        } else {
            /* Alternative path with different control flow */
            for (j = iterations; j > 0; j--) {
                switch (j % 4) {
                    case 0: r0++; r1--; break;
                    case 1: r2 *= 2; r3 /= 2; break;
                    case 2: r4 = r5 ^ r6; break;
                    case 3: r7 = r8 | r9; break;
                }
                result += j;
            }
        }
    }
    
    /* Final computation mixing all variables */
    result = r0 + r1 - r2 + r3 * r4 + r5 - r6 + r7 * r8 + r9 - r10 + r11 * r12 + r13 - r14 + r15;
    FORCE_USE(result);
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp/longjmp ========== */
static jmp_buf jump_buffer;
static int jump_counter = 0;

NOINLINE
static void helper_b1(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i * i;
    }
}

NOINLINE
static void helper_b2(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
}

int pattern_b_new_nodes(int depth, int size) {
    volatile int result = 0;
    int i, j;
    
    /* Many scalar variables to pressure registers */
    int a0 = 1, a1 = 2, a2 = 3, a3 = 4, a4 = 5, a5 = 6, a6 = 7, a7 = 8;
    int b0 = 9, b1 = 10, b2 = 11, b3 = 12, b4 = 13, b5 = 14, b6 = 15, b7 = 16;
    int c0 = 17, c1 = 18, c2 = 19, c3 = 20, c4 = 21, c5 = 22, c6 = 23, c7 = 24;
    
    int *array = (int*)malloc(size * sizeof(int));
    if (!array) return -1;
    
    /* setjmp creates exceptional control flow */
    if (setjmp(jump_buffer) == 0) {
        /* Normal path with complex computations */
        for (i = 0; i < depth; i++) {
            helper_b1(array, size);
            
            for (j = 0; j < size; j++) {
                /* Heavy register usage in loop */
                a0 = array[j] + a1;
                a1 = a0 * a2;
                a2 = a1 - a3;
                a3 = a2 / (a4 ? a4 : 1);
                a4 = a3 | a5;
                a5 = a4 ^ a6;
                a6 = a5 & a7;
                a7 = a6 + b0;
                
                b0 = a7 * b1;
                b1 = b0 - b2;
                b2 = b1 + b3;
                b3 = b2 * b4;
                b4 = b3 / (b5 ? b5 : 1);
                b5 = b4 | b6;
                b6 = b5 ^ b7;
                b7 = b6 & c0;
                
                c0 = b7 + c1;
                c1 = c0 * c2;
                c2 = c1 - c3;
                c3 = c2 + c4;
                c4 = c3 * c5;
                c5 = c4 / (c6 ? c6 : 1);
                c6 = c5 | c7;
                c7 = c6 ^ a0;
                
                result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 +
                         b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 +
                         c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7;
                
                FORCE_USE(result);
            }
            
            helper_b2(array, size);
            
            /* Occasionally longjmp to create exceptional edge */
            if (i > depth / 2 && jump_counter++ < 3) {
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* longjmp target - different register usage pattern */
        for (i = 0; i < size; i++) {
            a0 = array[i] * 2;
            a1 = a0 + array[i];
            result += a0 - a1;
            FORCE_USE(result);
        }
    }
    
    free(array);
    return result;
}

/* ========== PATTERN C: Mixed pressure with vector operations ========== */
#ifdef __GNUC__
typedef int v4si VECTOR_TYPE;
#else
typedef int v4si[4];
#endif

NOINLINE
static v4si vector_op(v4si a, v4si b, v4si c) {
#ifdef __GNUC__
    return a * b + c;
#else
    v4si result;
    for (int i = 0; i < 4; i++) result[i] = a[i] * b[i] + c[i];
    return result;
#endif
}

int pattern_c_mixed_pressure(int iterations, int case_val) {
    volatile int result = 0;
    int i;
    
    /* Scalar variables */
    int s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5, s5 = 6, s6 = 7, s7 = 8;
    int s8 = 9, s9 = 10, s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16;
    
    /* Vector variables */
#ifdef __GNUC__
    v4si v0 = {1, 2, 3, 4};
    v4si v1 = {5, 6, 7, 8};
    v4si v2 = {9, 10, 11, 12};
    v4si v3 = {13, 14, 15, 16};
    v4si v4 = {17, 18, 19, 20};
    v4si v5 = {21, 22, 23, 24};
#else
    v4si v0 = {1, 2, 3, 4};
    v4si v1 = {5, 6, 7, 8};
    v4si v2 = {9, 10, 11, 12};
    v4si v3 = {13, 14, 15, 16};
    v4si v4 = {17, 18, 19, 20};
    v4si v5 = {21, 22, 23, 24};
#endif
    
    /* Large switch statement with 30+ cases */
    for (i = 0; i < iterations; i++) {
        switch ((case_val + i) % 35) {
            case 0:
                s0 = s1 + s2; s1 = s3 * s4; s2 = s5 - s6;
                v0 = vector_op(v0, v1, v2);
                result += s0 + s1 + s2;
                break;
            case 1:
                s3 = s4 / (s5 ? s5 : 1); s4 = s6 | s7; s5 = s8 ^ s9;
                v1 = vector_op(v1, v2, v3);
                result += s3 + s4 + s5;
                break;
            case 2:
                s6 = s7 & s8; s7 = s9 + s10; s8 = s11 * s12;
                v2 = vector_op(v2, v3, v4);
                result += s6 + s7 + s8;
                break;
            case 3:
                s9 = s10 - s11; s10 = s12 / (s13 ? s13 : 1); s11 = s14 | s15;
                v3 = vector_op(v3, v4, v5);
                result += s9 + s10 + s11;
                break;
            case 4:
                s12 = s13 ^ s14; s13 = s15 + s0; s14 = s1 * s2;
                v4 = vector_op(v4, v5, v0);
                result += s12 + s13 + s14;
                break;
            case 5:
                s15 = s0 & s1; s0 = s2 - s3; s1 = s4 / (s5 ? s5 : 1);
                v5 = vector_op(v5, v0, v1);
                result += s15 + s0 + s1;
                break;
            case 6:
                s2 = s3 | s4; s3 = s5 ^ s6; s4 = s7 + s8;
                v0 = vector_op(v0, v1, v2);
                result += s2 + s3 + s4;
                break;
            case 7:
                s5 = s6 * s7; s6 = s8 - s9; s7 = s10 / (s11 ? s11 : 1);
                v1 = vector_op(v1, v2, v3);
                result += s5 + s6 + s7;
                break;
            case 8:
                s8 = s9 | s10; s9 = s11 ^ s12; s10 = s13 + s14;
                v2 = vector_op(v2, v3, v4);
                result += s8 + s9 + s10;
                break;
            case 9:
                s11 = s12 & s13; s12 = s14 * s15; s13 = s0 - s1;
                v3 = vector_op(v3, v4, v5);
                result += s11 + s12 + s13;
                break;
            case 10:
                s14 = s15 / (s0 ? s0 : 1); s15 = s1 | s2; s0 = s3 ^ s4;
                v4 = vector_op(v4, v5, v0);
                result += s14 + s15 + s0;
                break;
            case 11:
                s1 = s2 + s3; s2 = s4 * s5; s3 = s6 - s7;
                v5 = vector_op(v5, v0, v1);
                result += s1 + s2 + s3;
                break;
            case 12:
                s4 = s5 / (s6 ? s6 : 1); s5 = s7 | s8; s6 = s9 ^ s10;
                v0 = vector_op(v0, v1, v2);
                result += s4 + s5 + s6;
                break;
            case 13:
                s7 = s8 & s9; s8 = s10 + s11; s9 = s12 * s13;
                v1 = vector_op(v1, v2, v3);
                result += s7 + s8 + s9;
                break;
            case 14:
                s10 = s11 - s12; s11 = s13 / (s14 ? s14 : 1); s12 = s15 | s0;
                v2 = vector_op(v2, v3, v4);
                result += s10 + s11 + s12;
                break;
            case 15:
                s13 = s14 ^ s15; s14 = s0 + s1; s15 = s2 * s3;
                v3 = vector_op(v3, v4, v5);
                result += s13 + s14 + s15;
                break;
            case 16:
                s0 = s1 & s2; s1 = s3 - s4; s2 = s5 / (s6 ? s6 : 1);
                v4 = vector_op(v4, v5, v0);
                result += s0 + s1 + s2;
                break;
            case 17:
                s3 = s4 | s5; s4 = s6 ^ s7; s5 = s8 + s9;
                v5 = vector_op(v5, v0, v1);
                result += s3 + s4 + s5;
                break;
            case 18:
                s6 = s7 * s8; s7 = s9 - s10; s8 = s11 / (s12 ? s12 : 1);
                v0 = vector_op(v0, v1, v2);
                result += s6 + s7 + s8;
                break;
            case 19:
                s9 = s10 | s11; s10 = s12 ^ s13; s11 = s14 + s15;
                v1 = vector_op(v1, v2, v3);
                result += s9 + s10 + s11;
                break;
            case 20:
                s12 = s13 & s14; s13 = s15 * s0; s14 = s1 - s2;
                v2 = vector_op(v2, v3, v4);
                result += s12 + s13 + s14;
                break;
            case 21:
                s15 = s0 / (s1 ? s1 : 1); s0 = s2 | s3; s1 = s4 ^ s5;
                v3 = vector_op(v3, v4, v5);
                result += s15 + s0 + s1;
                break;
            case 22:
                s2 = s3 + s4; s3 = s5 * s6; s4 = s7 - s8;
                v4 = vector_op(v4, v5, v0);
                result += s2 + s3 + s4;
                break;
            case 23:
                s5 = s6 / (s7 ? s7 : 1); s6 = s8 | s9; s7 = s10 ^ s11;
                v5 = vector_op(v5, v0, v1);
                result += s5 + s6 + s7;
                break;
            case 24:
                s8 = s9 & s10; s9 = s11 + s12; s10 = s13 * s14;
                v0 = vector_op(v0, v1, v2);
                result += s8 + s9 + s10;
                break;
            case 25:
                s11 = s12 - s13; s12 = s14 / (s15 ? s15 : 1); s13 = s0 | s1;
                v1 = vector_op(v1, v2, v3);
                result += s11 + s12 + s13;
                break;
            case 26:
                s14 = s15 ^ s0; s15 = s1 + s2; s0 = s3 * s4;
                v2 = vector_op(v2, v3, v4);
                result += s14 + s15 + s0;
                break;
            case 27:
                s1 = s2 & s3; s2 = s4 - s5; s3 = s6 / (s7 ? s7 : 1);
                v3 = vector_op(v3, v4, v5);
                result += s1 + s2 + s3;
                break;
            case 28:
                s4 = s5 | s6; s5 = s7 ^ s8; s6 = s9 + s10;
                v4 = vector_op(v4, v5, v0);
                result += s4 + s5 + s6;
                break;
            case 29:
                s7 = s8 * s9; s8 = s10 - s11; s9 = s12 / (s13 ? s13 : 1);
                v5 = vector_op(v5, v0, v1);
                result += s7 + s8 + s9;
                break;
            case 30:
                s10 = s11 | s12; s11 = s13 ^ s14; s12 = s15 + s0;
                v0 = vector_op(v0, v1, v2);
                result += s10 + s11 + s12;
                break;
            case 31:
                s13 = s14 & s15; s14 = s0 * s1; s15 = s2 - s3;
                v1 = vector_op(v1, v2, v3);
                result += s13 + s14 + s15;
                break;
            case 32:
                s0 = s1 / (s2 ? s2 : 1); s1 = s3 | s4; s2 = s5 ^ s6;
                v2 = vector_op(v2, v3, v4);
                result += s0 + s1 + s2;
                break;
            case 33:
                s3 = s4 + s5; s4 = s6 * s7; s5 = s8 - s9;
                v3 = vector_op(v3, v4, v5);
                result += s3 + s4 + s5;
                break;
            case 34:
                s6 = s7 / (s8 ? s8 : 1); s7 = s9 | s10; s8 = s11 ^ s12;
                v4 = vector_op(v4, v5, v0);
                result += s6 + s7 + s8;
                break;
        }
        
        /* Force all variables to appear live */
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
        
        /* Control flow within loop */
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
        if (i % 17 == 0) {
            case_val = (case_val * 3) % 35;
            continue;
        }
    }
    
    /* Extract results from vectors */
#ifdef __GNUC__
    int vsum = v0[0] + v0[1] + v0[2] + v0[3] +
               v1[0] + v1[1] + v1[2] + v1[3] +
               v2[0] + v2[1] + v2[2] + v2[3] +
               v3[0] + v3[1] + v3[2] + v3[3] +
               v4[0] + v4[1] + v4[2] + v4[3] +
               v5[0] + v5[1] + v5[2] + v5[3];
#else
    int vsum = v0[0] + v0[1] + v0[2] + v0[3] +
               v1[0] + v1[1] + v1[2] + v1[3] +
               v2[0] + v2[1] + v2[2] + v2[3] +
               v3[0] + v3[1] + v3[2] + v3[3] +
               v4[0] + v4[1] + v4[2] + v4[3] +
               v5[0] + v5[1] + v5[2] + v5[3];
#endif
    
    result += vsum;
    return result;
}

/* ========== PATTERN D: Artificial register conflicts ========== */
#ifdef __GNUC__
register int reg_x asm ("r10");
register int reg_y asm ("r11");
#else
int reg_x, reg_y;
#endif

NOINLINE
static int helper_d1(int a, int b) {
    return a * b + 1;
}

NOINLINE
static int helper_d2(int a, int b) {
    return a / (b ? b : 1) - 2;
}

NOINLINE
static int helper_d3(int a, int b) {
    return (a | b) ^ 3;
}

int pattern_d_register_conflict(int n) {
    volatile int result = 0;
    int i;
    
    /* Use explicit register variables in conflicting ways */
#ifdef __GNUC__
    int old_x = reg_x;
    int old_y = reg_y;
    
    reg_x = n;
    reg_y = n * 2;
#endif
    
    /* Many local variables that will conflict with register vars */
    int a0 = 1, a1 = 2, a2 = 3, a3 = 4, a4 = 5, a5 = 6, a6 = 7, a7 = 8;
    int b0 = 9, b1 = 10, b2 = 11, b3 = 12, b4 = 13, b5 = 14, b6 = 15, b7 = 16;
    int c0 = 17, c1 = 18, c2 = 19, c3 = 20, c4 = 21, c5 = 22, c6 = 23, c7 = 24;
    
    for (i = 0; i < n; i++) {
        /* Call helpers that may clobber registers */
        a0 = helper_d1(a0, i);
        a1 = helper_d2(a1, i + 1);
        a2 = helper_d3(a2, i + 2);
        
        /* Use register variables in computations */
#ifdef __GNUC__
        reg_x = reg_x + a0;
        reg_y = reg_y * a1;
        a3 = reg_x - reg_y;
#else
        a3 = a0 - a1;
#endif
        
        /* More computations creating live range splits */
        a4 = a0 + a1;
        a5 = a2 * a3;
        a6 = a4 / (a5 ? a5 : 1);
        a7 = a5 | a6;
        
        b0 = helper_d1(b0, a0);
        b1 = helper_d2(b1, a1);
        b2 = helper_d3(b2, a2);
        
        b3 = b0 + b1;
        b4 = b2 * b3;
        b5 = b4 - b0;
        b6 = b5 / (b1 ? b1 : 1);
        b7 = b6 | b2;
        
        c0 = helper_d1(c0, b0);
        c1 = helper_d2(c1, b1);
        c2 = helper_d3(c2, b2);
        
        c3 = c0 + c1 + c2;
        c4 = c3 * a0;
        c5 = c4 / (a1 ? a1 : 1);
        c6 = c5 | a2;
        c7 = c6 ^ a3;
        
        /* Force all variables live */
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
        FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
        FORCE_USE(b0); FORCE_USE(b1); FORCE_USE(b2); FORCE_USE(b3);
        FORCE_USE(b4); FORCE_USE(b5); FORCE_USE(b6); FORCE_USE(b7);
        FORCE_USE(c0); FORCE_USE(c1); FORCE_USE(c2); FORCE_USE(c3);
        FORCE_USE(c4); FORCE_USE(c5); FORCE_USE(c6); FORCE_USE(c7);
        
        result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 +
                 b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 +
                 c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7;
        
        /* Complex loop control */
        if (i % 11 == 0) {
#ifdef __GNUC__
            reg_x = result % 100;
#endif
            continue;
        }
        if (i % 13 == 0) {
#ifdef __GNUC__
            reg_y = result / 100;
#endif
            break;
        }
    }
    
#ifdef __GNUC__
    result += reg_x + reg_y;
    reg_x = old_x;
    reg_y = old_y;
#endif
    
    return result;
}

/* ========== MAIN FUNCTION with PGO support ========== */
int main(int argc, char **argv) {
    volatile int total = 0;
    int i, iterations;
    
    if (argc > 1) {
        verbose = 1;
    }
    
    /* Determine iterations based on CPU capabilities */
    iterations = 100;
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        iterations = 150;  /* More iterations for AVX2 targets */
        if (verbose) printf("AVX2 supported, using %d iterations\n", iterations);
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        if (verbose && i % 20 == 0) {
            printf("Iteration %d\n", i);
        }
        
        /* Pattern A - targets ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(i % 50 + 10, i);
        
        /* Pattern B - targets NEW_EXIT/NEW_ENTRY nodes */
        total += pattern_b_new_nodes(i % 5 + 1, i % 20 + 10);
        
        /* Pattern C - mixed pressure with vectors */
        total += pattern_c_mixed_pressure(i % 30 + 5, i % 35);
        
        /* Pattern D - register conflicts */
        total += pattern_d_register_conflict(i % 40 + 8);
        
        /* Prevent optimization */
        FORCE_USE(total);
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return total != 0 ? 0 : 1;
}
