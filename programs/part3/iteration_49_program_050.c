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
    
    /* Mix of arithmetic operations */
    double t1 = a * b + c * d;
    float t2 = e * f - g * h;
    int t3 = i * j + k * l;
    double t4 = m * n - a * c;
    
    /* Function call within recursion */
    printf("Depth %d: t1=%.3f t2=%.3f t3=%d t4=%.3f\n", 
           depth, t1, t2, t3, t4);
    
    /* Recursive call with shuffled arguments to prevent optimization */
    return recursive_helper(depth - 1,
                           b, c, d, e, f, g, h, i, j,
                           k, l, m, n, a) + sin(t1) * cos(t2);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55, d6 = 6.66;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        v1 = v2 * v3 + outer;
        v4 = v5 - v6 * outer;
        f1 = f2 * f3 + outer;
        f4 = f5 / (outer + 1);
        d1 = d2 * d3 + outer;
        d4 = d5 - d6 * outer;
        
        /* Inline assembly clobbering multiple caller-saved registers */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r"(v1) : "r"(v2) : "%eax", "%ecx", "%edx", "%esi", "%edi"
        );
        
        /* Function call with many live variables */
        printf("Outer=%d: v1=%d v4=%d f1=%.3f f4=%.3f d1=%.3f d4=%.3f\n",
               outer, v1, v4, f1, f4, d1, d4);
        
        /* More arithmetic mixing variables */
        d2 = sin(d1) * cos(d3);
        f2 = powf(f1, 1.5f);
        v2 = rand() % 100 + v3 * v4;
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d3 = pow(d4, 2.0) + tan(d5);
            f3 = sinf(f4) * cosf(f5);
            v3 = v1 * v2 - v4;
            /* Function call right before label (potential BB_END) */
            printf("Even: d3=%.3f f3=%.3f v3=%d\n", d3, f3, v3);
even_block_end:
            checksum += d3 + f3 + v3;
        } else {
            /* Path 2: Different call pattern */
            d3 = log(d4) * exp(d5);
            f3 = sqrtf(f4) + logf(f5);
            v3 = v1 + v2 * v4;
            /* Another function call */
            double result = pow(d1, d2) + sin(d3);
            printf("Odd: result=%.3f\n", result);
            
            /* Use goto to create unusual BB structure */
            if (v3 > 50) {
                goto skip_recursive;
            }
            
            /* Recursive call with many arguments */
            double rec_result = recursive_helper(2, d1, d2, f1, f2,
                                                v1, v2, d3, f3, v3,
                                                d4, f4, v4, d5, f5);
            printf("Recursive result: %.3f\n", rec_result);
            checksum += rec_result;
            
skip_recursive:
            /* More arithmetic after label */
            d6 = d1 * d2 - d3 * d4;
        }
        
        /* Inline assembly with different clobbers */
        asm volatile (
            "mov %1, %%eax\n\t"
            "imul %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r"(v5) : "r"(v6), "r"(outer) : "%eax", "%edx"
        );
        
        /* Another function call with mixed arguments */
        f5 = sinf(f1 + f2) * cosf(f3 + f4);
        d5 = pow(d1 + d2, d3 + d4);
        v5 = v1 * v2 + v3 * v4;
        
        /* Call at potential block end before loop continues */
        printf("Loop end: f5=%.3f d5=%.3f v5=%d\n", f5, d5, v5);
        
        /* Update all variables to keep them live */
        v6 = v5 + v1;
        f1 = f5 * 0.9f;
        d6 = d5 * 0.9;
        v1 = v6 - v2;
        f2 = f1 + f3;
        d1 = d6 + d2;
        v2 = v1 * v3;
        f3 = f2 - f4;
        d2 = d1 * d3;
        v3 = v2 / (v4 + 1);
        f4 = f3 * f5;
        d3 = d2 / d4;
        v4 = v3 + v5;
        f5 = f4 + 1.0f;
        d4 = d3 * 1.1;
        v5 = v4 - v6;
        d5 = d4 + d6;
    }
    
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 +
                f1 + f2 + f3 + f4 + f5 +
                d1 + d2 + d3 + d4 + d5 + d6;
    
    /* One more function call before return */
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)(checksum * 1000) % 1000;
}
