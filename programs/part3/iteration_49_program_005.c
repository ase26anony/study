#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, float c, float d, 
                       int e, int f, double g, float h, int i, 
                       double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * j + k * l;
    
    /* Function call within recursion */
    printf("Depth %d: t1=%.3f t2=%.3f t3=%d\n", depth, t1, t2, t3);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        : "+r"(e), "+r"(f)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    
    return recursive_helper(depth - 1, 
                           a + 1.1, b * 0.9, c + 0.5f, d * 0.8f,
                           e + 2, f - 1, g * 1.1, h / 2.0f, i * 3,
                           j - 0.5, k + 0.3f, l / 2, m * 0.95, n + 0.1f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Start of complex control flow with goto */
    int iteration = 0;
    
loop_start:
    if (iteration >= 4) goto compute_final;
    
    /* Arithmetic on all variables before first function call */
    v1 = v2 * v3 + v4;
    v2 = v3 - v4 * v5;
    v3 = v4 + v5 / (v1 + 1);
    v4 = v5 * 2 - v1;
    v5 = v1 + v2 + v3 + v4;
    
    f1 = f2 * 0.5f + f3;
    f2 = f3 / 1.5f - f4;
    f3 = f4 * 2.0f + f5;
    f4 = f5 / 3.0f - f1;
    f5 = f1 + f2 + f3 + f4;
    
    d1 = d2 * 0.7 + d3;
    d2 = d3 / 1.3 - d4;
    d3 = d4 * 1.8 + d5;
    d4 = d5 / 2.2 - d1;
    d5 = d1 + d2 + d3 + d4;
    
    /* First function call with many live variables */
    double result1 = sin(d1) * cos(d2) + pow(d3, 1.5);
    printf("Iteration %d: sin*cos+pow=%.6f\n", iteration, result1);
    
    /* More arithmetic between calls */
    v1 += (int)(result1 * 100);
    f1 += (float)result1;
    d1 += result1;
    
    /* Conditional branch - both paths contain function calls */
    if (iteration % 2 == 0) {
        /* Path 1: Call at end of basic block before goto */
        double rand_val = (double)rand() / RAND_MAX;
        printf("Even iter rand: %.6f\n", rand_val);
        f2 = f3 * rand_val + f4;
        
        /* Inline assembly clobbering caller-saved registers */
        __asm__ volatile (
            "imul %1, %0\n\t"
            "add $1, %0\n\t"
            : "+r"(v2), "+r"(v3)
            : "r"(v4)
            : "%eax", "%ecx", "%edx", "%esi", "%edi", "cc"
        );
        
        /* Function call right before label (potential BB_END) */
        d2 = tan(d1) * atan(d3);
        goto after_condition;  /* This creates a basic block ending with call */
    } else {
        /* Path 2: Different call pattern */
        v3 = abs(v2 - v4) * v5;
        f3 = sqrtf(fabsf(f2 - f4));
        
        /* Another inline assembly block */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %%eax, %0\n\t"
            : "+r"(v1)
            : "r"(v5)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Call to external function */
        d3 = log(d4 + 1.0) * exp(d5);
        printf("Odd iter log*exp=%.6f\n", d3);
    }

after_condition:
    /* More arithmetic to extend live ranges */
    v4 = v1 * v2 - v3 * v5;
    f4 = f1 * 2.0f - f2 * 3.0f + f3;
    d4 = d1 * 1.5 - d2 * 2.0 + d3;
    
    /* Call recursive function with all variables as arguments */
    double rec_result = recursive_helper(
        2,  /* depth */
        d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, f5
    );
    
    printf("Recursive result: %.6f\n", rec_result);
    
    /* Another function call with mixed arguments */
    printf("Mixed: v1=%d f1=%.2f d1=%.2f v2=%d f2=%.2f\n", 
           v1, f1, d1, v2, f2, d2);
    
    /* Final arithmetic in the loop */
    v5 = v1 + v2 * 2 + v3 * 3 + v4 * 4;
    f5 = f1 + f2 * 1.5f + f3 * 2.0f + f4 * 2.5f;
    d5 = d1 + d2 * 0.8 + d3 * 0.6 + d4 * 0.4;
    
    iteration++;
    goto loop_start;

compute_final:
    /* Compute final checksum from all variables */
    double checksum = 0.0;
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    /* One final function call */
    checksum = fabs(checksum) + sin(checksum) * 0.5;
    
    printf("Final checksum: %.12f\n", checksum);
    
    /* Another basic block ending with a call */
    if (checksum > 100.0) {
        printf("Large checksum!\n");
        return 0;  /* Return here creates BB_END before call */
    }
    
    printf("Normal checksum\n");
    return 0;
}
