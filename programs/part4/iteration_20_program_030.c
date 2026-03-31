/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow register allocator
 * Designed to trigger print_node function with special indices:
 * ENTRY_BLOCK, ENTRY_BLOCK+1, 2*EXIT_BLOCK, 2*EXIT_BLOCK+1,
 * new_exit_index, and new_entry_index
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units for each pressure function */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* ============================================
   PATTERN A: Integer arithmetic chain with 30+ variables
   Creates massive register pressure in a loop
============================================ */
NOINLINE static long pattern_a(int input) {
    /* 35 integer variables to exceed most register files */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35;
    long sum = 0;
    
    /* Complex initialization chain */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - a1;
    a4 = a3 + input;
    a5 = a4 * a3;
    a6 = a5 / (a1 ? a1 : 1);
    a7 = a6 << 2;
    a8 = a7 ^ a6;
    a9 = a8 | a7;
    a10 = a9 & a8;
    
    a11 = a10 + a9;
    a12 = a11 * a10;
    a13 = a12 - a11;
    a14 = a13 + a12;
    a15 = a14 * a13;
    a16 = a15 / (a14 ? a14 : 1);
    a17 = a16 << 1;
    a18 = a17 ^ a16;
    a19 = a18 | a17;
    a20 = a19 & a18;
    
    a21 = a20 + a19;
    a22 = a21 * a20;
    a23 = a22 - a21;
    a24 = a23 + a22;
    a25 = a24 * a23;
    a26 = a25 / (a24 ? a24 : 1);
    a27 = a26 << 3;
    a28 = a27 ^ a26;
    a29 = a28 | a27;
    a30 = a29 & a28;
    
    a31 = a30 + a29;
    a32 = a31 * a30;
    a33 = a32 - a31;
    a34 = a33 + a32;
    a35 = a34 * a33;
    
    /* Interdependent loop to prevent optimization */
    for (int i = 0; i < 100; i++) {
        a1 = (a1 + a35) ^ i;
        a2 = (a2 + a1) ^ i;
        a3 = (a3 + a2) ^ i;
        a4 = (a4 + a3) ^ i;
        a5 = (a5 + a4) ^ i;
        a6 = (a6 + a5) ^ i;
        a7 = (a7 + a6) ^ i;
        a8 = (a8 + a7) ^ i;
        a9 = (a9 + a8) ^ i;
        a10 = (a10 + a9) ^ i;
        
        a11 = (a11 + a10) ^ i;
        a12 = (a12 + a11) ^ i;
        a13 = (a13 + a12) ^ i;
        a14 = (a14 + a13) ^ i;
        a15 = (a15 + a14) ^ i;
        a16 = (a16 + a15) ^ i;
        a17 = (a17 + a16) ^ i;
        a18 = (a18 + a17) ^ i;
        a19 = (a19 + a18) ^ i;
        a20 = (a20 + a19) ^ i;
        
        a21 = (a21 + a20) ^ i;
        a22 = (a22 + a21) ^ i;
        a23 = (a23 + a22) ^ i;
        a24 = (a24 + a23) ^ i;
        a25 = (a25 + a24) ^ i;
        a26 = (a26 + a25) ^ i;
        a27 = (a27 + a26) ^ i;
        a28 = (a28 + a27) ^ i;
        a29 = (a29 + a28) ^ i;
        a30 = (a30 + a29) ^ i;
        
        a31 = (a31 + a30) ^ i;
        a32 = (a32 + a31) ^ i;
        a33 = (a33 + a32) ^ i;
        a34 = (a34 + a33) ^ i;
        a35 = (a35 + a34) ^ i;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
    }
    
    /* Complex summation to use all variables */
    sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
          a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
          a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
          a31 + a32 + a33 + a34 + a35;
    
    return sum;
}

