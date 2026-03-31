/* test_mcf_coverage.c
 * 
 * This test program is designed to stress GCC's Min-Cost Flow register allocator
 * to trigger coverage of special node indices in the print_node function.
 * 
 * Compile with: gcc -O3 -fprofile-arcs -ftest-coverage -fdump-rtl-all test_mcf_coverage.c -o test_mcf
 * Run with: ./test_mcf
 * Generate coverage with: gcov test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure each function is compiled independently */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop
 * Forces spill decisions through high register pressure */
NOINLINE static long pattern_a_intensive(int iterations, int seed) {
    /* Declare 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    long sum = 0;
    
    /* Initialize with seed to prevent constant folding */
    a1 = seed;
    
    /* Create complex interdependent chain of operations */
    for (int i = 0; i < iterations; i++) {
        /* Chain of 40 interdependent operations */
        a2 = a1 + i;
        a3 = a2 * a1;
        a4 = a3 - a2;
        a5 = a4 ^ a3;
        a6 = a5 | a4;
        a7 = a6 & a5;
        a8 = a7 << 2;
        a9 = a8 >> 1;
        a10 = a9 + a8;
        a11 = a10 - a9;
        a12 = a11 * a10;
        a13 = a12 / (a11 != 0 ? a11 : 1);
        a14 = a13 % (a12 != 0 ? a12 : 1);
        a15 = a14 ^ a13;
        a16 = a15 | a14;
        a17 = a16 & a15;
        a18 = a17 << 3;
        a19 = a18 >> 2;
        a20 = a19 + a18;
        a21 = a20 - a19;
        a22 = a21 * a20;
        a23 = a22 / (a21 != 0 ? a21 : 1);
        a24 = a23 % (a22 != 0 ? a22 : 1);
        a25 = a24 ^ a23;
        a26 = a25 | a24;
        a27 = a26 & a25;
        a28 = a27 << 1;
        a29 = a28 >> 1;
        a30 = a29 + a28;
        a31 = a30 - a29;
        a32 = a31 * a30;
        a33 = a32 / (a31 != 0 ? a31 : 1);
        a34 = a33 % (a32 != 0 ? a32 : 1);
        a35 = a34 ^ a33;
        a36 = a35 | a34;
        a37 = a36 & a35;
        a38 = a37 << 4;
        a39 = a38 >> 2;
        a40 = a39 + a38;
        
        /* Use all variables to prevent dead code elimination */
        sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
               a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
               a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
        
        /* Rotate values to create data dependencies across iterations */
        a1 = a40 ^ i;
    }
    
    /* Prevent compiler from optimizing away the entire function */
    asm volatile ("" : : "r"(sum) : "memory");
    return sum;
}

/* Pattern B: Floating-point intensive computation
 * Pressures floating-point register class */
NOINLINE static double pattern_b_float_pressure(int iterations, double seed) {
    /* Declare many double variables */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25, b26, b27, b28, b29, b30;
    
    double total = 0.0;
    
    b1 = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex floating-point operations chain */
        b2 = b1 * 1.1 + i;
        b3 = b2 / 1.3 - b1;
        b4 = b3 * b2;
        b5 = b4 / (b3 != 0.0 ? b3 : 1.0);
        b6 = b5 + b4;
        b7 = b6 - b5;
        b8 = b7 * b6;
        b9 = b8 / (b7 != 0.0 ? b7 : 1.0);
        b10 = b9 + b8;
        b11 = b10 - b9;
        b12 = b11 * b10;
        b13 = b12 / (b11 != 0.0 ? b11 : 1.0);
        b14 = b13 + b12;
        b15 = b14 - b13;
        b16 = b15 * b14;
        b17 = b16 / (b15 != 0.0 ? b15 : 1.0);
        b18 = b17 + b16;
        b19 = b18 - b17;
        b20 = b19 * b18;
        b21 = b20 / (b19 != 0.0 ? b19 : 1.0);
        b22 = b21 + b20;
        b23 = b22 - b21;
        b24 = b23 * b22;
        b25 = b24 / (b23 != 0.0 ? b23 : 1.0);
        b26 = b25 + b24;
        b27 = b26 - b25;
        b28 = b27 * b26;
        b29 = b28 / (b27 != 0.0 ? b27 : 1.0);
        b30 = b29 + b28;
        
        total += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                 b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
                 b21 + b22 + b23 + b24 + b25 + b26 + b27 + b28 + b29 + b30;
        
        b1 = b30 * 0.99;
    }
    
    asm volatile ("" : : "r"(total) : "memory");
    return total;
}

