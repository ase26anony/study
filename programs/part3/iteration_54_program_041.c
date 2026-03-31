/* test_mcf.c - Complex CFG generator to stress GCC's MCF pass */
/* Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-bbro -c test_mcf.c */

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

/* Pattern A: Deep nested if-else with irreducible region for ENTRY/EXIT blocks */
HOT NOINLINE int pattern_a(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    
    /* Create complex entry block */
    if (seed < 0) {
        goto irreducible_start;
    }
    
    /* Deep nesting to create many basic blocks */
    for (i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            for (j = 0; j < 5; j++) {
                if (j % 2 == 0) {
                    result += i * j;
                    if (result > 1000) {
                        result -= 500;
                        continue; /* Complex back-edge */
                    }
                } else {
                    result -= i + j;
                    if (result < 0) {
                        result = -result;
                        break; /* Early exit from inner loop */
                    }
                }
                
                /* More nesting */
                for (k = 0; k < 3; k++) {
                    if ((i + j + k) % 4 == 0) {
                        result ^= (i << 3) | (j << 2) | k;
                    } else if ((i + j + k) % 4 == 1) {
                        result |= 0x7F;
                    } else if ((i + j + k) % 4 == 2) {
                        result &= 0xFF;
                    } else {
                        result = ~result;
                    }
                }
            }
        } else if (i % 3 == 1) {
            /* Another path */
            result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        } else {
            /* Yet another path */
            result = result ^ (result >> 16);
            result = result * 0x45D9F3B;
            result = result ^ (result >> 16);
        }
    }
    
irreducible_start:
    /* Irreducible region created by goto */
    if (result & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    result += 111;
    if (result > 0) {
        goto label3;
    } else {
        goto exit_point;
    }
    
label2:
    result -= 222;
    if (result < 0) {
        goto label1;
    } else {
        goto label3;
    }
    
label3:
    result *= 333;
    goto exit_point;
    
exit_point:
    FORCE_USE(result);
    return result;
}

/* Pattern B: setjmp/longjmp for exceptional edges requiring NEW_EXIT/NEW_ENTRY */
static jmp_buf env;
static int jump_counter = 0;

NOINLINE int pattern_b(int iterations, int seed) {
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    int result = seed;
    
    r0 = seed; r1 = seed + 1; r2 = seed + 2; r3 = seed + 3;
    r4 = seed + 4; r5 = seed + 5; r6 = seed + 6; r7 = seed + 7;
    r8 = seed + 8; r9 = seed + 9; r10 = seed + 10; r11 = seed + 11;
    r12 = seed + 12; r13 = seed + 13; r14 = seed + 14; r15 = seed + 15;
    
    if (setjmp(env) == 0) {
        /* First time through */
        for (int i = 0; i < iterations; i++) {
            /* Complex arithmetic on all registers */
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
            
            /* Occasionally longjmp to create exceptional edge */
            if (i > iterations / 2 && jump_counter < 3) {
                jump_counter++;
                longjmp(env, 1);
            }
            
            /* Mix results */
            result += r0 ^ r1 ^ r2 ^ r3 ^ r4 ^ r5 ^ r6 ^ r7 ^ 
                     r8 ^ r9 ^ r10 ^ r11 ^ r12 ^ r13 ^ r14 ^ r15;
        }
    } else {
        /* After longjmp */
        result = (r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                 r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15) & 0xFF;
    }
    
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    FORCE_USE(result);
    
    return result;
}

