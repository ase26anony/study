/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * in register allocation, specifically to exercise the print_node function
 * with special node indices: ENTRY_BLOCK, EXIT_BLOCK, new_exit_index, 
 * and new_entry_index.
 *
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_mcf_coverage.c
 * Then run: ./test_mcf_coverage (to generate profile data)
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
   PATTERN A: Integer arithmetic chain with 30+ variables
   Creates massive register pressure in a loop
============================================ */
NOINLINE static int pattern_a(int input) {
    /* 30+ integer variables to pressure general purpose registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    volatile int seed = input; /* volatile to prevent optimization */
    
    /* Complex interdependent calculations */
    a1 = seed + 1;
    a2 = a1 * seed;
    a3 = a2 - a1;
    a4 = a3 / (seed ? seed : 1);
    a5 = a4 << 2;
    a6 = a5 ^ a2;
    a7 = a6 | a3;
    a8 = a7 & a4;
    a9 = a8 + a5;
    a10 = a9 - a6;
    
    a11 = a10 * a1;
    a12 = a11 + a2;
    a13 = a12 - a3;
    a14 = a13 ^ a4;
    a15 = a14 | a5;
    a16 = a15 & a6;
    a17 = a16 << 1;
    a18 = a17 >> 2;
    a19 = a18 + a7;
    a20 = a19 - a8;
    
    a21 = a20 * a9;
    a22 = a21 + a10;
    a23 = a22 - a11;
    a24 = a23 ^ a12;
    a25 = a24 | a13;
    a26 = a25 & a14;
    a27 = a26 << 3;
    a28 = a27 >> 1;
    a29 = a28 + a15;
    a30 = a29 - a16;
    
    a31 = a30 * a17;
    a32 = a31 + a18;
    a33 = a32 - a19;
    a34 = a33 ^ a20;
    a35 = a34 | a21;
    a36 = a35 & a22;
    a37 = a36 << 2;
    a38 = a37 >> 3;
    a39 = a38 + a23;
    a40 = a39 - a24;
    
    /* Loop with all variables live to maximize pressure */
    for (int i = 0; i < 100; i++) {
        a1 = a40 + i;
        a2 = a1 ^ a39;
        a3 = a2 | a38;
        a4 = a3 & a37;
        a5 = a4 + a36;
        a6 = a5 - a35;
        a7 = a6 * a34;
        a8 = a7 + a33;
        a9 = a8 - a32;
        a10 = a9 ^ a31;
        
        /* Force all variables to be used */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5),
                        "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
    }
    
    /* Complex return to prevent dead code elimination */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
}

/* ============================================
   PATTERN B: Floating-point intensive computation
   Pressures floating-point registers
============================================ */
NOINLINE static double pattern_b(double input) {
    /* 20+ double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input + 1.0;
    b2 = b1 * 1.6180339887; /* golden ratio */
    b3 = b2 / (input ? input : 1.0);
    b4 = b3 - b1;
    b5 = b4 * b2;
    b6 = b5 + b3;
    b7 = b6 - b4;
    b8 = b7 * b5;
    b9 = b8 / b6;
    b10 = b9 - b7;
    
    b11 = b10 * 2.7182818284; /* e */
    b12 = b11 + b8;
    b13 = b12 - b9;
    b14 = b13 * 3.1415926535; /* pi */
    b15 = b14 / b10;
    b16 = b15 - b11;
    b17 = b16 * b12;
    b18 = b17 + b13;
    b19 = b18 - b14;
    b20 = b19 / b15;
    
    b21 = b20 * b16;
    b22 = b21 + b17;
    b23 = b22 - b18;
    b24 = b23 * b19;
    b25 = b24 / b20;
    
    /* Loop with mixed operations */
    for (int i = 0; i < 50; i++) {
        b1 = b25 * i;
        b2 = b1 + b24;
        b3 = b2 - b23;
        b4 = b3 * b22;
        b5 = b4 / b21;
        
        /* Mix integer and float to create conversion pressure */
        int temp = (int)b5;
        b6 = b5 + temp;
        b7 = b6 - b1;
        
        asm volatile("" : "+f"(b1), "+f"(b2), "+f"(b3), "+f"(b4), "+f"(b5),
                        "+f"(b6), "+f"(b7));
    }
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* ============================================
   PATTERN C: Complex control flow with switch
   Creates many basic blocks for CFG complexity
============================================ */
NOINLINE static int pattern_c(int input) {
    int result = input;
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) continue;
        
        for (int j = 0; j < 50; j++) {
            if (j % 7 == 0) break;
            
            /* Switch with 20+ cases */
            switch ((i * j + input) % 23) {
                case 0:  result += i * 2; break;
                case 1:  result -= j * 3; break;
                case 2:  result ^= i + j; break;
                case 3:  result |= i << 2; break;
                case 4:  result &= j << 3; break;
                case 5:  result += i * j; break;
                case 6:  result -= i / (j ? j : 1); break;
                case 7:  result ^= j * i; break;
                case 8:  result |= result << 1; break;
                case 9:  result &= result >> 1; break;
                case 10: result += (i ^ j); break;
                case 11: result -= (i | j); break;
                case 12: result ^= (i & j); break;
                case 13: result |= i * 3; break;
                case 14: result &= j * 4; break;
                case 15: result += i << j; break;
                case 16: result -= j << i; break;
                case 17: result ^= i >> 1; break;
                case 18: result |= j >> 2; break;
                case 19: result &= i % 5; break;
                case 20: result += j % 7; break;
                case 21: result -= i % 11; break;
                case 22: result ^= j % 13; break;
                default: result = ~result; break;
            }
            
            /* Early exit creates more CFG edges */
            if (result > 1000000) goto early_exit;
        }
        
        if (i % 13 == 0) {
            /* Computed goto (GCC extension) for extra CFG complexity */
            void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
            goto *labels[i % 5];
            
            label1: result += 111; continue;
            label2: result -= 222; continue;
            label3: result ^= 333; continue;
            label4: result |= 444; continue;
            label5: result &= 555; continue;
        }
    }
    
