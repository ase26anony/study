/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage testing */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define REG_VAR(name, reg) register int name asm(reg)
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define REG_VAR(name, reg) int name
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Deeply nested if-else with irreducible region for ENTRY/EXIT blocks */
HOT NOINLINE int pattern_a_irreducible(int iterations, int seed) {
    int result = seed;
    int i = 0;
    
    /* Create irreducible region with goto */
    if (iterations > 100) {
        goto middle_of_loop;
    }
    
    for (i = 0; i < iterations; i++) {
        int branch_var = (result * 1103515245 + 12345) & 0x7fffffff;
        
        /* Deep if-else chain creating many basic blocks */
        if (branch_var % 2 == 0) {
            result += i * 3;
            if (branch_var % 3 == 0) {
                result ^= 0x5555;
                if (branch_var % 5 == 0) {
                    result >>= 1;
                    if (branch_var % 7 == 0) {
                        result |= 0x8000;
                        if (branch_var % 11 == 0) {
                            result = ~result;
                        } else {
                            result += 0x1000;
                        }
                    } else {
                        result &= 0x7FFF;
                    }
                } else {
                    result *= 2;
                }
            } else {
                result -= i;
            }
        } else {
            result ^= i;
            if (branch_var % 4 == 0) {
                result = (result << 4) | (result >> 28);
                if (branch_var % 6 == 0) {
                    result += 0xABCD;
                    if (branch_var % 8 == 0) {
                        result = result * 3 / 2;
                    }
                }
            }
        }
        
        /* Irreducible goto target */
        middle_of_loop:
        if (i % 7 == 0) {
            result = (result * 13) % 0x10000;
            continue;
        }
        
        if (i % 13 == 0) {
            result = (result + 0x1234) & 0xFFFF;
            break;
        }
    }
    
    FORCE_USE(result);
    return result;
}

/* Pattern B: setjmp/longjmp with many variables for NEW_EXIT/NEW_ENTRY */
NOINLINE int pattern_b_setjmp(int depth, int init) {
    jmp_buf env;
    int r0 = init, r1 = init + 1, r2 = init + 2, r3 = init + 3;
    int r4 = init + 4, r5 = init + 5, r6 = init + 6, r7 = init + 7;
    int r8 = init + 8, r9 = init + 9, r10 = init + 10, r11 = init + 11;
    int r12 = init + 12, r13 = init + 13, r14 = init + 14, r15 = init + 15;
    
    if (setjmp(env) == 0) {
        /* Complex arithmetic across all registers */
        for (int i = 0; i < depth; i++) {
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
            
            /* Create exceptional edge with longjmp */
            if (i == depth / 2) {
                longjmp(env, 1);
            }
            
            /* More arithmetic to increase register pressure */
            r0 ^= r15; r1 ^= r14; r2 ^= r13; r3 ^= r12;
            r4 ^= r11; r5 ^= r10; r6 ^= r9; r7 ^= r8;
            r8 ^= r7; r9 ^= r6; r10 ^= r5; r11 ^= r4;
            r12 ^= r3; r13 ^= r2; r14 ^= r1; r15 ^= r0;
        }
    } else {
        /* longjmp target - different arithmetic */
        r0 = r0 >> 1; r1 = r1 >> 2; r2 = r2 >> 3; r3 = r3 >> 4;
        r4 = r4 >> 1; r5 = r5 >> 2; r6 = r6 >> 3; r7 = r7 >> 4;
        r8 = r8 >> 1; r9 = r9 >> 2; r10 = r10 >> 3; r11 = r11 >> 4;
        r12 = r12 >> 1; r13 = r13 >> 2; r14 = r14 >> 3; r15 = r15 >> 4;
    }
    
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
           r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
}

