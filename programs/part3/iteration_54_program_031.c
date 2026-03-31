/* test_mcf.c - Complex CFG generator for MCF pass testing */
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

/* Global flag for debug output */
static volatile int verbose = 0;

/* ============================================
   PATTERN A: ENTRY/EXIT block stress
   ============================================ */
HOT NOINLINE USED
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create complex irreducible region with goto */
    if (iterations > 0) {
        int state = 0;
        
        for (i = 0; i < iterations; i++) {
            /* Deeply nested if-else chain */
            if (state == 0) {
                if (result & 1) {
                    state = 1;
                    goto label_a;
                } else {
                    state = 2;
                    goto label_b;
                }
            } else if (state == 1) {
label_a:
                for (j = 0; j < 5; j++) {
                    result = (result * 1103515245 + 12345) & 0x7fffffff;
                    if (result % 7 == 0) {
                        state = 3;
                        goto label_c;
                    }
                }
                state = 0;
                continue;
            } else if (state == 2) {
label_b:
                for (k = 0; k < 3; k++) {
                    result ^= (result >> 13);
                    result ^= (result << 17);
                    result ^= (result >> 5);
                }
                state = 4;
                goto label_d;
            } else if (state == 3) {
label_c:
                result = result * 3 + 1;
                while (result > 1000) {
                    result >>= 1;
                }
                state = 2;
                goto label_b;
            } else {
label_d:
                result = (result << 1) | ((result >> 31) & 1);
                state = 0;
            }
            
            /* More branching to create many edges */
            switch (result % 8) {
                case 0: result += 1; break;
                case 1: result -= 1; break;
                case 2: result *= 2; break;
                case 3: result /= 2; break;
                case 4: result ^= 0xAAAAAAAA; break;
                case 5: result |= 0x55555555; break;
                case 6: result &= 0x33333333; break;
                case 7: result = ~result; break;
            }
        }
    }
    
    /* Force all variables to be considered live */
    asm volatile("" : : "g"(i), "g"(j), "g"(k));
    return result;
}

/* ============================================
   PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp
   ============================================ */
static jmp_buf jump_buffer;

