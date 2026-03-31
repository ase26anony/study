/* caller-save-test.c */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                        int e, int f, double g, float h, int i, 
                        double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix arithmetic to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * l + (int)(j * k);
    double t4 = m * n + depth;
    
    /* Call external function within recursion */
    double sin_val = sin(t1 + t4);
    
    /* More arithmetic */
    t1 += sin_val;
    t2 *= 1.1f;
    t3 ^= (int)t4;
    
    /* Recursive call with shuffled arguments to prevent optimization */
    return t1 + recursive_helper(depth - 1, 
                                 b, a, d, c, 
                                 f, e, h, g, 
                                 l, i, k, j, 
                                 n, m) + t2 + t3;
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
            /* Arithmetic operations before function call */
            v1 = v2 * v3 + v4;
            v2 = v3 ^ v5;
            v3 = v4 | v1;
            v4 = v5 + inner;
            v5 = v1 - v2;
            
            f1 = f2 * f3 + f4;
            f2 = f3 / f5;
            f3 = f4 - f1;
            f4 = f5 + (float)inner;
            f5 = f1 * 0.9f;
            
            d1 = d2 * d3 + d4;
            d2 = d3 / d5;
            d3 = d4 - d1;
            d4 = d5 + (double)inner;
            d5 = d1 * 0.99;
            
            /* Inline assembly that clobbers caller-saved registers */
            /* Clobber eax, ecx, edx (x86-64: rax, rcx, rdx) */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(v1)
                : "r"(v2)
                : "%eax", "%ecx", "%edx"
            );
            
            /* Another inline assembly clobbering different registers */
            /* Clobber xmm0, xmm1 (floating-point registers) */
            asm volatile (
                "addsd %1, %0\n\t"
                : "+x"(d1)
                : "x"(d2)
                : "%xmm0", "%xmm1"
            );
            
            /* Function call with many arguments - forces caller-save */
            printf("Iteration %d-%d: v1=%d, f1=%.2f, d1=%.2f\n", 
                   outer, inner, v1, f1, d1);
            
            /* Arithmetic operations after function call */
            v1 += rand() % 10;
            f1 += (float)rand() / RAND_MAX;
            d1 += (double)rand() / RAND_MAX;
            
            /* Call external math function */
            double power_result = pow(d1, d2);
            checksum += power_result;
            
            /* More arithmetic */
            v2 = (int)(sin(d3) * 100);
            f2 = (float)cos(d4);
            
            /* Conditional branch where both paths contain function calls */
            if (v1 % 2 == 0) {
                /* Path 1: Function call at the end of basic block */
                d3 = sin(d1) + cos(d2);
                printf("Even path: sin+cos=%.4f\n", d3);
                /* Function call just before label (end of block) */
                checksum += tan(d3);
                goto special_case;  /* Creates unusual control flow */
            } else {
                /* Path 2: Multiple function calls */
                float trig_sum = (float)(sin(d1) + cos(d2));
                printf("Odd path: trig_sum=%.4f\n", trig_sum);
                
                /* Call at end of this basic block too */
                checksum += asin(f3 > 1.0 ? 0.5 : f3);
            }
            
            /* Call recursive function with all 15 variables */
            double rec_result = recursive_helper(2, d1, d2, f1, f2,
                                                v1, v2, d3, f3, v3,
                                                d4, f4, v4, d5, f5);
            checksum += rec_result;
            
            continue;  /* Skip the special_case label in normal flow */
            
            special_case:
            /* This block is only reached via goto */
            /* Function call at end of this unusual basic block */
            d4 = log(fabs(d1) + 1.0);
            printf("Special case: log=%.4f\n", d4);
            
            /* More arithmetic to keep variables live */
            v3 = (int)(d4 * 1000);
            f3 = (float)sqrt(d4);
        }
        
        /* Another function call with different arguments */
        double hypot_result = hypot(d1, d2);
        checksum += hypot_result;
        
        /* Inline assembly with multiple clobbers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(v5)
            : "r"(v3), "r"(v4)
            : "%eax", "%edx"
        );
    }
    
    /* Final computation using all variables */
    double final_result = 
        v1 + v2 + v3 + v4 + v5 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5 +
        checksum;
    
    /* Print final result to prevent dead code elimination */
    printf("Final checksum: %.15f\n", final_result);
    
    /* One more conditional with function call at block end */
    if (final_result > 1000.0) {
        printf("Large result detected!\n");
        /* Function call at end of basic block */
        return (int)fmod(final_result, 1000.0);
    } else {
        printf("Normal result\n");
        /* Another function call at end of basic block */
        return (int)ceil(final_result);
    }
}
