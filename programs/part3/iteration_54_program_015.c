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

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex irreducible CFG with goto to force ENTRY_BLOCK+1 and 2*EXIT_BLOCK */
    volatile int result = seed;
    int i, j, k;
    
    /* Many local variables to create register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Deeply nested if-else chain */
    if (iterations > 0) {
        if (iterations > 10) {
            if (iterations > 20) {
                /* Irreducible region using goto */
                if (seed % 3 == 0) {
                    goto irreducible_label;
                }
            } else {
                for (i = 0; i < iterations; i++) {
                    r0 += r1 * r2;
                    r3 -= r4 ^ r5;
                    if (i % 7 == 0) continue;
                    r6 = r7 << (r8 & 3);
                }
            }
        } else {
            while (iterations-- > 0) {
                r9 = r10 | r11;
                r12 = r13 & r14;
                if (r15 > 1000) break;
                r15 += r0;
            }
        }
    }
    
    /* Multiple exit points */
    if (result > 1000000) {
        return result % 1000;
    }
    
irreducible_label:
    /* This creates an irreducible region */
    for (j = 0; j < 5; j++) {
        if (j == 2) {
            goto back_to_main;
        }
        r0 += r1;
    }
    
    if (seed % 2 == 0) {
        return r0 + r1;
    }
    
back_to_main:
    /* Complex loop with continue to different points */
    for (k = 0; k < iterations; k++) {
        if (k % 2 == 0) {
            r2 = r3 * r4;
            continue;
        } else if (k % 3 == 0) {
            r5 = r6 / (r7 + 1);
            goto irreducible_label;
        }
        r8 = r9 ^ r10;
    }
    
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    return result + r0 + r1 + r2;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int setjmp_counter = 0;

NOINLINE
static void helper_b1(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 3 + 7;
    }
    FORCE_USE(arr);
}

NOINLINE
static void helper_b2(int *arr, int n) {
    if (n > 10 && setjmp_counter < 3) {
        setjmp_counter++;
        longjmp(jump_buffer, 1);
    }
    FORCE_USE(arr);
}