NOINLINE USED
int pattern_b_new_indices(int depth, int value) {
    volatile int r0 = value, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    volatile int r10, r11, r12, r13, r14, r15;
    
    if (depth <= 0) {
        return value;
    }
    
    /* Initialize many variables to create register pressure */
    r1 = r0 * 2; r2 = r1 + 1; r3 = r2 ^ 0xFF; r4 = r3 << 3;
    r5 = r4 >> 1; r6 = r5 | 0xAA; r7 = r6 & 0x55; r8 = r7 + r6;
    r9 = r8 - r7; r10 = r9 * r8; r11 = r10 % 17; r12 = r11 << 4;
    r13 = r12 >> 2; r14 = r13 ^ r12; r15 = r14 + r13;
    
    /* Use setjmp/longjmp to create exceptional edges */
    if (setjmp(jump_buffer) == 0) {
        /* Complex loop with many variables live */
        for (int i = 0; i < depth; i++) {
            /* Update all variables in a way that prevents optimization */
            r0 = r15 ^ i; r1 = r0 + r14; r2 = r1 * r13;
            r3 = r2 - r12; r4 = r3 | r11; r5 = r4 & r10;
            r6 = r5 ^ r9; r7 = r6 + r8; r8 = r7 - r6;
            r9 = r8 * r5; r10 = r9 / (r4 + 1); r11 = r10 % 19;
            r12 = r11 << (i & 3); r13 = r12 >> 1; r14 = r13 | 1;
            r15 = r14 ^ 0xDEADBEEF;
            
            /* Conditional longjmp creates complex CFG */
            if ((r15 & 0xFFF) == 0xABC) {
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* Handler path - different variable usage */
        r0 = (r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
              r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15) & 0xFFFF;
    }
    
    /* Force all variables to be live */
    asm volatile("" : : 
        "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
        "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
        "g"(r12), "g"(r13), "g"(r14), "g"(r15)
    );
    
    return r0 + depth;
}

/* ============================================
   PATTERN C: Mixed vector/scalar pressure
   ============================================ */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_mixed_pressure(int selector, int iterations) {
    volatile int result = 0;
    
#ifdef __GNUC__
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    /* Large switch with 30+ cases */
    for (int iter = 0; iter < iterations; iter++) {
        int r0 = iter, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        int r10, r11, r12, r13, r14, r15;
        
        switch ((selector + iter) % 35) {
            case 0:
                r0 = r0 * 2; r1 = r0 + 1; r2 = r1 ^ 0xFF;
                r3 = r2 << 3; r4 = r3 >> 1; r5 = r4 | 0xAA;
                r6 = r5 & 0x55; r7 = r6 + r5; r8 = r7 - r6;
                r9 = r8 * r7; r10 = r9 / (r8 + 1); r11 = r10 % 17;
                r12 = r11 << 4; r13 = r12 >> 2; r14 = r13 ^ r12;
                r15 = r14 + r13;
#ifdef __GNUC__
                vec0 += vec1; vec1 *= vec2;
#endif
                break;
            case 1:
                r0 = r0 ^ 0x5555; r1 = r0 >> 1; r2 = r1 << 2;
                r3 = r2 | 0x3333; r4 = r3 & 0xCCCC; r5 = r4 + r3;
                r6 = r5 - r4; r7 = r6 * r5; r8 = r7 / (r6 + 1);
                r9 = r8 % 23; r10 = r9 << 3; r11 = r10 >> 1;
                r12 = r11 ^ r10; r13 = r12 + r11; r14 = r13 * 3;
                r15 = r14 + 7;
#ifdef __GNUC__
                vec2 -= vec3; vec3 |= vec0;
#endif
                break;
            case 2:
                r0 = ~r0; r1 = r0 * 3; r2 = r1 + 5; r3 = r2 ^ 0xAAAA;
                r4 = r3 << 1; r5 = r4 >> 3; r6 = r5 | 0x8888;
                r7 = r6 & 0x7777; r8 = r7 + r6; r9 = r8 - r7;
                r10 = r9 * r8; r11 = r10 / (r9 + 1); r12 = r11 % 29;
                r13 = r12 << 2; r14 = r13 >> 1; r15 = r14 ^ r13;
#ifdef __GNUC__
                vec0 ^= vec1; vec1 &= vec2;
#endif
                break;
            /* Cases 3-32 follow similar pattern with unique arithmetic */
            case 3: r0 = r0 * 5 + 1; r1 = r0 ^ 0x1234; /* ... */ break;
            case 4: r0 = r0 / 2 | 0x8000; /* ... */ break;
            case 5: r0 = (r0 << 4) | (r0 >> 28); /* ... */ break;
            case 6: r0 = r0 * 7 - 3; /* ... */ break;
            case 7: r0 = r0 ^ (r0 >> 16); /* ... */ break;
            case 8: r0 = r0 * 11 % 65536; /* ... */ break;
            case 9: r0 = ~(r0 * 13); /* ... */ break;
            case 10: r0 = (r0 << 8) | (r0 >> 24); /* ... */ break;
            case 11: r0 = r0 * 17 + 19; /* ... */ break;
            case 12: r0 = r0 ^ 0xDEAD; /* ... */ break;
            case 13: r0 = r0 * 23 >> 4; /* ... */ break;
            case 14: r0 = r0 | 0xBEEF; /* ... */ break;
            case 15: r0 = r0 & 0xF0F0; /* ... */ break;
            case 16: r0 = r0 * 29 - 11; /* ... */ break;
            case 17: r0 = r0 ^ (r0 << 8); /* ... */ break;
            case 18: r0 = r0 * 31 % 32768; /* ... */ break;
            case 19: r0 = ~(r0 * 37); /* ... */ break;
            case 20: r0 = (r0 << 12) | (r0 >> 20); /* ... */ break;
            case 21: r0 = r0 * 41 + 43; /* ... */ break;
            case 22: r0 = r0 ^ 0xCAFE; /* ... */ break;
            case 23: r0 = r0 * 47 >> 6; /* ... */ break;
            case 24: r0 = r0 | 0xBABE; /* ... */ break;
            case 25: r0 = r0 & 0x0F0F; /* ... */ break;
            case 26: r0 = r0 * 53 - 17; /* ... */ break;
            case 27: r0 = r0 ^ (r0 << 4); /* ... */ break;
            case 28: r0 = r0 * 59 % 16384; /* ... */ break;
            case 29: r0 = ~(r0 * 61); /* ... */ break;
            case 30: r0 = (r0 << 16) | (r0 >> 16); /* ... */ break;
            case 31: r0 = r0 * 67 + 71; /* ... */ break;
            case 32: r0 = r0 ^ 0xF00D; /* ... */ break;
            case 33: r0 = r0 * 73 >> 8; /* ... */ break;
            case 34:
                /* Complex case with continue to different iteration */
                r0 = r0 * 79 - 23;
                if (iter < iterations - 1) {
                    selector = (selector + r0) % 35;
                    continue;  /* Creates back-edge to switch start */
                }
                break;
        }
        
        /* Force all scalar variables live */
        asm volatile("" : : 
            "g"(r0), "g"(r1), "g"(r2), "g"(r3), "g"(r4), "g"(r5),
            "g"(r6), "g"(r7), "g"(r8), "g"(r9), "g"(r10), "g"(r11),
            "g"(r12), "g"(r13), "g"(r14), "g"(r15)
        );
        
        result += r0;
        
        /* Break to different case based on condition */
        if ((result & 0xFF) == 0) {
            selector = (selector + 5) % 35;
            if (iter < iterations - 1) {
                continue;
            }
        }
    }
    
    return result;
}

/* ============================================
   PATTERN D: Artificial register conflicts
   ============================================ */
#ifdef __GNUC__
register int reg_var1 asm ("r10");
register int reg_var2 asm ("r11");
#else
int reg_var1, reg_var2;
#endif

NOINLINE static void dummy_helper1(int x) {
    asm volatile("" : : "g"(x));
}

NOINLINE static void dummy_helper2(int x, int y) {
    asm volatile("" : : "g"(x), "g"(y));
}

NOINLINE static void dummy_helper3(int x, int y, int z) {
    asm volatile("" : : "g"(x), "g"(y), "g"(z));
}

NOINLINE USED
int pattern_d_register_conflict(int n) {
    volatile int result = 0;
    
#ifdef __GNUC__
    /* Force use of specific registers */
    reg_var1 = n;
    reg_var2 = n * 2;
    
    /* Create conflicts by using same registers for different purposes */
    asm volatile("" : "+r"(reg_var1), "+r"(reg_var2));
#endif
    
    int local1, local2, local3, local4, local5, local6;
    int local7, local8, local9, local10, local11, local12;
    
    /* Complex loop with many calls creating live range splits */
    for (int i = 0; i < n; i++) {
        local1 = i * 3;
        local2 = local1 + 1;
        local3 = local2 ^ 0xFF;
        
        dummy_helper1(local1);
        
        local4 = local3 << 2;
        local5 = local4 >> 1;
        local6 = local5 | 0xAA;
        
        dummy_helper2(local2, local3);
        
        local7 = local6 & 0x55;
        local8 = local7 + local6;
        local9 = local8 - local7;
        
        dummy_helper3(local4, local5, local6);
        
        local10 = local9 * local8;
        local11 = local10 / (local9 + 1);
        local12 = local11 % 17;
        
#ifdef __GNUC__
        /* More register variable usage interspersed */
        reg_var1 = (reg_var1 + local1) ^ local2;
        reg_var2 = (reg_var2 * local3) | local4;
        asm volatile("" : "+r"(reg_var1), "+r"(reg_var2));
#endif
        
        dummy_helper1(local7);
        dummy_helper2(local8, local9);
        dummy_helper3(local10, local11, local12);
        
        result += local12;
        
        /* Conditional with complex control flow */
        if (result % 100 == 0) {
            for (int j = 0; j < 3; j++) {
                local1 = (local1 << j) | (local12 >> j);
                dummy_helper1(local1);
            }
        } else if (result % 50 == 0) {
            int temp = local2;
            local2 = local3;
            local3 = temp;
            dummy_helper2(local2, local3);
        }
    }
    
    /* Force all locals live */
    asm volatile("" : : 
        "g"(local1), "g"(local2), "g"(local3), "g"(local4), "g"(local5),
        "g"(local6), "g"(local7), "g"(local8), "g"(local9), "g"(local10),
        "g"(local11), "g"(local12)
    );
    
#ifdef __GNUC__
    result += reg_var1 + reg_var2;
#endif
    
    return result;
}

/* ============================================
   Main driver with PGO support
   ============================================ */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %d\n", use_avx2);
    }
#endif
    
    /* Call all patterns multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        int seed = i * 1103515245 + 12345;
        
        /* Pattern A - ENTRY/EXIT blocks */
        total_result ^= pattern_a_entry_exit(i % 50 + 1, seed);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 10 == 0) {
            total_result += pattern_b_new_indices(i % 20 + 1, seed);
        }
        
        /* Pattern C - Mixed pressure */
        total_result += pattern_c_mixed_pressure(i % 100, i % 10 + 1);
        
        /* Pattern D - Register conflicts */
        if (i % 5 == 0) {
            total_result ^= pattern_d_register_conflict(i % 30 + 1);
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+g"(total_result));
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    return total_result & 0xFF;
}
