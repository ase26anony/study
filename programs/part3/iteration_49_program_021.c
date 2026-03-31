/* caller-save-test.c */
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
    
    /* Mix of arithmetic operations */
    double t1 = a * b - c / d + e % (f + 1);
    float t2 = c * d - h * k;
    int t3 = e ^ f ^ i ^ l ^ n;
    double t4 = g + j + m + t1;
    
    /* Function call within recursion */
    double sin_val = sin(t4);
    
    /* More arithmetic */
    t1 += sin_val * 0.5;
    t2 += (float)cos(t1);
    t3 += (int)(t2 * 100);
    
    /* Recursive call with shuffled arguments */
    return recursive_helper(depth - 1,
                           b, a, d, c,
                           f, e, j, k,
                           l, g, h, i,
                           t1, t3) * 0.99;
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop creating register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Nested loop */
        for (int inner = 0; inner < 2; inner++) {
            /* Arithmetic on all variables before call */
            v1 = v2 * v3 + v4 - v5;
            v2 = v3 ^ v4 ^ v5;
            v3 = v4 % (v1 + 1) + v5;
            v4 = v5 * 2 - v1;
            v5 = v1 + v2 + v3 + v4;
            
            f1 = f2 * 0.5f + f3 - f4;
            f2 = f3 / (f1 + 0.1f) * f5;
            f3 = f4 + f5 * 2.0f;
            f4 = f5 - f1 * f2;
            f5 = f1 + f2 + f3 + f4;
            
            d1 = d2 * 0.7 + d3 / d4;
            d2 = d3 + sin(d1) * 0.3;
            d3 = d4 * pow(1.1, d5);
            d4 = d5 - tan(d1) * 0.2;
            d5 = d1 * d2 + d3 - d4;
            
            /* Inline assembly clobbering caller-saved registers */
            __asm__ volatile (
                "addl %1, %0\n\t"
                "imull %2, %0\n\t"
                : "+r"(v1)
                : "r"(v2), "r"(v3)
                : "%eax", "%ecx", "%edx", "cc"
            );
            
            /* Function call with many live variables */
            printf("Iteration %d-%d: v1=%d, f1=%.2f, d1=%.3f\n", 
                   outer, inner, v1, f1, d1);
            
            /* More arithmetic after call */
            v1 += (int)(f1 * 10);
            f1 += (float)d1 * 0.1f;
            d1 += (double)v1 * 0.01;
            
            /* Another function call */
            double pow_result = pow(d2, 1.5);
            f2 += (float)pow_result;
            
            /* Inline assembly with different clobbers */
            __asm__ volatile (
                "mov %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "=r"(v2)
                : "r"(v3), "r"(v4)
                : "%eax", "cc"
            );
            
            /* Conditional branch with calls at block ends */
            if (v1 % 2 == 0) {
                /* Path 1: Call at end of basic block */
                d3 = sin(d4) + cos(d5);
                v3 = rand() % 100;  /* Call at block end before goto */
                goto compute_block;
            } else {
                /* Path 2: Different call pattern */
                f3 = (float)log(fabs(d1) + 1.0);
                v4 = abs(v5 * 2 - v1);  /* Call at block end */
                /* Fall through */
            }
            
            /* More arithmetic */
            v5 = v1 * v2 - v3 + v4;
            
compute_block:
            /* Recursive call with all variables as arguments */
            double rec_result = recursive_helper(
                2,  /* depth */
                d1, d2, f1, f2,
                v1, v2, d3, f3,
                v3, d4, f4, v4,
                d5, v5
            );
            
            /* Use result */
            d5 += rec_result * 0.01;
            
            /* Another external call */
            float sinf_result = sinf(f5);
            f5 = f5 * 0.5f + sinf_result;
            
            /* Unusual control flow with goto */
            if (inner == 0 && outer > 0) {
                goto skip_call;
            }
            
            /* Function call just before label (potential BB_END) */
            printf("Middle: d5=%.3f\n", d5);
            
skip_call:
            /* More arithmetic to extend live ranges */
            v1 = v2 + v3;
            v2 = v4 - v5;
            v3 = v1 * v2;
            v4 = v5 / (v1 + 1);
            v5 = v3 % (v4 + 1);
            
            f1 = f2 + f3;
            f2 = f4 - f5;
            f3 = f1 * f2;
            f4 = f5 / (f1 + 0.1f);
            f5 = f3 + f4;
            
            d1 = d2 + d3;
            d2 = d4 - d5;
            d3 = d1 * d2;
            d4 = d5 / (d1 + 0.1);
            d5 = d3 + d4;
        }
        
        /* Another conditional with call at end */
        if (outer == 1) {
            v1 = (int)(sqrt(fabs(d1)) * 100);
            /* Call at potential BB_END */
            printf("Outer=%d: sqrt result affects v1=%d\n", outer, v1);
        }
    }
    
    /* Final computation using all variables */
    double checksum = 
        v1 + v2 + v3 + v4 + v5 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5;
    
    /* One more call before return */
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)(checksum * 1000) % 1000;
}
