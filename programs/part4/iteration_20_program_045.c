/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger GCC's Min-Cost Flow pass
 * during register allocation, specifically to cover the special node
 * printing logic in mcf.cc's print_node function.
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

/* Vector type for SIMD pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE int pattern_a(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
    int a21, a22, a23, a24, a25, a26, a27, a28, a29, a30;
    int a31, a32, a33, a34, a35, a36, a37, a38, a39, a40;
    
    /* Initialize with input to prevent constant folding */
    a1 = input;
    a2 = a1 + 1;
    a3 = a2 * a1;
    a4 = a3 - a2;
    a5 = a4 ^ a3;
    a6 = a5 | a4;
    a7 = a6 & a5;
    a8 = a7 << 2;
    a9 = a8 >> 1;
    a10 = a9 + a8;
    
    a11 = a10 * 3;
    a12 = a11 - a10;
    a13 = a12 ^ a11;
    a14 = a13 | a12;
    a15 = a14 & a13;
    a16 = a15 << 3;
    a17 = a16 >> 2;
    a18 = a17 + a16;
    a19 = a18 * 5;
    a20 = a19 - a18;
    
    a21 = a20 ^ a19;
    a22 = a21 | a20;
    a23 = a22 & a21;
    a24 = a23 << 4;
    a25 = a24 >> 3;
    a26 = a25 + a24;
    a27 = a26 * 7;
    a28 = a27 - a26;
    a29 = a28 ^ a27;
    a30 = a29 | a28;
    
    a31 = a30 & a29;
    a32 = a31 << 5;
    a33 = a32 >> 4;
    a34 = a33 + a32;
    a35 = a34 * 11;
    a36 = a35 - a34;
    a37 = a36 ^ a35;
    a38 = a37 | a36;
    a39 = a38 & a37;
    a40 = a39 << 6;
    
    /* Complex loop with interdependencies */
    for (int i = 0; i < 100; i++) {
        a1 = a2 + a3;
        a2 = a3 * a4;
        a3 = a4 - a5;
        a4 = a5 ^ a6;
        a5 = a6 | a7;
        a6 = a7 & a8;
        a7 = a8 << (i & 3);
        a8 = a9 >> 1;
        a9 = a10 + a11;
        a10 = a11 * a12;
        
        a11 = a12 - a13;
        a12 = a13 ^ a14;
        a13 = a14 | a15;
        a14 = a15 & a16;
        a15 = a16 << 2;
        a16 = a17 >> (i & 1);
        a17 = a18 + a19;
        a18 = a19 * a20;
        a19 = a20 - a21;
        a20 = a21 ^ a22;
        
        /* Use asm to prevent optimization */
        asm volatile ("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
        asm volatile ("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
    }
    
    /* Return a complex expression to prevent dead code elimination */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 +
           a21 + a22 + a23 + a24 + a25 + a26 + a27 + a28 + a29 + a30 +
           a31 + a32 + a33 + a34 + a35 + a36 + a37 + a38 + a39 + a40;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE double pattern_b(double input) {
    /* 20+ double variables to pressure FP registers */
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    double b11, b12, b13, b14, b15, b16, b17, b18, b19, b20;
    double b21, b22, b23, b24, b25;
    
    b1 = input;
    b2 = b1 * 1.1;
    b3 = b2 + 2.2;
    b4 = b3 / 1.5;
    b5 = b4 - 0.5;
    b6 = b5 * 3.14159;
    b7 = b6 / 2.71828;
    b8 = b7 + b6;
    b9 = b8 * b7;
    b10 = b9 - b8;
    
    b11 = b10 * 1.618;  /* golden ratio */
    b12 = b11 / 0.618;
    b13 = b12 + b11;
    b14 = b13 * 2.0;
    b15 = b14 - 1.0;
    b16 = b15 * 0.5;
    b17 = b16 + 0.25;
    b18 = b17 * 4.0;
    b19 = b18 / 3.0;
    b20 = b19 - 2.0;
    
    b21 = b20 * 1.5;
    b22 = b21 + 0.75;
    b23 = b22 / 1.25;
    b24 = b23 * 0.8;
    b25 = b24 - 0.4;
    
    /* Nested loops with floating operations */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 10; j++) {
            b1 = b2 * b3;
            b2 = b3 + b4;
            b3 = b4 - b5;
            b4 = b5 / (j + 1);
            b5 = b6 * (i + 1);
            
            b6 = b7 + b8;
            b7 = b8 - b9;
            b8 = b9 * 1.1;
            b9 = b10 / 1.2;
            b10 = b11 + 0.1;
            
            /* Mix integer index into FP calculations */
            b11 = b12 * (i * 0.01);
            b12 = b13 + (j * 0.001);
            b13 = b14 - 0.0001;
            b14 = b15 / 1.0001;
            b15 = b16 * 0.9999;
        }
        
        /* Conditional break to create complex CFG */
        if (i > 25 && b1 > 1000.0) {
            break;
        }
        
        /* Continue with different path */
        if (i % 7 == 0) {
            continue;
        }
        
        b16 = b17 * 1.7;
        b17 = b18 + 1.8;
        b18 = b19 - 1.9;
        b19 = b20 / 2.0;
        b20 = b21 * 2.1;
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5));
    
    return b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           b11 + b12 + b13 + b14 + b15 + b16 + b17 + b18 + b19 + b20 +
           b21 + b22 + b23 + b24 + b25;
}