/* Pattern C: Complex control flow with switch statement
 * Creates many basic blocks for complex CFG */
NOINLINE static int pattern_c_complex_cfg(int value, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with many cases creates multiple basic blocks */
        switch ((value + i) % 25) {
            case 0: result += i * 2; break;
            case 1: result += i / 2; break;
            case 2: result += i << 1; break;
            case 3: result += i >> 1; break;
            case 4: result += i ^ 0x55; break;
            case 5: result += i | 0xAA; break;
            case 6: result += i & 0xFF; break;
            case 7: result += i * 3; break;
            case 8: result += i / 3; break;
            case 9: result += i << 2; break;
            case 10: result += i >> 2; break;
            case 11: result += i ^ 0xAA; break;
            case 12: result += i | 0x55; break;
            case 13: result += i & 0xF0; break;
            case 14: result += i * 5; break;
            case 15: result += i / 5; break;
            case 16: result += i << 3; break;
            case 17: result += i >> 3; break;
            case 18: result += i ^ 0xF0; break;
            case 19: result += i | 0x0F; break;
            case 20: result += i & 0x0F; break;
            case 21: result += i * 7; break;
            case 22: result += i / 7; break;
            case 23: result += i << 4; break;
            case 24: result += i >> 4; break;
            default: result += i; break;
        }
        
        /* Nested loop with break/continue for additional CFG complexity */
        for (int j = 0; j < 5; j++) {
            if (j == 2) continue;
            if (j == 4) break;
            result += j;
        }
    }
    
    asm volatile ("" : : "r"(result) : "memory");
    return result;
}

/* Pattern D: Vector operations using GCC vector extensions
 * Pressures SIMD/vector registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static v4si pattern_d_vector_ops(int iterations, v4si seed) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si result = {0, 0, 0, 0};
    
    v1 = seed;
    
    for (int i = 0; i < iterations; i++) {
        v2 = v1 + (v4si){i, i+1, i+2, i+3};
        v3 = v2 * v1;
        v4 = v3 - v2;
        v5 = v4 & v3;
        v6 = v5 | v4;
        v7 = v6 << 1;
        v8 = v7 >> 1;
        v9 = v8 + v7;
        v10 = v9 - v8;
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        v1 = v10 ^ (v4si){i, i, i, i};
    }
    
    asm volatile ("" : : "r"(result) : "memory");
    return result;
}

/* Pattern E: Explicit register variables conflicting with allocator
 * Uses explicit register assignments to create conflicts */
NOINLINE static int pattern_e_register_conflict(int iterations, int seed) {
    /* Try to use specific registers that might conflict with allocator */
    register int r1 asm ("r12") = seed;
    register int r2 asm ("r13") = seed + 1;
    register int r3 asm ("r14") = seed + 2;
    register int r4 asm ("r15") = seed + 3;
    
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Operations using both register variables and regular variables */
        temp1 = r1 * i;
        temp2 = r2 + temp1;
        temp3 = r3 - temp2;
        temp4 = r4 ^ temp3;
        
        temp5 = temp1 & temp2;
        temp6 = temp3 | temp4;
        temp7 = temp5 << (i % 4);
        temp8 = temp6 >> (i % 4);
        
        sum += r1 + r2 + r3 + r4 + temp1 + temp2 + temp3 + temp4 + 
               temp5 + temp6 + temp7 + temp8;
        
        /* Update register variables */
        r1 = temp8 + 1;
        r2 = temp7 - 1;
        r3 = r1 ^ r2;
        r4 = r3 * 2;
    }
    
    asm volatile ("" : : "r"(sum), "r"(r1), "r"(r2), "r"(r3), "r"(r4) : "memory");
    return sum;
}

