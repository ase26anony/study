#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to create register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                       int e, int f, double g, float h, int i, 
                       double j, float k, int l, double m, int n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * j + k * l;
    
    /* Function call within recursion to create more caller-save opportunities */
    double sin_val = sin(t1);
    printf("Recursive depth %d: sin(%f) = %f\n", depth, t1, sin_val);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "add %1, %0\n\t"
        "imul %2, %3\n\t"
        : "+r"(e), "+r"(f)
        : "r"(g), "r"(h)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    return recursive_helper(depth - 1, 
                           b, t1, d, t2,
                           f, t3, h, j,
                           l, m, k, n,
                           sin_val, depth) * 0.9;
}

int main() {
    srand(time(NULL));
    
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    /* Additional variables to increase pressure */
    int v6 = 6, v7 = 7;
    float f6 = 6.6f;
    double d6 = 6.66, d7 = 7.77;
    
    double checksum = 0.0;
    int iterations = 4;
    
    /* Label for goto to create unusual basic block structure */
    loop_start:
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic to create long live ranges */
        v1 = v2 * v3 + v4 - v5;
        v2 = v3 / (v1 + 1) * v4;
        v3 = v4 ^ v5 | v1;
        v4 = v5 + v1 * v2;
        v5 = v1 - v2 + v3 * v4;
        v6 = v7 * i + v1;
        v7 = v6 ^ (v2 + i);
        
        f1 = f2 * 1.5f + f3;
        f2 = f3 / 2.0f - f4;
        f3 = f4 * f5 + f1;
        f4 = f5 - f2 * f3;
        f5 = f6 + f1 * 0.7f;
        f6 = f2 * 3.14f - f4;
        
        d1 = d2 * 3.14159 + d3;
        d2 = d3 / 2.71828 - d4;
        d3 = d4 * d5 + sin(d1);
        d4 = d5 - d2 * cos(d3);
        d5 = d6 + d1 * 0.333;
        d6 = d7 * 1.414 - d2;
        d7 = d3 * 2.718 + d4;
        
        /* Conditional branch with function calls in both paths */
        if (i % 2 == 0) {
            /* Path 1: Multiple function calls with live variables */
            double pow_result = pow(d1, d2);
            printf("Iteration %d, even: pow(%f, %f) = %f\n", i, d1, d2, pow_result);
            
            /* Call with many arguments to pressure caller-saved regs */
            checksum += recursive_helper(2, d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, v5);
            
            /* Inline assembly clobbering multiple caller-saved registers */
            asm volatile (
                "mov %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "=r"(v1)
                : "r"(v2), "r"(v3)
                : "%eax", "%ecx", "%edx", "memory"
            );
            
            /* Function call at the end of basic block (before goto) */
            float sinf_result = sinf(f1);
            checksum += sinf_result;
            
            /* This call is at block end - may trigger BB_END logic */
            printf("sinf(%f) = %f\n", f1, sinf_result);
            goto after_condition;  /* Creates block boundary after call */
        } else {
            /* Path 2: Different pattern of function calls */
            double log_result = log(fabs(d3) + 1.0);
            printf("Iteration %d, odd: log(%f) = %f\n", i, d3, log_result);
            
            /* Another recursive call with different arguments */
            checksum += recursive_helper(1, d4, d5, f5, f6, v5, v6, d6, f3, v7, d7, f2, v3, d1, v4);
            
            /* Different inline assembly pattern */
            asm volatile (
                "imul %1, %0\n\t"
                "add $1, %0\n\t"
                : "+r"(v2)
                : "r"(v3)
                : "%eax", "%ecx", "memory"
            );
            
            /* Call to external function */
            int rand_val = rand();
            checksum += rand_val % 100;
            printf("Random value: %d\n", rand_val);
            
            /* Another call at block end */
            double cos_result = cos(d2);
            checksum += cos_result;
        }
        
        after_condition:
        
        /* More operations after conditional to extend live ranges */
        v1 = v1 + v7 * i;
        v2 = v2 ^ (v6 + i);
        f1 = f1 * 1.1f + sinf(f2);
        f2 = f2 / 1.2f - cosf(f3);
        d1 = d1 * 1.01 + sin(d2);
        d2 = d2 / 1.02 - cos(d3);
        
        /* Another function call in the middle of operations */
        double sqrt_result = sqrt(d4 * d4 + d5 * d5);
        checksum += sqrt_result;
        
        /* Mix variables in function arguments */
        printf("Mixed args: %d, %f, %f, %d, %f\n", v1, f1, d1, v2, f2);
        
        /* Additional inline assembly */
        asm volatile (
            "mov %1, %%ecx\n\t"
            "lea (%%ecx, %2, 2), %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r"(v3)
            : "r"(v4), "r"(v5)
            : "%eax", "%ecx", "memory"
        );
    }
    
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7;
    checksum += f1 + f2 + f3 + f4 + f5 + f6;
    checksum += d1 + d2 + d3 + d4 + d5 + d6 + d7;
    
    /* One more function call before return */
    printf("Final checksum: %f\n", checksum);
    
    /* Use goto to create another basic block boundary */
    if (checksum > 1000.0) {
        goto final_print;
    } else {
        /* Function call at end of this basic block */
        printf("Checksum is small: %f\n", checksum);
        goto final_print;
    }
    
    /* Dead code to create more block boundaries */
    v1 = rand();
    
    final_print:
    /* Final output */
    printf("Program completed. Final values:\n");
    printf("ints: %d %d %d %d %d %d %d\n", v1, v2, v3, v4, v5, v6, v7);
    printf("floats: %f %f %f %f %f %f\n", f1, f2, f3, f4, f5, f6);
    printf("doubles: %f %f %f %f %f %f %f\n", d1, d2, d3, d4, d5, d6, d7);
    
    return (int)(checksum * 1000) % 256;
}
