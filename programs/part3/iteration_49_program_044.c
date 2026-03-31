#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, float c, float d,
                       int e, int f, double g, float h, int i,
                       double j, float k, int l, double m, int n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix up the arguments to create register pressure */
    double sum = a * b - c / d + e % (f + 1);
    sum += sin(g) * cos(h) + pow(j, 2.0);
    
    /* Function call within recursion */
    printf("Depth %d: sum=%.3f\n", depth, sum);
    
    /* Recursive call with shuffled arguments */
    return sum + recursive_helper(depth - 1,
                                 b, a, d, c, 
                                 f, e, h, g,
                                 i + 1, k, j, 
                                 l - 1, m * 0.9, n % 7);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double total = 0.0;
    
    /* Label for goto jumps to manipulate basic block structure */
    loop_start:
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Nested inner loop with arithmetic on all variables */
        for (int inner = 0; inner < 2; inner++) {
            /* Arithmetic operations before function call */
            v1 = v2 * v3 + v4 - v5;
            f1 = f2 * f3 / f4 + f5;
            d1 = d2 * d3 - d4 / d5;
            
            /* Inline assembly that clobbers caller-saved registers */
            /* Clobber eax, ecx, edx which are caller-saved on x86 */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(v1) : "r"(v2) : "%eax", "%ecx", "%edx"
            );
            
            /* Function call with many live variables */
            /* This should force caller-save restoration */
            double result = sin(d1) * cos(d2) + pow(d3, f1);
            printf("Iteration %d-%d: sin=%.3f, v1=%d\n", 
                   outer, inner, result, v1);
            
            /* More arithmetic after function call */
            v2 = v1 + v3 * v4;
            f2 = f1 * 1.5f - f3;
            d2 = d1 * 2.0 + d4;
            
            /* Another function call with different arguments */
            /* rand() is external, uses caller-saved registers */
            int r = rand() % 100;
            v3 = (v2 + v4 + v5) * (r + 1);
            
            /* Inline assembly with different clobbers */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(v3) : "r"(v4) : "%eax", "%edx"
            );
            
            /* Conditional branch where both paths have function calls */
            if (v3 % 2 == 0) {
                /* Path 1: Function call at end of basic block */
                f3 = tan(f2) * exp(f1);
                /* Call just before label - might be at BB end */
                printf("Even path: f3=%.3f\n", f3);
                goto even_label;  /* Creates BB boundary */
            } else {
                /* Path 2: Different function call pattern */
                d3 = log(d2) * sqrt(d1);
                /* Multiple operations after call */
                v4 = v3 * 2 - v5;
                printf("Odd path: d3=%.3f, v4=%d\n", d3, v4);
                
                /* Another external function call */
                double power_result = pow(d4, 1.5);
                f4 = (float)power_result * f5;
                
                goto odd_label;  /* Another BB boundary */
            }
            
            /* Labels for goto targets */
            even_label:
            /* Arithmetic after label */
            v5 = v4 + v1 * 2;
            f5 = f4 / 2.0f + f2;
            
            /* Recursive function call with many arguments */
            /* All 15 variables are live across this call */
            double rec_result = recursive_helper(2, d1, d2, f1, f2,
                                               v1, v2, d3, f3, v3,
                                               d4, f4, v4, d5, v5);
            
            total += rec_result;
            
            odd_label:
            /* More arithmetic mixing all variable types */
            d4 = d3 * 1.1 + d5;
            d5 = d4 / 1.5 - d2;
            
            /* Another function call with mixed arguments */
            printf("Running total: %.3f, d4=%.3f\n", total, d4);
            
            /* Final arithmetic in the loop */
            v1 = (v1 + v2 + v3 + v4 + v5) % 1000;
            f1 = f1 + f2 + f3 + f4 + f5;
            d1 = d1 + d2 + d3 + d4 + d5;
        }
        
        /* Function call at potential BB end */
        if (outer < 2) {
            /* Call to external function */
            double last_sin = sin(d1 * d5);
            printf("Loop %d complete, last_sin=%.3f\n", outer, last_sin);
            /* This call might be at BB end if no more statements follow */
        }
    }
    
    /* One more goto to create unusual control flow */
    if (total < 1000) {
        printf("Total below 1000, recalculating...\n");
        /* Function call then goto */
        v1 = rand() % 50;
        goto loop_start;
    }
    
    /* Compute final checksum using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5 +
                     total;
    
    /* Final printf to prevent dead code elimination */
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)checksum % 256;
}
