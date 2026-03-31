/* caller-save-test.c
 * Designed to trigger uncovered lines in caller-save.cc (lines 905-913)
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -lm
 */

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
    
    /* Mix in some external function calls */
    double sin_val = sin(a + b);
    double pow_val = pow(g, 2.0);
    
    /* Inline assembly that clobbers caller-saved registers */
    int temp_e = e;
    int temp_f = f;
    __asm__ volatile (
        "addl %1, %0\n\t"
        "imull $7, %0\n\t"
        : "+r"(temp_e)
        : "r"(temp_f)
        : "%eax", "%ecx", "%edx", "cc"
    );
    e = temp_e;
    
    /* Call printf with some variables */
    printf("Depth %d: sin=%.3f pow=%.3f\n", depth, sin_val, pow_val);
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           a * 0.9, b * 1.1, c * 1.05f, d * 0.95f,
                           e + 1, f - 1, g * sin_val, h * 1.2f, i * 3,
                           j + pow_val, k * 0.8f, l / 2, m * 0.99, n % 100);
}

int main() {
    /* Declare and initialize many local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 4; outer++) {
        /* Complex arithmetic on all variables before function calls */
        v1 = v1 * 3 + v2;
        v2 = v2 - v3 * 2;
        v3 = v3 + v4 / 2;
        v4 = v4 ^ v5;
        v5 = v5 * 7 - v1;
        
        f1 = f1 * 1.5f + f2;
        f2 = f2 - f3 * 0.5f;
        f3 = f3 + f4 / 2.0f;
        f4 = f4 * 3.14f;
        f5 = f5 - f1 * 0.1f;
        
        d1 = d1 * 2.0 + d2;
        d2 = d2 - d3 * 0.5;
        d3 = d3 + d4 / 3.0;
        d4 = d4 * 1.618;
        d5 = d5 - d1 * 0.2;
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Call external functions */
            double sin_result = sin(d1 + d2);
            printf("sin(%.3f + %.3f) = %.3f\n", d1, d2, sin_result);
            
            /* More arithmetic to keep variables live */
            v1 += (int)(sin_result * 100);
            f1 += (float)sin_result;
            
            /* Call to pow with live variables */
            double pow_result = pow(d3, d4);
            printf("pow(%.3f, %.3f) = %.3f\n", d3, d4, pow_result);
            
            /* Inline assembly clobbering caller-saved registers */
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "addl %2, %%eax\n\t"
                "imull $13, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r"(v3)
                : "r"(v4), "r"(v5)
                : "%eax", "%ecx", "%edx", "cc"
            );
            
            /* Call at the end of basic block before goto */
            int rand_val = rand() % 100;
            printf("Random value: %d\n", rand_val);
            v2 += rand_val;
            
            goto alternate_path;
        } else {
alternate_path:
            /* Path 2: Different function call pattern */
            /* Call external function with many arguments */
            double cos_result = cos(d5);
            printf("cos(%.3f) = %.3f\n", d5, cos_result);
            
            /* Inline assembly with different clobbered registers */
            float temp_f2 = f2;
            __asm__ volatile (
                "addss %1, %0\n\t"
                "mulss %2, %0\n\t"
                : "+x"(temp_f2)
                : "x"(f3), "x"(f4)
                : "cc"
            );
            f2 = temp_f2;
            
            /* Call recursive helper with many live variables */
            double rec_result = recursive_helper(
                2,  /* depth */
                d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, v5
            );
            printf("Recursive result: %.3f\n", rec_result);
            
            /* Function call at block end (before label) */
            float fabs_result = fabsf(f5);
            printf("fabs(%.3f) = %.3f\n", f5, fabs_result);
            f5 = fabs_result;
            
            end_of_block:
            /* Empty label to create block boundary */
            ;
        }
        
        /* More arithmetic after calls */
        v1 = v1 * 2 - v3;
        v2 = v2 + v4 / 3;
        v3 = v3 ^ v5;
        v4 = v4 * 11 + v2;
        v5 = v5 - v1 * 2;
        
        f1 = f1 + f3 * 0.25f;
        f2 = f2 - f4 / 1.5f;
        f3 = f3 * f5;
        f4 = f4 + f1 * 0.33f;
        f5 = f5 - f2 * 0.75f;
        
        d1 = d1 + d3 * 0.3;
        d2 = d2 - d4 / 2.5;
        d3 = d3 * d5;
        d4 = d4 + d1 * 0.15;
        d5 = d5 - d2 * 0.6;
        
        /* Another function call */
        double log_result = log(fabs(d1) + 1.0);
        printf("log(%.3f) = %.3f\n", fabs(d1) + 1.0, log_result);
        
        /* Another conditional with call at block end */
        if (v1 > 1000) {
            printf("v1 is large: %d\n", v1);
            /* Call at the end of this basic block */
            double exp_result = exp(d2);
            printf("exp(%.3f) = %.3f\n", d2, exp_result);
            goto end_of_block;
        }
    }
    
    /* Compute final checksum from all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    printf("Final checksum: %.6f\n", checksum);
    
    /* One more external call */
    int final_rand = rand() % 1000;
    printf("Final random: %d\n", final_rand);
    
    return (int)(checksum + final_rand) % 256;
}
