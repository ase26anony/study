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
    
    /* Mix all variables in complex ways */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * l + (int)(j * k);
    double t4 = m * n + t1;
    
    /* Function call within recursion to create more caller-save opportunities */
    double sin_val = sin(t4);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "add %1, %0\n\t"
        "imul %2, %3\n\t"
        : "+r"(e), "+r"(f)
        : "r"(i), "r"(l)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    /* Recursive call with shuffled arguments */
    return sin_val + recursive_helper(depth - 1, 
                                     b, t1, d, t2,
                                     f, t3, h, j,
                                     l, k, m, i,
                                     t4, n);
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
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Nested inner loop with arithmetic on all variables */
        for (int inner = 0; inner < 2; inner++) {
            /* Complex arithmetic keeping all variables live */
            v1 = v2 * v3 + v4 - v5;
            v2 = v3 ^ v4 | v5;
            v3 = v4 + v5 * v1;
            v4 = v5 - v1 / (v2 + 1);
            v5 = v1 % (v2 + 3) + v3;
            
            f1 = f2 * f3 + f4;
            f2 = f3 / f4 * f5;
            f3 = f4 + f5 - f1;
            f4 = f5 * 1.5f + f2;
            f5 = f1 / 2.0f + f3;
            
            d1 = d2 * d3 + sin(d4);
            d2 = d3 / d4 * cos(d5);
            d3 = d4 + tan(d5) * d1;
            d4 = d5 * 2.0 + log(fabs(d2) + 1.0);
            d5 = d1 / 3.0 + pow(d3, 1.5);
            
            /* Conditional branch - both paths contain function calls */
            if ((outer + inner) % 2 == 0) {
                /* Path 1: Multiple function calls with live variables as arguments */
                printf("Outer=%d, Inner=%d: v1=%d, f1=%.2f, d1=%.3f\n", 
                       outer, inner, v1, f1, d1);
                
                /* Call external math functions */
                double pow_result = pow(d2, d3);
                checksum += pow_result;
                
                /* Function call at the end of basic block (before goto) */
                if (inner == 1) {
                    /* This call is at block end before the goto */
                    double sin_result = sin(d4);
                    checksum += sin_result;
                    goto special_label;  /* Creates BB_END before label */
                }
                
                /* Another call after the conditional goto */
                double cos_result = cos(d5);
                checksum += cos_result;
            } else {
                /* Path 2: Different set of function calls */
                /* Call with many arguments to increase register pressure */
                int rand_val = rand() % 100;
                printf("Random: %d, v2=%d, f2=%.2f\n", rand_val, v2, f2);
                
                /* Multiple consecutive function calls */
                double log_result = log(fabs(d1) + 1.0);
                double exp_result = exp(d2 * 0.1);
                checksum += log_result + exp_result;
                
                /* Inline assembly clobbering caller-saved registers */
                asm volatile (
                    "mov %1, %%eax\n\t"
                    "add %2, %%eax\n\t"
                    "imul %3, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "=r"(v1)
                    : "r"(v2), "r"(v3), "r"(v4)
                    : "%eax", "%ecx", "%edx", "memory"
                );
            }
            
            special_label:
            /* Recursive function call with many live variables as arguments */
            double rec_result = recursive_helper(2, d1, d2, f1, f2,
                                                v1, v2, d3, f3, v3,
                                                d4, f4, v4, d5, f5);
            checksum += rec_result;
            
            /* More arithmetic after function call to extend live ranges */
            v1 = v1 + (int)(rec_result * 100);
            f1 = f1 + (float)fmod(rec_result, 2.0);
            d1 = d1 + rec_result / 10.0;
            
            /* Another function call */
            if (outer > 0) {
                printf("Checksum so far: %.6f\n", checksum);
            }
        }
        
        /* Goto to create unusual control flow */
        if (outer == 1) {
            /* Jump back to create loop with different structure */
            v5 = v5 * 2;
            goto loop_start;
        }
    }
    
    /* Final computation using all variables */
    double final_result = 
        v1 + v2 + v3 + v4 + v5 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5 +
        checksum;
    
    /* Final printf to prevent dead code elimination */
    printf("Final result: %.15f\n", final_result);
    
    return (int)(final_result * 1000) % 1000;
}