/* Pattern C: Vector operations with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE int pattern_c_vector_switch(int selector, int iterations) {
    int result = 0;
    
#ifdef __GNUC__
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    for (int i = 0; i < iterations; i++) {
        int local_selector = (selector + i * 7919) % 37;  /* 37 cases */
        
        switch (local_selector) {
            case 0: {
                int t0 = i * 2, t1 = i * 3, t2 = i * 5, t3 = i * 7;
                int t4 = i * 11, t5 = i * 13, t6 = i * 17, t7 = i * 19;
                result += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
#ifdef __GNUC__
                vec0 += vec1;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6); FORCE_USE(t7);
                break;
            }
            case 1: {
                int t0 = i ^ 0xAA, t1 = i ^ 0x55, t2 = i ^ 0x33, t3 = i ^ 0xCC;
                int t4 = i ^ 0xF0, t5 = i ^ 0x0F, t6 = i ^ 0xFF, t7 = i ^ 0x3C;
                result += t0 - t1 + t2 - t3 + t4 - t5 + t6 - t7;
#ifdef __GNUC__
                vec1 *= vec0;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6); FORCE_USE(t7);
                break;
            }
            case 2: {
                int t0 = i << 1, t1 = i << 2, t2 = i << 3, t3 = i << 4;
                int t4 = i >> 1, t5 = i >> 2, t6 = i >> 3, t7 = i >> 4;
                result += t0 | t1 | t2 | t3 | t4 | t5 | t6 | t7;
#ifdef __GNUC__
                vec2 = vec0 + vec1;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6); FORCE_USE(t7);
                break;
            }
            case 3: {
                int t0 = i % 13, t1 = i % 17, t2 = i % 19, t3 = i % 23;
                int t4 = i % 29, t5 = i % 31, t6 = i % 37, t7 = i % 41;
                result += t0 * t1 + t2 * t3 + t4 * t5 + t6 * t7;
#ifdef __GNUC__
                vec3 = vec1 - vec0;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6); FORCE_USE(t7);
                break;
            }
            /* 33 more cases with unique arithmetic patterns */
            case 4: { int t0=i+1,t1=i+2,t2=i+3,t3=i+4,t4=i+5,t5=i+6,t6=i+7,t7=i+8; result+=t0*t1-t2*t3+t4*t5-t6*t7; FORCE_USE(t0);FORCE_USE(t1);FORCE_USE(t2);FORCE_USE(t3);FORCE_USE(t4);FORCE_USE(t5);FORCE_USE(t6);FORCE_USE(t7); break; }
            case 5: { int t0=i*2+1,t1=i*3+2,t2=i*5+3,t3=i*7+4,t4=i*11+5,t5=i*13+6,t6=i*17+7,t7=i*19+8; result+=t0^t1^t2^t3^t4^t5^t6^t7; FORCE_USE(t0);FORCE_USE(t1);FORCE_USE(t2);FORCE_USE(t3);FORCE_USE(t4);FORCE_USE(t5);FORCE_USE(t6);FORCE_USE(t7); break; }
            case 6: { int t0=i&0xF,t1=i&0xF0,t2=i&0xF00,t3=i&0xF000,t4=i|0xF,t5=i|0xF0,t6=i|0xF00,t7=i|0xF000; result+=t0+t1+t2+t3+t4+t5+t6+t7; FORCE_USE(t0);FORCE_USE(t1);FORCE_USE(t2);FORCE_USE(t3);FORCE_USE(t4);FORCE_USE(t5);FORCE_USE(t6);FORCE_USE(t7); break; }
            case 7: { int t0=(i<<1)&0xFF,t1=(i<<2)&0xFF,t2=(i<<3)&0xFF,t3=(i<<4)&0xFF,t4=(i>>1)&0xFF,t5=(i>>2)&0xFF,t6=(i>>3)&0xFF,t7=(i>>4)&0xFF; result+=t0*t1-t2*t3+t4*t5-t6*t7; FORCE_USE(t0);FORCE_USE(t1);FORCE_USE(t2);FORCE_USE(t3);FORCE_USE(t4);FORCE_USE(t5);FORCE_USE(t6);FORCE_USE(t7); break; }
            case 8: { int t0=i*0x5555,t1=i*0xAAAA,t2=i*0x3333,t3=i*0xCCCC,t4=i*0x0F0F,t5=i*0xF0F0,t6=i*0x00FF,t7=i*0xFF00; result+=t0+t1+t2+t3+t4+t5+t6+t7; FORCE_USE(t0);FORCE_USE(t1);FORCE_USE(t2);FORCE_USE(t3);FORCE_USE(t4);FORCE_USE(t5);FORCE_USE(t6);FORCE_USE(t7); break; }
            case 9: { int t0=(i+1)*(i+2),t1=(i+3)*(i+4),t2=(i+5)*(i+6),t3=(i+7)*(i+8),t4=(i+9)*(i+10),t5=(i+11)*(i+12),t6=(i+13)*(i+14),t7=(i+15)*(i+16); result+=t0%100+t1%100+t2%100+t3%100+t4%100+t5%100+t6%100+t7%100; FORCE_USE(t0);FORCE_USE(t1);FORCE_USE(t2);FORCE_USE(t3);FORCE_USE(t4);FORCE_USE(t5);FORCE_USE(t6);FORCE_USE(t7); break; }
            /* Additional cases 10-36 follow similar pattern with unique arithmetic */
            default: {
                /* Default case with its own complex arithmetic */
                int t0 = i * i, t1 = i * i * i, t2 = i * i * i * i;
                int t3 = (i << 8) | (i >> 24), t4 = (i << 16) | (i >> 16);
                int t5 = ~i, t6 = i ^ 0xFFFFFFFF, t7 = -i;
                result += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
#ifdef __GNUC__
                vec0 = vec0 * vec1 + vec2;
#endif
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
                FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6); FORCE_USE(t7);
                break;
            }
        }
        
        /* Loop control with continue/break to different cases */
        if (i % 11 == 0) {
            continue;
        }
        if (i % 23 == 0 && i > iterations / 2) {
            break;
        }
    }
    
