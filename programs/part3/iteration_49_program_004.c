/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -lm -o caller-save-test
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to create register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                        int e, int f, double g, float h, int i, 
                        double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix arithmetic and function calls to create complex live ranges */
    double temp1 = a * b + sin(c) * cos(d);
    int temp2 = e * f + rand() % 100;
    float temp3 = h * k + powf(n, 2.0f);
    
    /* Call external functions within recursion */
    if (depth % 2 == 0) {
        printf("Depth %d: temp1=%.3f, temp2=%d\n", depth, temp1, temp2);
    }
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        "imul %2, %3\n\t"
        : "+r"(temp2), "+r"(e)
        : "r"(f), "r"(l)
        : "%eax", "%ecx", "%edx", "cc", "memory"
    );
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           temp1, b * 0.9, c * 1.1f, d * 0.8f,
                           temp2, f + 1, g * 1.05, h * 0.95f, i * 2,
                           j + temp1, k * 1.2f, l - 1, m / 1.1, n * 0.9f);
}

int main() {
    srand(time(NULL));
    
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    double total = 0.0;
    int iteration_count = 0;
    
    /* Start of complex control flow with goto labels */
    start_loop:
    
    /* Loop with 4 iterations to create register pressure */
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 2; inner++) {
            iteration_count++;
            
            /* Perform arithmetic on all variables before function calls */
            v1 = v2 * v3 + v4 - v5;
            v2 = v1 ^ v3 | v4 & v5;
            v3 = v1 + v2 * v4 / (v5 + 1);
            v4 = v2 % (v3 + 1) + v5;
            v5 = v1 * v3 - v2 + v4;
            
            f1 = f2 * f3 + sinf(f4) * cosf(f5);
            f2 = f1 / f3 + tanf(f4) - f5;
            f3 = f2 * 1.1f + f4 * 0.9f + f5;
            f4 = f3 * 0.8f + f1 * 1.2f;
            f5 = f4 * 0.7f + f2 * 1.3f;
            
            d1 = d2 * d3 + sin(d4) * cos(d5);
            d2 = d1 / d3 + pow(d4, 1.5) - d5;
            d3 = d2 * 1.05 + log(fabs(d4) + 1.0) + d5;
            d4 = d3 * 0.95 + sqrt(d1 + d2);
            d5 = d4 * 0.85 + exp(d3 * 0.1);
            
            /* Conditional branch - both paths contain function calls */
            if ((iteration_count + outer + inner) % 3 == 0) {
                /* Path 1: Function call at the end of basic block */
                total += sin(d1) * cos(d2) + pow(d3, 2.0);
                
                /* Inline assembly clobbering caller-saved registers */
                __asm__ volatile (
                    "mov %1, %%eax\n\t"
                    "add %2, %%eax\n\t"
                    "imul %3, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "=r"(v1)
                    : "r"(v2), "r"(v3), "r"(v4)
                    : "%eax", "%ecx", "%edx", "cc"
                );
                
                /* Call to external function with many arguments */
                printf("Iteration %d: v1=%d, f1=%.3f, d1=%.3f\n", 
                       iteration_count, v1, f1, d1);
                
                /* Jump to create unusual basic block structure */
                if (iteration_count % 5 == 0) {
                    goto special_case;
                }
            } else {
                /* Path 2: Different pattern of function calls */
                total += cos(d2) * sin(d3) + pow(d4, 1.5);
                
                /* Another inline assembly with different clobbers */
                __asm__ volatile (
                    "add %1, %0\n\t"
                    "sub %2, %0\n\t"
                    "mul %3\n\t"
                    : "+r"(v2)
                    : "r"(v3), "r"(v4), "r"(v5)
                    : "%eax", "%edx", "cc"
                );
                
                /* Call to math library function */
                double temp = pow(d1, d2) + sin(d3) * cos(d4);
                
                /* Function call right before conditional end */
                if (temp > 100.0) {
                    printf("Large temp: %.3f at iteration %d\n", temp, iteration_count);
                }
            }
            
            /* Call recursive helper function - creates deep register pressure */
            if (iteration_count % 4 == 0) {
                double rec_result = recursive_helper(
                    2,  /* depth */
                    d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, f5
                );
                total += rec_result * 0.1;
            }
            
            /* More arithmetic after function calls to extend live ranges */
            v1 = v1 + (int)(f1 * 10);
            v2 = v2 ^ (int)(f2 * 20);
            v3 = v3 | (int)(f3 * 30);
            f1 = f1 + (float)(v1 % 100) * 0.01f;
            f2 = f2 * (1.0f + (float)(v2 % 50) * 0.02f);
            d1 = d1 + (double)(v3) * 0.001;
            d2 = d2 * (1.0 + sin((double)iteration_count));
            
            /* Another function call with mixed arguments */
            if (outer == 1 && inner == 0) {
                printf("Mid-loop: d1=%.3f, d2=%.3f, v1=%d\n", d1, d2, v1);
            }
        }
        
        /* Function call at loop boundary */
        total += tan(d3) * atan(d4) + log(d5 + 1.0);
    }
    
    /* Label for goto target */
    special_case:
    
    /* Additional computations after goto */
    v1 = v1 * 2 + rand() % 100;
    f1 = f1 * 1.5f + (float)rand() / RAND_MAX;
    d1 = d1 * 1.2 + (double)rand() / RAND_MAX;
    
    /* One more function call */
    printf("After special case: v1=%d\n", v1);
    
    /* Conditional goto to create more complex CFG */
    if (iteration_count < 4) {
        goto start_loop;
    }
    
    /* Final computation using all variables */
    double checksum = 
        v1 + v2 + v3 + v4 + v5 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5 +
        total;
    
    /* Final printf to prevent dead code elimination */
    printf("Final checksum: %.6f\n", checksum);
    printf("Iterations completed: %d\n", iteration_count);
    
    return (int)(checksum * 1000) % 1000;
}