early_exit:
    return result;
}

/* ============================================
   PATTERN D: Vector/SIMD operations
   Pressures vector registers
============================================ */
NOINLINE static v4si pattern_d(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 3, 4, 5};
    v3 = v2 - v1;
    v4 = v3 + (v4si){5, 6, 7, 8};
    v5 = v4 * (v4si){3, 4, 5, 6};
    
    v6 = v5 - v2;
    v7 = v6 + v3;
    v8 = v7 * v4;
    v9 = v8 - v5;
    v10 = v9 + v6;
    
    v11 = v10 * v7;
    v12 = v11 - v8;
    v13 = v12 + v9;
    v14 = v13 * v10;
    v15 = v14 - v11;
    
    v16 = v15 + v12;
    v17 = v16 * v13;
    v18 = v17 - v14;
    v19 = v18 + v15;
    v20 = v19 * v16;
    
    /* Loop with vector operations */
    for (int i = 0; i < 25; i++) {
        v1 = v20 + (v4si){i, i+1, i+2, i+3};
        v2 = v1 * v19;
        v3 = v2 - v18;
        v4 = v3 + v17;
        v5 = v4 * v16;
        
        /* Shuffle-like operations */
        v6 = __builtin_shuffle(v5, v15, (v4si){3, 2, 1, 0});
        v7 = __builtin_shuffle(v6, v14, (v4si){1, 0, 3, 2});
        
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3), "+x"(v4), "+x"(v5),
                        "+x"(v6), "+x"(v7));
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* ============================================
   PATTERN E: Explicit register variables
   Conflicts with allocator's choices
============================================ */
NOINLINE static int pattern_e(int input) {
    /* Explicit register variables that conflict */
    register int r1 asm ("r12") = input + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 - input;
    register int r4 asm ("r15") = r3 | r1;
    register int r5 asm ("rbx") = r4 & r2;
    
    /* More variables to pressure registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Interact with register variables */
    v1 = r1 + r2;
    v2 = r3 - r4;
    v3 = r5 * r1;
    v4 = v1 ^ v2;
    v5 = v3 | v4;
    
    v6 = v5 + r2;
    v7 = v6 - r3;
    v8 = v7 * r4;
    v9 = v8 ^ r5;
    v10 = v9 | v1;
    
    v11 = v10 + v2;
    v12 = v11 - v3;
    v13 = v12 * v4;
    v14 = v13 ^ v5;
    v15 = v14 | v6;
    
    v16 = v15 + v7;
    v17 = v16 - v8;
    v18 = v17 * v9;
    v19 = v18 ^ v10;
    v20 = v19 | v11;
    
    /* Force register variable usage in loop */
    for (int i = 0; i < 50; i++) {
        r1 = v20 + i;
        r2 = r1 ^ v19;
        r3 = r2 | v18;
        r4 = r3 & v17;
        r5 = r4 + v16;
        
        /* Force spills by using many variables */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4), "+r"(r5));
    }
    
    return r1 + r2 + r3 + r4 + r5 + v1 + v2 + v3 + v4 + v5 +
           v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
           v16 + v17 + v18 + v19 + v20;
}

/* ============================================
   Helper to prevent optimization
============================================ */
static void use_result(int result) {
    /* Use result to prevent dead code elimination */
    static volatile int sink;
    sink = result;
}

/* ============================================
   Main function - cold attribute may affect block ordering
============================================ */
COLD int main(int argc, char **argv) {
    int results[5] = {0};
    double fp_result = 0.0;
    v4si vec_result = {0};
    
    /* Use CPU feature check to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation strategies */
        asm volatile("" ::: "memory");
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    /* Call all patterns multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        results[0] += pattern_a(inputs[i]);
        fp_result += pattern_b((double)inputs[i]);
        results[1] += pattern_c(inputs[i]);
        
        v4si vec_input = {inputs[i], inputs[i]+1, inputs[i]+2, inputs[i]+3};
        vec_result += pattern_d(vec_input);
        
        results[2] += pattern_e(inputs[i]);
        
        /* Mix patterns to create interprocedural pressure */
        if (i % 2 == 0) {
            results[3] = pattern_a(pattern_c(inputs[i]));
        } else {
            results[4] = pattern_e(pattern_a(inputs[i]));
        }
    }
    
    /* Use all results to prevent elimination */
    int final_result = results[0] + results[1] + results[2] + results[3] + results[4];
    final_result += (int)fp_result;
    final_result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    use_result(final_result);
    
    /* Optional debug output */
    if (argc > 1) {
        printf("Final result: %d\n", final_result);
    }
    
    return 0;
}
