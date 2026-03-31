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
    
    /* Mix operations to keep variables live */
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
    
    /* Recursive call with shuffled arguments to prevent optimization */
    return sin_val + recursive_helper(depth - 1,
                                     b, t1, d, t2,
                                     f, t3, h, j,
                                     l, m, a, c,
                                     e, g);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Start with a goto to create unusual basic block structure */
    goto start_block;
    
loop_begin:
    /* This block will have function calls at the end */
    for (int outer = 0; outer < 3; outer++) {
        /* Nested loops to extend live ranges */
        for (int inner = 0; inner < 100; inner++) {
            /* Arithmetic on all variables to keep them live */
            v1 = v2 * v3 + inner;
            v2 = v3 ^ v4 | v5;
            v3 = v4 - v5 * outer;
            v4 = v5 + v1 / (inner + 1);
            v5 = v1 % (v2 + 1);
            
            f1 = f2 * f3 + inner * 0.1f;
            f2 = f3 / f4 - outer * 0.2f;
            f3 = f4 + f5 * sinf(inner * 0.01f);
            f4 = f5 - f1 * cosf(outer * 0.01f);
            f5 = f1 + f2 * tanf((inner + outer) * 0.001f);
            
            d1 = d2 * d3 + sin(inner * 0.01);
            d2 = d3 / d4 - cos(outer * 0.01);
            d3 = d4 + d5 * tan((inner + outer) * 0.001);
            d4 = d5 - d1 * log(fabs(d2) + 1.0);
            d5 = d1 + d2 * exp(-d3 * 0.0001);
            
            /* Function call with many live variables as arguments */
            printf("Iteration %d.%d: v1=%d, f1=%.2f, d1=%.2f\n", 
                   outer, inner, v1, f1, d1);
            
            /* Another function call with different arguments */
            double pow_result = pow(d1 + d2, f1 + f2);
            
            /* Inline assembly that clobbers multiple caller-saved registers */
            __asm__ volatile (
                "mov %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "imul %3, %4\n\t"
                "add %%eax, %0\n\t"
                : "+r"(v1), "+r"(v2)
                : "r"(v3), "r"(v4), "r"(v5)
                : "%eax", "%ecx", "%edx", "memory"
            );
            
            /* Conditional branch where both paths contain function calls */
            if ((inner + outer) % 7 == 0) {
                /* Path 1: Function call at the end of basic block */
                double sin_val = sin(d1) + cos(d2) + tan(d3);
                checksum += sin_val;
                /* This call is at block end before goto */
                printf("Special case triggered: %.4f\n", sin_val);
                goto special_handler;
            } else {
                /* Path 2: Different function call pattern */
                float rand_val = (float)rand() / RAND_MAX;
                checksum += rand_val;
                /* Call to external function */
                double log_val = log(fabs(d4) + 1.0);
                printf("Normal case: rand=%.4f, log=%.4f\n", rand_val, log_val);
            }
            
            /* Recursive function call with many arguments */
            if (inner % 13 == 0) {
                double rec_result = recursive_helper(
                    2,  /* depth */
                    d1, d2, f1, f2,
                    v1, v2, d3, f3,
                    v3, d4, f4, v4,
                    d5, v5
                );
                checksum += rec_result;
            }
        }
        
        /* Function call placed right before loop end (potential block end) */
        printf("Outer loop %d complete. f5=%.2f, d5=%.2f\n", outer, f5, d5);
    }
    
    goto final_block;
    
start_block:
    /* Initial computations before entering main loop */
    d1 = sin(d1) * 2.0;
    f1 = cosf(f1) * 3.0f;
    v1 = rand() % 100;
    printf("Starting computation...\n");
    goto loop_begin;
    
special_handler:
    /* Handler block for special case */
    /* Function call at beginning of handler */
    double special_pow = pow(d1, d2);
    
    /* More arithmetic to keep variables live */
    v1 = v1 * 2 + v2;
    f1 = f1 * 1.5f + f2;
    d1 = d1 * 1.25 + d2;
    
    /* Another function call */
    printf("Special handler: pow=%.4f\n", special_pow);
    
    /* Inline assembly with different clobbered registers */
    __asm__ volatile (
        "xchg %1, %0\n\t"
        "sub %2, %0\n\t"
        : "+r"(v3), "+r"(v4)
        : "r"(v5)
        : "%eax", "%ebx", "memory"
    );
    
    goto loop_begin;
    
final_block:
    /* Final computations and output */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    /* One more function call before return */
    printf("Final checksum: %.10f\n", checksum);
    
    /* Additional arithmetic after function call */
    int final_check = v1 * 100 + v2 * 10 + v3;
    
    return final_check % 256;
}