int pattern_b_new_nodes(int size, int threshold) {
    /* Many scalar variables to pressure registers */
    int a0 = size, a1 = size + 1, a2 = size + 2, a3 = size + 3;
    int a4 = size + 4, a5 = size + 5, a6 = size + 6, a7 = size + 7;
    int a8 = size + 8, a9 = size + 9, a10 = size + 10, a11 = size + 11;
    int a12 = size + 12, a13 = size + 13, a14 = size + 14, a15 = size + 15;
    
    int *buffer = malloc(size * sizeof(int));
    if (!buffer) return -1;
    
    for (int i = 0; i < size; i++) {
        buffer[i] = i;
    }
    
    /* setjmp creates exceptional control flow edges */
    if (setjmp(jump_buffer) == 0) {
        /* Normal path with complex arithmetic */
        for (int i = 0; i < size; i++) {
            a0 = a1 + a2 * a3;
            a4 = a5 ^ a6 | a7;
            a8 = a9 - a10 / (a11 + 1);
            a12 = a13 << (a14 & 3);
            a15 = a0 + a4 + a8 + a12;
            
            buffer[i] += a15;
            
            if (buffer[i] > threshold && i % 7 == 0) {
                helper_b2(buffer, i);
            }
        }
        helper_b1(buffer, size);
    } else {
        /* longjmp target - different register usage pattern */
        for (int i = 0; i < size; i += 2) {
            a0 = a2 + a4;
            a1 = a3 * a5;
            a6 = a7 ^ a8;
            buffer[i] = a0 + a1 + a6;
        }
    }
    
    int result = 0;
    for (int i = 0; i < size; i++) {
        result += buffer[i];
    }
    
    /* Force all variables live */
    FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
    FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
    FORCE_USE(a8); FORCE_USE(a9); FORCE_USE(a10); FORCE_USE(a11);
    FORCE_USE(a12); FORCE_USE(a13); FORCE_USE(a14); FORCE_USE(a15);
    
    free(buffer);
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si VECTOR_TYPE;
#else
typedef int v4si[4];
#endif

NOINLINE
int pattern_c_mixed_pressure(int base, int iterations) {
    /* Large switch statement with 30+ cases */
    int result = base;
    int r0 = base, r1 = base + 1, r2 = base + 2, r3 = base + 3;
    int r4 = base + 4, r5 = base + 5, r6 = base + 6, r7 = base + 7;
    int r8 = base + 8, r9 = base + 9, r10 = base + 10, r11 = base + 11;
    int r12 = base + 12, r13 = base + 13, r14 = base + 14, r15 = base + 15;
    
    /* Vector variables for additional pressure */
    v4si v0, v1, v2, v3;
#ifdef __GNUC__
    v0 = (v4si){r0, r1, r2, r3};
    v1 = (v4si){r4, r5, r6, r7};
    v2 = (v4si){r8, r9, r10, r11};
    v3 = (v4si){r12, r13, r14, r15};
#endif
    
    /* Switch inside loop with continue/break to different cases */
    for (int i = 0; i < iterations; i++) {
        int selector = (result + i) % 35;  /* 35 cases */
        
        switch (selector) {
            case 0:
                r0 = r1 + r2 * r3;
                v0[0] = r0;
                if (i % 3 == 0) continue;
                break;
            case 1:
                r4 = r5 - r6 / (r7 + 1);
                v1[1] = r4;
                if (i % 5 == 0) break;
                /* fall through */
            case 2:
                r8 = r9 ^ r10 | r11;
                v2[2] = r8;
                break;
            case 3:
                r12 = r13 << (r14 & 3);
                v3[3] = r12;
                if (r15 > 1000) continue;
                break;
            case 4:
                r1 = r2 * r3 + r4;
                v0[0] += r1;
                break;
            case 5:
                r5 = r6 ^ r7 & r8;
                v1[1] ^= r5;
                break;
            case 6:
                r9 = r10 - r11 * r12;
                v2[2] -= r9;
                if (i % 7 == 0) continue;
                break;
            case 7:
                r13 = r14 | r15 << 2;
                v3[3] |= r13;
                break;
            case 8:
                r0 = r1 + r2 + r3 + r4;
                v0[0] = r0;
                break;
            case 9:
                r5 = r6 * r7 - r8;
                v1[1] *= r5;
                break;
            case 10:
                r9 = r10 / (r11 + 1) + r12;
                v2[2] += r9;
                break;
            case 11:
                r13 = r14 & r15 ^ r0;
                v3[3] ^= r13;
                if (i % 11 == 0) break;
                /* fall through */
            case 12:
                r1 = r2 << 1;
                v0[0] <<= 1;
                break;
            case 13:
                r3 = r4 >> 2;
                v0[1] >>= 2;
                break;
            case 14:
                r5 = r6 + r7 * 3;
                v1[0] = r5;
                break;
            case 15:
                r8 = r9 - r10 / 2;
                v1[1] = r8;
                break;
            case 16:
                r11 = r12 ^ r13;
                v2[0] ^= r11;
                break;
            case 17:
                r14 = r15 | r0;
                v2[1] |= r14;
                break;
            case 18:
                r1 = r2 & r3;
                v0[2] &= r1;
                break;
            case 19:
                r4 = r5 + r6 + r7;
                v0[3] += r4;
                break;
            case 20:
                r8 = r9 * r10;
                v1[2] *= r8;
                if (i % 13 == 0) continue;
                break;
            case 21:
                r11 = r12 - r13;
                v1[3] -= r11;
                break;
            case 22:
                r14 = r15 << 3;
                v2[2] <<= 3;
                break;
            case 23:
                r0 = r1 >> 1;
                v2[3] >>= 1;
                break;
            case 24:
                r2 = r3 ^ r4 ^ r5;
                v3[0] ^= r2;
                break;
            case 25:
                r6 = r7 | r8 | r9;
                v3[1] |= r6;
                break;
            case 26:
                r10 = r11 & r12 & r13;
                v3[2] &= r10;
                break;
            case 27:
                r14 = r15 + r0 + r1;
                v3[3] += r14;
                break;
            case 28:
                r2 = r3 * r4 * r5;
                v0[0] *= r2;
                if (i % 17 == 0) break;
                /* fall through */
            case 29:
                r6 = r7 - r8 - r9;
                v0[1] -= r6;
                break;
            case 30:
                r10 = r11 ^ r12 ^ r13;
                v1[0] ^= r10;
                break;
            case 31:
                r14 = r15 | (r0 & r1);
                v1[1] |= r14;
                break;
            case 32:
                r2 = (r3 + r4) * r5;
                v2[0] = r2;
                break;
            case 33:
                r6 = r7 / (r8 + 1) + r9;
                v2[1] += r6;
                break;
            case 34:
                r10 = r11 << (r12 % 4);
                v3[0] <<= (r12 % 4);
                if (result > 1000000) break;
                continue;  /* Back to loop start */
        }
        
        /* Vector operations mixed with scalar */
#ifdef __GNUC__
        v0 = v0 + v1;
        v2 = v2 * v3;
        v1 = v1 - v2;
        v3 = v3 ^ v0;
#endif
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                  r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    }
    
    /* Force all variables live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
#ifdef __GNUC__
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
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

NOINLINE static void dummy_helper1(int x) {
    FORCE_USE(x);
}

NOINLINE static void dummy_helper2(int x, int y) {
    FORCE_USE(x); FORCE_USE(y);
}

NOINLINE static void dummy_helper3(int x, int y, int z) {
    FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
}

int pattern_d_artificial_conflict(int param) {
    /* Artificial register conflicts */
    int local1 = param * 2;
    int local2 = param + 7;
    int local3 = param ^ 0xABCD;
    int local4 = param << 3;
    int local5 = param >> 2;
    int local6 = param | 0x1234;
    int local7 = param & 0xFF;
    int local8 = param % 17;
    int local9 = param * param;
    int local10 = param + param;
    
#ifdef __GNUC__
    /* Use explicit register variables conflicting with allocator */
    reg_var1 = local1;
    reg_var2 = local2;
    
    /* Force spills by using same register for different purposes */
    asm volatile("" : "+r" (reg_var1), "+r" (reg_var2));
#endif
    
    /* Call dummy functions to split live ranges */
    dummy_helper1(local1);
    local3 = local1 + local2;
    
    dummy_helper2(local3, local4);
    local5 = local3 * local4;
    
#ifdef __GNUC__
    reg_var1 = local5;
    asm volatile("" : "+r" (reg_var1));
#endif
    
    dummy_helper3(local5, local6, local7);
    local8 = local5 ^ local6 ^ local7;
    
    /* Complex arithmetic to create many temporary values */
    for (int i = 0; i < 20; i++) {
        local9 = local9 * 3 + local8;
        local10 = local10 ^ (local9 >> i);
        
        if (i % 3 == 0) {
#ifdef __GNUC__
            reg_var2 = local9;
            asm volatile("" : "+r" (reg_var2));
#endif
            dummy_helper1(local9);
        } else if (i % 5 == 0) {
            dummy_helper2(local10, i);
        }
        
        /* Force register pressure */
        int temp1 = local1 + local2;
        int temp2 = local3 * local4;
        int temp3 = local5 ^ local6;
        int temp4 = local7 & local8;
        int temp5 = local9 | local10;
        
        FORCE_USE(temp1); FORCE_USE(temp2); FORCE_USE(temp3);
        FORCE_USE(temp4); FORCE_USE(temp5);
    }
    
#ifdef __GNUC__
    asm volatile("" : : "r" (reg_var1), "r" (reg_var2));
#endif
    
    return local1 + local2 + local3 + local4 + local5 +
           local6 + local7 + local8 + local9 + local10;
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    /* Use profile-guided optimization pattern */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* Pattern A - targeting ENTRY/EXIT blocks */
        int res_a = pattern_a_entry_exit(iteration % 50, iteration * 3);
        if (verbose) printf("Iteration %d, Pattern A: %d\n", iteration, res_a);
        total_result ^= res_a;
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY */
        if (iteration % 10 == 0) {
            setjmp_counter = 0;
            int res_b = pattern_b_new_nodes(50 + (iteration % 30), 1000);
            if (verbose) printf("Iteration %d, Pattern B: %d\n", iteration, res_b);
            total_result += res_b;
        }
        
        /* Pattern C - mixed pressure */
        int res_c = pattern_c_mixed_pressure(iteration, 10 + (iteration % 20));
        if (verbose) printf("Iteration %d, Pattern C: %d\n", iteration, res_c);
        total_result *= (res_c + 1);
        
        /* Pattern D - artificial conflict */
        if (iteration % 7 == 0) {
            int res_d = pattern_d_artificial_conflict(iteration * 5);
            if (verbose) printf("Iteration %d, Pattern D: %d\n", iteration, res_d);
            total_result -= res_d;
        }
        
        /* Use __builtin_cpu_supports to engage target-specific allocation */
        if (iteration == 50) {
#ifdef __GNUC__
            if (__builtin_cpu_supports("avx2")) {
                if (verbose) printf("AVX2 supported, increasing pressure\n");
                /* Do some extra vector work */
                for (int i = 0; i < 1000; i++) {
                    total_result += i * (iteration % 256);
                }
            }
#endif
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    /* Prevent optimization of the entire program */
    FORCE_USE(total_result);
    
    return total_result != 0;
}
