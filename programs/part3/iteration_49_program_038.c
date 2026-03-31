/* caller-save-test.c
 * Designed to trigger uncovered lines in caller-save.cc (lines 905-913)
 * by creating complex register pressure and specific instruction ordering.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                        int e, int f, double g, float h, int i, 
                        double j, float k, int l, double m, int n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix of operations to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * j + k * l;
    
    /* Function call within recursion to create more caller-save opportunities */
    double sin_val = sin(m + n);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        "imul %2, %3\n\t"
        : "+r"(e), "+r"(f)
        : "r"(i), "r"(l)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    return recursive_helper(depth - 1, 
                           t1 + sin_val, b * 0.9, c * 1.1, d * 0.8,
                           e + 1, f - 1, g * 1.2, h * 0.7, i * 2,
                           j / 2.0, k * 1.5, l / 2, m * 0.95, n + depth) 
           + printf(".");  /* printf returns int, affects result */
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Label for goto to create unusual basic block structure */
    loop_start:
    
    /* Outer loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Nested loop with arithmetic on all variables */
        for (int inner = 0; inner < 100; inner++) {
            /* Arithmetic operations before function call */
            v1 = v1 * 3 + v2;
            v2 = v2 + v3 - v4;
            v3 = v3 ^ v5;
            v4 = v4 * 2 + inner;
            v5 = v5 % 7 + outer;
            
            f1 = f1 * 1.01f + f2;
            f2 = f2 - f3 * 0.5f;
            f3 = f3 + f4 / 2.0f;
            f4 = f4 * f5 * 0.9f;
            f5 = f5 + sinf(f1);
            
            d1 = d1 * 1.001 + d2;
            d2 = d2 + pow(d3, 1.1);
            d3 = d3 * 0.99 + d4;
            d4 = d4 / 1.1 + d5;
            d5 = d5 + tan(d1 * 0.1);
            
            /* Conditional branch - both paths contain function calls */
            if ((inner % 7) == 0) {
                /* Path 1: Call external functions with mixed arguments */
                printf("Iteration %d: v1=%d, f1=%.2f, d1=%.3f\n", 
                       inner, v1, f1, d1);
                
                /* Call at end of basic block (just before control flow change) */
                double pow_result = pow(d2, d3);
                checksum += pow_result;
                
                /* Inline assembly clobbering caller-saved registers */
                __asm__ volatile (
                    "mov %1, %%eax\n\t"
                    "add %2, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "=r"(v1)
                    : "r"(v2), "r"(v3)
                    : "%eax", "%ecx", "%edx", "memory"
                );
                
                /* Another function call */
                double sin_result = sin(d4 + d5);
                checksum += sin_result;
                
                /* This call is at the end of a basic block before goto */
                if (inner == 42) {
                    printf("Special case!\n");
                    goto special_label;  /* Creates block boundary */
                }
            } else {
                /* Path 2: Different function call pattern */
                /* Call external function */
                int rand_val = rand() % 100;
                checksum += rand_val;
                
                /* Arithmetic after function call */
                v1 = v1 + rand_val;
                f1 = f1 + rand_val * 0.1f;
                d1 = d1 + rand_val * 0.01;
                
                /* Call recursive function with many live arguments */
                double rec_result = recursive_helper(
                    2, d1, d2, f1, f2, v1, v2, d3, f3, v3, 
                    d4, f4, v4, d5, v5
                );
                checksum += rec_result;
                
                /* Another external function call */
                double cos_result = cos(d1);
                checksum += cos_result;
            }
            
            /* More arithmetic to keep variables live across potential spills */
            v1 = v1 | v2;
            v2 = v2 & v3;
            v3 = v3 + v4 * v5;
            
            f1 = f1 + f2 * f3;
            f2 = f2 - f4 / f5;
            
            d1 = d1 * d2 + d3;
            d2 = d2 - d4 * d5;
            
            /* Function call in the middle of the loop */
            if ((inner % 13) == 0) {
                printf("Checkpoint: %d\n", inner);
            }
        }
        
        special_label:
        /* Function call right after a label (basic block start) */
        double log_result = log(fabs(d1) + 1.0);
        checksum += log_result;
        
        /* Inline assembly that uses and clobbers registers */
        __asm__ volatile (
            "addl $1, %0\n\t"
            "addl $2, %1\n\t"
            : "+r"(v4), "+r"(v5)
            : 
            : "%eax", "%ecx", "%edx", "memory"
        );
        
        /* Another external function call with many arguments */
        printf("Outer loop %d: v1=%d v2=%d v3=%d f1=%.3f d1=%.3f\n",
               outer, v1, v2, v3, f1, d1);
    }
    
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    /* One more function call before return */
    printf("Final checksum: %.15f\n", checksum);
    
    /* Conditional goto to create another basic block with call at end */
    if (checksum > 1000.0) {
        printf("Large checksum!\n");
        goto loop_start;  /* Would create infinite loop without condition */
    }
    
    return (int)(checksum * 1000) % 256;
}