/* Pattern C: Large switch with vector operations for mixed pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE int pattern_c(int iterations, int seed) {
    int result = seed;
    
#ifdef __GNUC__
    v4si vec0 = {seed, seed + 1, seed + 2, seed + 3};
    v4si vec1 = {seed + 4, seed + 5, seed + 6, seed + 7};
    v4si vec2 = {seed + 8, seed + 9, seed + 10, seed + 11};
    v4si vec3 = {seed + 12, seed + 13, seed + 14, seed + 15};
#endif
    
    for (int i = 0; i < iterations; i++) {
        int switch_val = (result + i) % 35; /* 35 cases to exceed typical jump table thresholds */
        
        switch (switch_val) {
            case 0: {
                int t0 = result * 3;
                int t1 = t0 + 1;
                int t2 = t1 * 5;
                int t3 = t2 - 7;
                result = t3 ^ 0x1234;
#ifdef __GNUC__
                vec0 = vec0 + vec1;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                break;
            }
            case 1: {
                int t0 = result / 2;
                int t1 = t0 | 0xFF00;
                int t2 = t1 << 3;
                int t3 = t2 >> 1;
                result = t3 + 5678;
#ifdef __GNUC__
                vec1 = vec1 * vec2;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                break;
            }
            case 2: {
                int t0 = result ^ 0xABCD;
                int t1 = t0 & 0x7F7F;
                int t2 = t1 + 12345;
                int t3 = t2 % 1000;
                result = t3 * 11;
#ifdef __GNUC__
                vec2 = vec2 - vec3;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                break;
            }
            case 3: {
                int t0 = result | 0xF0F0;
                int t1 = t0 ^ result;
                int t2 = t1 << 4;
                int t3 = t2 >> 2;
                result = t3 & 0x3FFF;
#ifdef __GNUC__
                vec3 = vec3 & vec0;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                break;
            }
            case 4: {
                int t0 = result + 1111;
                int t1 = t0 - 2222;
                int t2 = t1 * 3;
                int t3 = t2 / 4;
                result = t3 | 0x8000;
#ifdef __GNUC__
                vec0 = vec0 | vec2;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                break;
            }
            /* 30 more unique cases following the same pattern */
            case 5: { int t0 = result * 7; int t1 = t0 + 9; result = t1 ^ 0x1111; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 6: { int t0 = result / 3; int t1 = t0 | 0x2222; result = t1 + 33; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 7: { int t0 = result ^ 0x3333; int t1 = t0 & 0x4444; result = t1 * 5; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 8: { int t0 = result | 0x5555; int t1 = t0 << 2; result = t1 >> 1; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 9: { int t0 = result + 6666; int t1 = t0 - 7777; result = t1 ^ 0x8888; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 10: { int t0 = result * 9; int t1 = t0 / 10; result = t1 | 0x9999; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 11: { int t0 = result ^ 0xAAAA; int t1 = t0 + 0xBBBB; result = t1 & 0xCCCC; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 12: { int t0 = result | 0xDDDD; int t1 = t0 * 13; result = t1 >> 3; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 13: { int t0 = result + 14141; int t1 = t0 ^ 15151; result = t1 << 4; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 14: { int t0 = result & 0xEEEE; int t1 = t0 | 0xFFFF; result = t1 / 7; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 15: { int t0 = result * 16; int t1 = t0 - 17171; result = t1 ^ 0x1818; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 16: { int t0 = result | 0x1919; int t1 = t0 + 20202; result = t1 & 0x2121; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 17: { int t0 = result ^ 0x2222; int t1 = t0 * 23; result = t1 >> 5; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 18: { int t0 = result + 24242; int t1 = t0 / 25; result = t1 | 0x2626; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 19: { int t0 = result & 0x2727; int t1 = t0 ^ 28282; result = t1 + 29292; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 20: { int t0 = result * 30; int t1 = t0 - 31313; result = t1 << 6; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 21: { int t0 = result | 0x3232; int t1 = t0 & 0x3333; result = t1 / 34; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 22: { int t0 = result ^ 0x3434; int t1 = t0 + 35353; result = t1 * 36; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 23: { int t0 = result + 37373; int t1 = t0 | 0x3838; result = t1 >> 7; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 24: { int t0 = result & 0x3939; int t1 = t0 ^ 40404; result = t1 - 41414; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 25: { int t0 = result * 42; int t1 = t0 / 43; result = t1 | 0x4444; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 26: { int t0 = result ^ 0x4545; int t1 = t0 + 46464; result = t1 & 0x4747; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 27: { int t0 = result | 0x4848; int t1 = t0 * 49; result = t1 >> 8; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 28: { int t0 = result + 50505; int t1 = t0 ^ 51515; result = t1 << 9; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 29: { int t0 = result & 0x5252; int t1 = t0 | 0x5353; result = t1 / 54; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 30: { int t0 = result * 55; int t1 = t0 - 56565; result = t1 ^ 0x5757; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 31: { int t0 = result | 0x5858; int t1 = t0 + 59595; result = t1 & 0x6060; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 32: { int t0 = result ^ 0x6161; int t1 = t0 * 62; result = t1 >> 10; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 33: { int t0 = result + 63636; int t1 = t0 / 64; result = t1 | 0x6565; FORCE_USE(t0); FORCE_USE(t1); break; }
            case 34: { int t0 = result & 0x6666; int t1 = t0 ^ 67676; result = t1 + 68686; FORCE_USE(t0); FORCE_USE(t1); break; }
            
            default:
                result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
                break;
        }
        
        /* Loop control with complex edges */
        if (result % 7 == 0) {
            continue;
        } else if (result % 13 == 0) {
            result += 1000;
            break;
        }
        
#ifdef __GNUC__
        /* Vector operations mixing with scalar */
        vec0 = vec0 + vec1;
        vec1 = vec1 * vec2;
        vec2 = vec2 - vec3;
        vec3 = vec3 & vec0;
        
        /* Extract scalar from vector */
        int vec_sum = vec0[0] + vec0[1] + vec0[2] + vec0[3] +
                     vec1[0] + vec1[1] + vec1[2] + vec1[3];
        result ^= vec_sum;
#endif
    }
    
#ifdef __GNUC__
    FORCE_USE(vec0); FORCE_USE(vec1); FORCE_USE(vec2); FORCE_USE(vec3);
#endif
    FORCE_USE(result);
    
    return result;
}

/* Pattern D: Explicit register variables for artificial conflicts */
NOINLINE int helper1(int x) { return x * 3 + 1; }
NOINLINE int helper2(int x) { return x / 2 - 1; }
NOINLINE int helper3(int x) { return x ^ 0x1234; }
NOINLINE int helper4(int x) { return x | 0x5678; }

NOINLINE int pattern_d(int iterations, int seed) {
    /* Try to force register conflicts */
#ifdef __GNUC__
    register int r10_var asm ("r10") = seed;
    register int r11_var asm ("r11") = seed + 1;
    register int r12_var asm ("r12") = seed + 2;
    register int r13_var asm ("r13") = seed + 3;
#else
    int r10_var = seed;
    int r11_var = seed + 1;
    int r12_var = seed + 2;
    int r13_var = seed + 3;
#endif
    
    int result = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Force use of register variables in complex expressions */
        r10_var = helper1(r10_var) + helper2(r11_var);
        r11_var = helper3(r12_var) ^ helper4(r13_var);
        r12_var = helper1(r13_var) - helper2(r10_var);
        r13_var = helper3(r11_var) | helper4(r12_var);
        
        /* Create artificial live range splits */
        if (i % 5 == 0) {
            int temp = r10_var;
            r10_var = r11_var;
            r11_var = r12_var;
            r12_var = r13_var;
            r13_var = temp;
            result += helper1(temp);
        } else if (i % 5 == 1) {
            result += helper2(r10_var + r11_var);
        } else if (i % 5 == 2) {
            result += helper3(r12_var * r13_var);
        } else if (i % 5 == 3) {
            result += helper4(r10_var ^ r11_var ^ r12_var ^ r13_var);
        } else {
            result = helper1(result) + helper2(result) + 
                    helper3(result) + helper4(result);
        }
        
        /* More register pressure */
        int local1 = r10_var * 3;
        int local2 = r11_var / 2;
        int local3 = r12_var + 7;
        int local4 = r13_var - 9;
        int local5 = local1 ^ local2;
        int local6 = local3 | local4;
        int local7 = local5 & local6;
        int local8 = local7 << 2;
        int local9 = local8 >> 1;
        int local10 = local9 % 100;
        
        result ^= local10;
        
        FORCE_USE(local1); FORCE_USE(local2); FORCE_USE(local3); FORCE_USE(local4);
        FORCE_USE(local5); FORCE_USE(local6); FORCE_USE(local7); FORCE_USE(local8);
        FORCE_USE(local9); FORCE_USE(local10);
    }
    
    result += r10_var + r11_var + r12_var + r13_var;
    
    FORCE_USE(r10_var); FORCE_USE(r11_var); FORCE_USE(r12_var); FORCE_USE(r13_var);
    FORCE_USE(result);
    
    return result;
}

/* Main driver with PGO support */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (argc > 2 && argv[2][0] == 'v') {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        int seed = i * 12345;
        
        if (verbose && i % 100 == 0) {
            printf("Iteration %d, seed = %d\n", i, seed);
        }
        
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a(iterations / 10, seed);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        total += pattern_b(iterations / 20, seed + 1);
        
        /* Pattern C - Mixed pressure with vectors */
        total += pattern_c(iterations / 5, seed + 2);
        
        /* Pattern D - Register conflicts */
        total += pattern_d(iterations / 8, seed + 3);
        
        /* Mix patterns together */
        if (i % 3 == 0) {
            total ^= pattern_a(iterations / 30, total);
        } else if (i % 3 == 1) {
            total |= pattern_b(iterations / 40, total);
        } else {
            total &= pattern_c(iterations / 50, total);
        }
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    /* Ensure result is used */
    FORCE_USE(total);
    
    return total & 0xFF;
}