/* ============================================
   PATTERN B: Floating-point intensive computation
   Pressures floating-point register file
============================================ */
NOINLINE static double pattern_b(double input) {
    /* 20 double variables for FP register pressure */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double result;
    
    /* FP computation chain */
    b1 = input + 1.0;
    b2 = b1 * 2.0;
    b3 = b2 - b1;
    b4 = b3 / (b1 + 0.001);
    b5 = b4 * b3;
    b6 = b5 + b4;
    b7 = b6 * 0.5;
    b8 = b7 - b6;
    b9 = b8 / (b7 + 0.001);
    b10 = b9 * 3.14159;
    
    b11 = b10 + b9;
    b12 = b11 * 2.71828;
    b13 = b12 - b11;
    b14 = b13 / (b12 + 0.001);
    b15 = b14 * b13;
    b16 = b15 + b14;
    b17 = b16 * 0.333;
    b18 = b17 - b16;
    b19 = b18 / (b17 + 0.001);
    b20 = b19 * 1.61803;
    
    /* Nested loops with complex control flow */
    for (int i = 0; i < 50; i++) {
        if (i % 3 == 0) {
            for (int j = 0; j < 10; j++) {
                b1 = b1 * 1.01 + b20;
                b2 = b2 * 0.99 + b19;
                b3 = b3 * 1.02 + b18;
                b4 = b4 * 0.98 + b17;
                b5 = b5 * 1.03 + b16;
                
                /* Mix integer index for more complexity */
                b6 = b6 + (j * 0.1);
                b7 = b7 - (j * 0.05);
                b8 = b8 * (1.0 + j * 0.001);
                b9 = b9 / (1.0 + j * 0.002);
                b10 = b10 + (i * 0.01);
            }
        } else if (i % 3 == 1) {
            continue;  /* Skip iteration - creates CFG complexity */
        } else {
            b11 = b11 * b10;
            b12 = b12 + b11;
            b13 = b13 - b12;
            b14 = b14 / (b13 + 1.0);
            b15 = b15 * b14;
        }
        
        /* Prevent elimination */
        asm volatile("" : "+x"(b1), "+x"(b2), "+x"(b3), "+x"(b4), "+x"(b5));
    }
    
    /* Final computation using all variables */
    result = b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
             b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20;
    
    return result;
}

/* ============================================
   PATTERN C: Complex control flow with switch statement
   Creates many basic blocks for CFG complexity
============================================ */
NOINLINE static int pattern_c(int input) {
    int result = input;
    
    /* Switch with 25 cases - creates many basic blocks */
    switch (input % 25) {
        case 0: result = result * 2; break;
        case 1: result = result + 7; break;
        case 2: result = result - 3; break;
        case 3: result = result ^ 0xFF; break;
        case 4: result = result | 0xAA; break;
        case 5: result = result & 0x55; break;
        case 6: result = result << 1; break;
        case 7: result = result >> 2; break;
        case 8: result = result * 3; break;
        case 9: result = result / 2; break;
        case 10: result = result % 13; break;
        case 11: result = ~result; break;
        case 12: result = result + result; break;
        case 13: result = result - result/2; break;
        case 14: result = result ^ result; break;
        case 15: result = result | (result << 8); break;
        case 16: result = result & 0xFFFF; break;
        case 17: result = result << 4; break;
        case 18: result = result >> 4; break;
        case 19: result = result * 5; break;
        case 20: result = result / 3; break;
        case 21: result = result % 7; break;
        case 22: result = -result; break;
        case 23: result = result + 100; break;
        case 24: result = result - 50; break;
        default: result = 0; break;
    }
    
    /* Nested loops with breaks and continues */
    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) {
            for (int j = 0; j < 20; j++) {
                if (j == result % 5) {
                    result += j;
                    break;  /* Inner loop break */
                } else if (j == result % 3) {
                    result -= j;
                    continue;  /* Skip rest of iteration */
                }
                result = result ^ j;
            }
            continue;  /* Skip to next outer iteration */
        }
        
        if (i == result % 17) {
            result = result * 2;
            break;  /* Outer loop break */
        }
        
        result = result + i;
    }
    
    /* Computed goto for additional CFG complexity (GCC extension) */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    int idx = result % 5;
    
    goto *labels[idx];
    
label0:
    result += 1000;
    goto end;
label1:
    result += 2000;
    goto end;
label2:
    result += 3000;
    goto end;
label3:
    result += 4000;
    goto end;
label4:
    result += 5000;
    goto end;
    
end:
    return result;
}

