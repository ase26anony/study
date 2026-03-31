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

/* Prevent inlining to ensure each function is compiled separately */
#define NOINLINE __attribute__((noinline))
#define COLD __attribute__((cold))

/* Pattern A: Integer arithmetic chain with 30+ variables in a loop */
NOINLINE static int integer_pressure(int input) {
    /* 30+ integer variables to create register pressure */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    int d1, d2, d3, d4, d5;
    
    /* Complex interdependent calculations */
    a1 = input + 1;
    a2 = a1 * 2;
    a3 = a2 - input;
    a4 = a3 ^ a1;
    a5 = a4 | a2;
    a6 = a5 & a3;
    a7 = a6 << 2;
    a8 = a7 >> 1;
    a9 = a8 + a1;
    a10 = a9 - a2;
    
    b1 = a10 * 3;
    b2 = b1 / 2;
    b3 = b2 % 7;
    b4 = b3 ^ b1;
    b5 = b4 | b2;
    b6 = b5 & b3;
    b7 = b6 << 3;
    b8 = b7 >> 2;
    b9 = b8 + b1;
    b10 = b9 - b2;
    
    c1 = b10 * 5;
    c2 = c1 / 3;
    c3 = c2 % 11;
    c4 = c3 ^ c1;
    c5 = c4 | c2;
    c6 = c5 & c3;
    c7 = c6 << 1;
    c8 = c7 >> 1;
    c9 = c8 + c1;
    c10 = c9 - c2;
    
    d1 = c10 * 7;
    d2 = d1 / 5;
    d3 = d2 % 13;
    d4 = d3 ^ d1;
    d5 = d4 | d2;
    
    /* Loop with all variables to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        a1 += i; a2 += i; a3 += i; a4 += i; a5 += i;
        a6 += i; a7 += i; a8 += i; a9 += i; a10 += i;
        b1 += i; b2 += i; b3 += i; b4 += i; b5 += i;
        b6 += i; b7 += i; b8 += i; b9 += i; b10 += i;
        c1 += i; c2 += i; c3 += i; c4 += i; c5 += i;
        c6 += i; c7 += i; c8 += i; c9 += i; c10 += i;
        d1 += i; d2 += i; d3 += i; d4 += i; d5 += i;
        
        /* Prevent dead code elimination */
        asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5));
        asm volatile("" : "+r"(a6), "+r"(a7), "+r"(a8), "+r"(a9), "+r"(a10));
    }
    
    /* Complex return calculation using all variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
           c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 +
           d1 + d2 + d3 + d4 + d5;
}

/* Pattern B: Floating-point intensive computation */
NOINLINE static double float_pressure(double input) {
    /* 20+ double variables */
    double f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double g1, g2, g3, g4, g5, g6, g7, g8, g9, g10;
    
    f1 = input + 1.0;
    f2 = f1 * 2.0;
    f3 = f2 - input;
    f4 = f3 / f1;
    f5 = f4 * f2;
    f6 = f5 - f3;
    f7 = f6 / f4;
    f8 = f7 * f5;
    f9 = f8 - f6;
    f10 = f9 / f7;
    
    g1 = f10 * 3.14159;
    g2 = g1 / 2.71828;
    g3 = g2 - g1;
    g4 = g3 * g2;
    g5 = g4 / g3;
    g6 = g5 - g4;
    g7 = g6 * g5;
    g8 = g7 / g6;
    g9 = g8 - g7;
    g10 = g9 * g8;
    
    /* Nested loops with break/continue to create complex CFG */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        if (i % 7 == 0) continue;
        
        for (int j = 0; j < 20; j++) {
            if (j == 15) break;
            
            f1 += 0.1 * i;
            f2 -= 0.2 * j;
            f3 *= 1.01;
            f4 /= 1.02;
            f5 = f1 + f2;
            f6 = f3 - f4;
            f7 = f5 * f6;
            f8 = f7 / (j + 1);
            f9 = f8 + f1;
            f10 = f9 - f2;
            
            g1 = f10 * i;
            g2 = g1 / (j + 2);
            g3 = g2 + g1;
            g4 = g3 - g2;
            g5 = g4 * g3;
            g6 = g5 / g4;
            g7 = g6 + g5;
            g8 = g7 - g6;
            g9 = g8 * g7;
            g10 = g9 / g8;
            
            result += g10;
        }
        
        if (i % 13 == 0) {
            /* Early exit from outer loop */
            result *= 1.5;
            if (result > 1000.0) break;
        }
    }
    
    return result;
}

/* Pattern C: Complex switch statement with many cases */
NOINLINE static int switch_pressure(int input) {
    int result = input;
    
    /* Switch with many cases creating many basic blocks */
    switch (input % 23) {
        case 0:
            result = result * 2 + 1;
            break;
        case 1:
            result = result / 3 | 0xFF;
            break;
        case 2:
            result = result ^ 0xAAAA;
            break;
        case 3:
            result = result << 4;
            break;
        case 4:
            result = result >> 2;
            break;
        case 5:
            result = ~result;
            break;
        case 6:
            result = result + 0x1234;
            break;
        case 7:
            result = result - 0x5678;
            break;
        case 8:
            result = result * result;
            break;
        case 9:
            result = result % 17;
            break;
        case 10:
            result = result & 0x5555;
            break;
        case 11:
            result = result | 0xAAAA;
            break;
        case 12:
            result = result ^ result;
            break;
        case 13:
            result = result << 1;
            break;
        case 14:
            result = result >> 3;
            break;
        case 15:
            result = -result;
            break;
        case 16:
            result = result + result;
            break;
        case 17:
            result = result * 3;
            break;
        case 18:
            result = result / 2;
            break;
        case 19:
            result = result % 19;
            break;
        case 20:
            result = result & 0x3333;
            break;
        case 21:
            result = result | 0xCCCC;
            break;
        case 22:
            result = result ^ 0x9999;
            break;
        default:
            result = 0;
    }
    
    /* Second switch inside a loop */
    for (int i = 0; i < 10; i++) {
        switch ((result + i) % 7) {
            case 0: result += i * 2; break;
            case 1: result -= i * 3; break;
            case 2: result ^= i; break;
            case 3: result |= i << 4; break;
            case 4: result &= ~i; break;
            case 5: result = result << (i % 4); break;
            case 6: result = result >> (i % 4); break;
        }
    }
    
    return result;
}