/* Pattern C: Complex control flow with switch statement */
NOINLINE int pattern_c(int input) {
    int result = input;
    
    /* Switch with many cases to create many basic blocks */
    for (int i = 0; i < 100; i++) {
        switch (i % 23) {  /* Prime number to avoid patterns */
            case 0:  result += 1; break;
            case 1:  result *= 2; break;
            case 2:  result ^= 0x55; break;
            case 3:  result |= 0xAA; break;
            case 4:  result &= 0xF0; break;
            case 5:  result <<= 1; break;
            case 6:  result >>= 2; break;
            case 7:  result += i; break;
            case 8:  result *= i; break;
            case 9:  result ^= i; break;
            case 10: result |= i; break;
            case 11: result &= i; break;
            case 12: result <<= (i & 3); break;
            case 13: result >>= (i & 2); break;
            case 14: result += result; break;
            case 15: result *= result; break;
            case 16: result ^= result; break;
            case 17: result |= 0xFF; break;
            case 18: result &= 0x0F; break;
            case 19: result = ~result; break;
            case 20: result = -result; break;
            case 21: result = abs(result); break;
            case 22: result = result % 256; break;
            default: result += 999; break;
        }
        
        /* Nested conditional to add more CFG edges */
        if (i % 5 == 0) {
            if (result > 1000) {
                result -= 500;
            } else if (result < -1000) {
                result += 500;
            } else {
                result *= 2;
            }
        }
        
        /* Loop with break/continue */
        for (int j = 0; j < 5; j++) {
            if (j == 2 && i > 50) {
                break;
            }
            if (j == 3) {
                continue;
            }
            result += j;
        }
    }
    
    return result;
}

