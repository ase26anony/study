#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, float f,
                        int g, int h, int i, double j, float k, int l, int m, int n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + f;
    int t3 = g ^ h | i;
    double t4 = j + k * 2.0;
    int t5 = l + m * n;
    
    /* Call external functions within recursion */
    double s1 = sin(t1);
    double s2 = pow(t4, 2.0);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r" (t5)
        : "r" (t3), "r" (depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, t1 + s1, b - s2, c * 0.5, 
                           d + t2, e * 1.1f, f - 0.5f,
                           g + t3, h ^ t5, i | depth,
                           j + t4, k * 0.8f, l + 1, m - 1, n * 2);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    /* Variable to track control flow */
    int branch_selector = 0;
    
    /* Loop to create register pressure */
    for (int iter = 0; iter < 4; iter++) {
        /* Perform arithmetic on all variables before calls */
        v1 = v2 * v3 + iter;
        v2 = v4 ^ v5 | v6;
        v3 = v1 + v2 - v3;
        v4 = v5 * v6 / (iter + 1);
        v5 = v6 + iter * 2;
        v6 = v1 ^ v2 ^ v3;
        
        f1 = f2 * 0.5f + f3;
        f2 = f4 / 1.3f - f5;
        f3 = f1 + f2 * f3;
        f4 = f5 * 2.0f + iter;
        f5 = f3 - f4 / 1.7f;
        
        d1 = d2 * 0.75 + d3;
        d2 = d4 / 1.9 - d5;
        d3 = d1 + d2 * sin(d3);
        d4 = d5 * 3.0 + iter;
        d5 = d3 - d4 / 2.3;
        
        /* Call external functions with different argument subsets */
        printf("Iter %d: v1=%d, f1=%.2f, d1=%.3f\n", iter, v1, f1, d1);
        
        /* Inline assembly that clobbers multiple caller-saved registers */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "imul %3, %%ecx"
            : "=r" (v1)
            : "r" (v2), "r" (v3), "r" (iter)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (branch_selector % 2 == 0) {
            /* Path 1: Multiple operations then a call at block end */
            double result1 = pow(d1, d2) + cos(d3);
            float result2 = fabsf(f1) * f2;
            int result3 = abs(v4) + abs(v5);
            
            /* Function call at the end of basic block (just before goto) */
            printf("Branch A: pow=%.3f, fabs=%.3f\n", result1, result2);
            
            /* Use goto to create unusual basic block structure */
            goto special_block;
        } else {
            /* Path 2: Different pattern of operations */
            double result1 = sin(d4) * tan(d5);
            float result2 = sqrtf(f3) + logf(f4);
            
            /* Another call at block end */
            printf("Branch B: sin=%.3f, sqrt=%.3f\n", result1, result2);
            
            /* Call recursive function with many live variables */
            double rec_result = recursive_helper(
                2, d1, d2, d3, f1, f2, f3,
                v1, v2, v3, d4, f4, v4, v5, v6
            );
            printf("Recursive result: %.3f\n", rec_result);
            
            /* Jump to another block */
            goto alternate_block;
        }
        
    special_block:
        /* Block with function call at the end */
        d1 = d2 + d3 * 0.5;
        f1 = f2 - f3 / 1.2f;
        v1 = v2 * v3 + v4;
        
        /* Call external function */
        double rand_val = (double)rand() / RAND_MAX;
        printf("Special block: rand=%.3f\n", rand_val);
        
        /* Check if this is the last iteration to avoid infinite loop */
        if (iter < 3) {
            branch_selector++;
            continue;
        }
        
    alternate_block:
        /* Another block with operations and calls */
        d5 = d4 * 2.0 - d3;
        f5 = f4 * 1.5f + f3;
        v6 = v5 ^ v4 | v3;
        
        /* Function call with mixed arguments */
        printf("Alternate: d5=%.3f, f5=%.3f\n", d5, f5);
        
        /* More inline assembly */
        __asm__ volatile (
            "addl %1, %0\n\t"
            "subl %2, %0"
            : "+r" (v6)
            : "r" (v1), "r" (iter)
            : "%eax", "cc"
        );
        
        branch_selector = (branch_selector + 1) % 4;
    }
    
    /* Compute final checksum from all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 + v6 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* Final printf to prevent dead code elimination */
    printf("Final checksum: %.6f\n", checksum);
    
    return 0;
}