/* Pattern D: Vector extensions for SIMD register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

NOINLINE static int vector_pressure(int input) {
#ifdef __GNUC__
    /* Multiple vector variables */
    v4si v1 = {input, input + 1, input + 2, input + 3};
    v4si v2 = {input + 4, input + 5, input + 6, input + 7};
    v4si v3 = {input + 8, input + 9, input + 10, input + 11};
    v4si v4 = {input + 12, input + 13, input + 14, input + 15};
    v4si v5 = {input + 16, input + 17, input + 18, input + 19};
    
    v4sf fv1 = {input * 1.0f, input * 2.0f, input * 3.0f, input * 4.0f};
    v4sf fv2 = {input * 5.0f, input * 6.0f, input * 7.0f, input * 8.0f};
    
    v2df dv1 = {input * 1.0, input * 2.0};
    v2df dv2 = {input * 3.0, input * 4.0};
    
    /* Vector operations in a loop */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        v1 = v1 + v2;
        v2 = v2 - v3;
        v3 = v3 * v4;
        v4 = v4 & v5;
        v5 = v5 | v1;
        
        fv1 = fv1 + fv2;
        fv2 = fv2 * fv1;
        
        dv1 = dv1 + dv2;
        dv2 = dv2 * dv1;
        
        /* Extract and sum elements */
        int v1_arr[4];
        __builtin_memcpy(v1_arr, &v1, sizeof(v1));
        sum += v1_arr[0] + v1_arr[1] + v1_arr[2] + v1_arr[3];
        
        /* Prevent optimization */
        asm volatile("" : "+x"(v1), "+x"(v2), "+x"(v3), "+x"(v4), "+x"(v5));
        asm volatile("" : "+x"(fv1), "+x"(fv2));
        asm volatile("" : "+x"(dv1), "+x"(dv2));
    }
    
    return sum;
#else
    return input;  /* Fallback for non-GCC */
#endif
}

/* Pattern E: Explicit register variables to conflict with allocator */
NOINLINE static int register_conflict(int input) {
    /* Try to use specific registers */
    register int r1 asm ("r12") = input;
    register int r2 asm ("r13") = input + 1;
    register int r3 asm ("r14") = input + 2;
    register int r4 asm ("r15") = input + 3;
    
    int local1, local2, local3, local4, local5;
    int local6, local7, local8, local9, local10;
    
    /* Mix register variables with locals */
    local1 = r1 * 2;
    local2 = r2 + local1;
    local3 = r3 - local2;
    local4 = r4 ^ local3;
    local5 = local1 | local4;
    
    local6 = local2 & local5;
    local7 = local3 << 2;
    local8 = local4 >> 1;
    local9 = local5 + local6;
    local10 = local7 - local8;
    
    /* Computed goto (GCC extension) for complex control flow */
    void* labels[] = {&&label0, &&label1, &&label2, &&label3, &&label4,
                      &&label5, &&label6, &&label7, &&label8, &&label9};
    
    int goto_target = input % 10;
    goto *labels[goto_target];
    
label0:
    r1 += local9;
    goto end;
label1:
    r2 -= local10;
    goto end;
label2:
    r3 *= local1;
    goto end;
label3:
    r4 /= (local2 + 1);
    goto end;
label4:
    r1 = r1 ^ r2;
    goto end;
label5:
    r2 = r2 | r3;
    goto end;
label6:
    r3 = r3 & r4;
    goto end;
label7:
    r4 = r4 << 1;
    goto end;
label8:
    r1 = r1 >> 2;
    goto end;
label9:
    r2 = ~r2;
    goto end;
    
end:
    /* Force all variables to be live */
    asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
    asm volatile("" : "+r"(local1), "+r"(local2), "+r"(local3), "+r"(local4));
    asm volatile("" : "+r"(local5), "+r"(local6), "+r"(local7), "+r"(local8));
    asm volatile("" : "+r"(local9), "+r"(local10));
    
    return r1 + r2 + r3 + r4 + local1 + local2 + local3 + local4 + local5 +
           local6 + local7 + local8 + local9 + local10;
}

/* Main function that calls all pressure functions */
COLD int main(int argc, char** argv) {
    int result = 0;
    
    /* Use CPU feature detection to engage target-specific optimizations */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        /* This may affect register allocation strategy */
        result += 1000;
    }
#endif
    
    /* Call pressure functions with varying inputs */
    int inputs[] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23};
    
    for (int i = 0; i < 10; i++) {
        result += integer_pressure(inputs[i]);
        result += (int)float_pressure(inputs[i] * 1.0);
        result += switch_pressure(inputs[i]);
        result += vector_pressure(inputs[i]);
        result += register_conflict(inputs[i]);
        
        /* Prevent optimization across iterations */
        asm volatile("" : "+r"(result));
    }
    
    /* Optional debug output */
    printf("Final result: %d\n", result);
    
    return result % 256;  /* Return non-zero to indicate execution */
}