/* Pattern D: SIMD vector operations */
NOINLINE v4si pattern_d(v4si input) {
    v4si v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    v4si v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    v1 = input;
    v2 = v1 + (v4si){1, 2, 3, 4};
    v3 = v2 * (v4si){2, 2, 2, 2};
    v4 = v3 - v2;
    v5 = v4 & (v4si){0xFF, 0xFF, 0xFF, 0xFF};
    v6 = v5 | (v4si){0xAA, 0xAA, 0xAA, 0xAA};
    v7 = v6 ^ (v4si){0x55, 0x55, 0x55, 0x55};
    v8 = v7 << 1;
    v9 = v8 >> 2;
    v10 = v9 + v8;
    
    v11 = v10 * (v4si){3, 3, 3, 3};
    v12 = v11 - v10;
    v13 = v12 & v11;
    v14 = v13 | v12;
    v15 = v14 ^ v13;
    v16 = v15 << (v4si){1, 2, 1, 2};
    v17 = v16 >> (v4si){2, 1, 2, 1};
    v18 = v17 + v16;
    v19 = v18 * (v4si){5, 5, 5, 5};
    v20 = v19 - v18;
    
    /* Loop with vector operations */
    for (int i = 0; i < 50; i++) {
        v1 = v2 + v3;
        v2 = v3 * (v4si){i, i+1, i+2, i+3};
        v3 = v4 - v5;
        v4 = v5 & (v4si){i, i, i, i};
        v5 = v6 | v7;
        v6 = v7 ^ v8;
        v7 = v8 << (i & 3);
        v8 = v9 >> 1;
        v9 = v10 + v11;
        v10 = v11 * v12;
        
        /* Conditional to create CFG complexity */
        if (i % 7 == 0) {
            v11 = v12 - v13;
            v12 = v13 & v14;
        } else if (i % 3 == 0) {
            v13 = v14 | v15;
            v14 = v15 ^ v16;
        } else {
            v15 = v16 + v17;
            v16 = v17 * v18;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE int pattern_e(int input) {
    /* Explicit register variables that may conflict with allocator */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = r1 + 1;
    register int r3 asm ("r14") = r2 * 2;
    register int r4 asm ("r15") = r3 - 1;
    
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Mix register variables with regular variables */
    v1 = r1;
    v2 = r2;
    v3 = r3;
    v4 = r4;
    
    v5 = v1 + v2;
    v6 = v2 * v3;
    v7 = v3 - v4;
    v8 = v4 ^ v1;
    v9 = v5 | v6;
    v10 = v6 & v7;
    
    v11 = v7 << v8;
    v12 = v8 >> 1;
    v13 = v9 + v10;
    v14 = v10 * v11;
    v15 = v11 - v12;
    v16 = v12 ^ v13;
    v17 = v13 | v14;
    v18 = v14 & v15;
    v19 = v15 << 2;
    v20 = v16 >> 3;
    
    /* Complex loop mixing register and regular vars */
    for (int i = 0; i < 100; i++) {
        /* Force spills by using many variables */
        r1 = v1 + i;
        r2 = v2 * i;
        r3 = v3 - i;
        r4 = v4 ^ i;
        
        v1 = r1 + r2;
        v2 = r2 * r3;
        v3 = r3 - r4;
        v4 = r4 ^ r1;
        
        v5 = v1 + v2;
        v6 = v2 * v3;
        v7 = v3 - v4;
        v8 = v4 ^ v1;
        
        v9 = v5 | v6;
        v10 = v6 & v7;
        v11 = v7 << (i & 3);
        v12 = v8 >> 1;
        
        /* Use computed goto (GCC extension) for complex CFG */
        void* labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4, &&L5};
        goto *labels[i % 6];
        
        L0: v13 = v9 + 1; goto end;
        L1: v13 = v10 * 2; goto end;
        L2: v13 = v11 - 3; goto end;
        L3: v13 = v12 ^ 4; goto end;
        L4: v13 = v9 | v10; goto end;
        L5: v13 = v10 & v11; goto end;
        end:;
        
        v14 = v13 + v12;
        v15 = v14 * v13;
        v16 = v15 - v14;
        v17 = v16 ^ v15;
        v18 = v17 | v16;
        v19 = v18 & v17;
        v20 = v19 << 1;
    }
    
    /* Force use of all variables */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    return r1 + r2 + r3 + r4 + v1 + v2 + v3 + v4 + v5 + v6 +
           v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
           v16 + v17 + v18 + v19 + v20;
}

/* Main function that calls all patterns */
COLD int main(int argc, char** argv) {
    int result = 0;
    
    /* Use CPU feature check to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation decisions */
        result += 1;
    }
#endif
    
    /* Array of inputs to prevent constant folding */
    int inputs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    double dinputs[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    v4si vinput = {1, 2, 3, 4};
    
    /* Call each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result += pattern_a(inputs[i % 10]);
        result += pattern_b(dinputs[i % 5]);
        result += pattern_c(inputs[i % 10]);
        
        v4si vresult = pattern_d(vinput);
        /* Extract elements from vector */
        int* vptr = (int*)&vresult;
        result += vptr[0] + vptr[1] + vptr[2] + vptr[3];
        
        result += pattern_e(inputs[i % 10]);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result % 256;  /* Return non-zero to indicate execution */
}