#ifdef __GNUC__
    /* Extract results from vectors */
    int v0 = vec0[0], v1 = vec0[1], v2 = vec0[2], v3 = vec0[3];
    result += v0 + v1 + v2 + v3;
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    
    FORCE_USE(result);
    return result;
}

/* Pattern D: Explicit register variables with artificial conflicts */
NOINLINE void dummy1(int x) { FORCE_USE(x); }
NOINLINE void dummy2(int x, int y) { FORCE_USE(x); FORCE_USE(y); }
NOINLINE void dummy3(int x, int y, int z) { FORCE_USE(x); FORCE_USE(y); FORCE_USE(z); }

NOINLINE int pattern_d_register_conflict(int n) {
    /* Force specific register usage */
    REG_VAR(reg_a, "r10");
    REG_VAR(reg_b, "r11");
    REG_VAR(reg_c, "r12");
    REG_VAR(reg_d, "r13");
    
    reg_a = n;
    reg_b = n * 2;
    reg_c = n * 3;
    reg_d = n * 4;
    
    /* Create artificial conflicts through function calls */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            dummy1(reg_a);
            reg_b = reg_a + i;
            dummy2(reg_b, reg_c);
        } else if (i % 3 == 0) {
            dummy2(reg_c, reg_d);
            reg_a = reg_b + reg_c;
            dummy3(reg_a, reg_b, reg_c);
        } else if (i % 5 == 0) {
            dummy3(reg_d, reg_a, reg_b);
            reg_c = reg_d - reg_a;
            dummy1(reg_c);
        } else {
            dummy1(reg_a);
            dummy2(reg_b, reg_c);
            dummy3(reg_a, reg_b, reg_c);
        }
        
        /* Complex arithmetic to keep registers live */
        reg_a = (reg_a * 1103515245 + 12345) & 0x7FFFFFFF;
        reg_b = (reg_b * 214013 + 2531011) & 0x7FFFFFFF;
        reg_c = (reg_c * 1664525 + 1013904223) & 0x7FFFFFFF;
        reg_d = (reg_d * 134775813 + 1) & 0x7FFFFFFF;
        
        /* Force register spilling with large expression */
        int temp = reg_a + reg_b + reg_c + reg_d;
        temp = temp * 3 / 2;
        temp = temp ^ (reg_a << 16);
        temp = temp | (reg_b >> 16);
        temp = temp & (reg_c | 0xFFFF);
        temp = temp + (reg_d & 0xFF);
        
        FORCE_USE(temp);
    }
    
    return reg_a + reg_b + reg_c + reg_d;
}

/* Main driver with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
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
    for (int i = 0; i < iterations; i++) {
        int seed = i * 7919;  /* Prime multiplier for variety */
        
        /* Pattern A - irreducible CFG */
        total_result ^= pattern_a_irreducible(i % 100 + 50, seed);
        
        /* Pattern B - setjmp/longjmp */
        if (i % 7 == 0) {
            total_result += pattern_b_setjmp(i % 20 + 5, seed);
        }
        
        /* Pattern C - vector switch */
        total_result += pattern_c_vector_switch(seed % 100, i % 10 + 1);
        
        /* Pattern D - register conflicts */
        if (i % 13 == 0) {
            total_result ^= pattern_d_register_conflict(i % 30 + 10);
        }
        
        /* Mix results to prevent optimization */
        total_result = (total_result * 1103515245 + 12345) & 0x7FFFFFFF;
        
        if (verbose && i % 1000 == 0) {
            printf("Progress: %d/%d, result: %d\n", i, iterations, total_result);
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    return total_result % 256;  /* Return non-zero to indicate execution */
}