/* ============================================
   PATTERN D: SIMD vector operations
   Pressures vector/SIMD registers
============================================ */
NOINLINE static v4si pattern_d(v4si input) {
    /* 8 vector variables for SIMD register pressure */
    v4si v1, v2, v3, v4, v5, v6, v7, v8;
    v4si result;
    
    /* Initialize vectors */
    v1 = input + (v4si){1, 2, 3, 4};
    v2 = v1 * (v4si){2, 2, 2, 2};
    v3 = v2 - v1;
    v4 = v3 + (v4si){4, 3, 2, 1};
    v5 = v4 * v3;
    v6 = v5 + v4;
    v7 = v6 - v5;
    v8 = v7 * (v4si){3, 3, 3, 3};
    
    /* Vector loop with shuffling */
    for (int i = 0; i < 50; i++) {
        /* Rotate vectors */
        v4si temp = v1;
        v1 = v2;
        v2 = v3;
        v3 = v4;
        v4 = v5;
        v5 = v6;
        v6 = v7;
        v7 = v8;
        v8 = temp;
        
        /* Mix with loop counter */
        v4si idx = {i, i+1, i+2, i+3};
        v1 = v1 + idx;
        v2 = v2 - idx;
        v3 = v3 * (idx + 1);
        v4 = v4 / ((idx % 5) + 1);
        
        /* Conditional vector operations */
        if (i % 3 == 0) {
            v5 = v5 | idx;
            v6 = v6 & ~idx;
        } else if (i % 3 == 1) {
            v7 = v7 ^ idx;
            v8 = v8 << (i % 4);
        }
        
        /* Prevent elimination */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3), "+x"(v4));
    }
    
    /* Combine all vectors */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    return result;
}

/* ============================================
   PATTERN E: Explicit register variables
   Conflicts with allocator's choices
============================================ */
NOINLINE static int pattern_e(int input) {
    /* Explicit register variables that conflict */
    register int r0 asm ("r12") = input;
    register int r1 asm ("r13") = input + 1;
    register int r2 asm ("r14") = input + 2;
    register int r3 asm ("r15") = input + 3;
    
    int local1, local2, local3, local4, local5;
    int local6, local7, local8, local9, local10;
    
    /* Mix explicit registers with locals */
    local1 = r0 * 2;
    local2 = r1 + local1;
    local3 = r2 - local2;
    local4 = r3 * local3;
    
    /* Force spilling */
    for (int i = 0; i < 100; i++) {
        r0 = r0 + i;
        r1 = r1 ^ i;
        r2 = r2 * (i % 7 + 1);
        r3 = r3 - i;
        
        local5 = local1 + r0;
        local6 = local2 + r1;
        local7 = local3 + r2;
        local8 = local4 + r3;
        local9 = local5 * local6;
        local10 = local7 * local8;
        
        /* Swap values to increase pressure */
        int temp = r0;
        r0 = r1;
        r1 = r2;
        r2 = r3;
        r3 = temp;
        
        temp = local1;
        local1 = local2;
        local2 = local3;
        local3 = local4;
        local4 = temp;
        
        /* Prevent elimination */
        asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3));
        asm volatile("" : "+r"(local1), "+r"(local2), "+r"(local3), "+r"(local4));
    }
    
    return r0 + r1 + r2 + r3 + local1 + local2 + local3 + local4 +
           local5 + local6 + local7 + local8 + local9 + local10;
}

/* ============================================
   Main function - calls all patterns
============================================ */
COLD int main(int argc, char *argv[]) {
    long total = 0;
    int iterations = 10;
    
    /* Use CPU features to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        iterations = 15;  /* More iterations if AVX2 available */
    }
#else
    /* Stub for non-GCC compilers */
    #define __builtin_cpu_supports(x) 0
#endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    
    /* Call each pattern multiple times with different inputs */
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < num_inputs; i++) {
            int idx = (iter + i) % num_inputs;
            
            /* Pattern A - Integer pressure */
            total += pattern_a(inputs[idx]);
            
            /* Pattern B - Floating-point pressure */
            total += (long)pattern_b((double)inputs[idx]);
            
            /* Pattern C - Control flow complexity */
            total += pattern_c(inputs[idx]);
            
            /* Pattern D - SIMD vector pressure */
            v4si vec_input = {inputs[idx], inputs[idx]+1, 
                              inputs[idx]+2, inputs[idx]+3};
            v4si vec_result = pattern_d(vec_input);
            total += vec_result[0] + vec_result[1] + 
                     vec_result[2] + vec_result[3];
            
            /* Pattern E - Explicit register conflicts */
            total += pattern_e(inputs[idx]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total: %ld\n", total);
    
    return (int)(total % 256);
}