/* Pattern F: Mixed integer/float with complex loop structure
 * Additional pressure pattern */
NOINLINE static double pattern_f_mixed_types(int iterations, int seed) {
    int i1 = seed, i2 = seed + 1, i3 = seed + 2, i4 = seed + 3;
    double f1 = seed * 1.5, f2 = seed * 2.5, f3 = seed * 3.5, f4 = seed * 4.5;
    double total = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Integer operations */
        i1 = i2 * i3 + i;
        i2 = i3 ^ i4;
        i3 = i4 | i1;
        i4 = i1 & i2;
        
        /* Floating-point operations */
        f1 = f2 * 1.1 + i1;
        f2 = f3 / 1.3 - i2;
        f3 = f4 * f1 + i3;
        f4 = f1 / (f2 != 0.0 ? f2 : 1.0) + i4;
        
        /* Type conversions and mixing */
        total += (double)i1 + (double)i2 + (double)i3 + (double)i4 +
                 f1 + f2 + f3 + f4;
        
        /* Complex loop control with early exit */
        if (i % 7 == 0) {
            total *= 1.01;
        }
        if (i % 13 == 0) {
            continue;
        }
        if (i % 17 == 0 && i > iterations / 2) {
            break;
        }
    }
    
    asm volatile ("" : : "r"(total) : "memory");
    return total;
}

/* Main function marked as cold to potentially affect block ordering */
COLD int main(int argc, char *argv[]) {
    int iterations = 1000;
    long total = 0;
    
    /* Use varying inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    
    /* Force CPU feature detection to engage target-specific optimizations */
    #ifdef __GNUC__
    if (__builtin_cpu_supports("sse2") ||
        __builtin_cpu_supports("avx") ||
        __builtin_cpu_supports("avx2")) {
        /* This ensures the compilation pipeline includes target-specific
         * optimizations that interact with register allocation */
    }
    #endif
    
    printf("Starting MCF stress test...\n");
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < num_inputs; i++) {
        int seed = inputs[i];
        
        /* Pattern A: Integer pressure */
        total += pattern_a_intensive(iterations / (seed + 1), seed);
        
        /* Pattern B: Floating-point pressure */
        total += (long)pattern_b_float_pressure(iterations / (seed + 2), seed * 1.5);
        
        /* Pattern C: Complex CFG */
        total += pattern_c_complex_cfg(seed, iterations / 10);
        
        /* Pattern D: Vector operations */
        v4si vec_seed = {seed, seed + 1, seed + 2, seed + 3};
        v4si vec_result = pattern_d_vector_ops(iterations / 20, vec_seed);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        /* Pattern E: Register conflict */
        total += pattern_e_register_conflict(iterations / 5, seed);
        
        /* Pattern F: Mixed types */
        total += (long)pattern_f_mixed_types(iterations / 3, seed);
    }
    
    printf("Total result: %ld\n", total);
    
    /* Additional compilation trick: Use computed goto for extra CFG complexity */
    #ifdef __GNUC__
    if (total > 0) {
        static void *labels[] = {&&label1, &&label2, &&label3, &&label4, &&label5};
        int idx = total % 5;
        
        goto *labels[idx];
        
        label1:
            total += 1;
            goto end;
        label2:
            total += 2;
            goto end;
        label3:
            total += 3;
            goto end;
        label4:
            total += 4;
            goto end;
        label5:
            total += 5;
            goto end;
        end:;
    }
    #endif
    
    return (int)(total % 1000);
}
