#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, double c, 
                       float d, float e, float f,
                       int g, int h, int i,
                       double j, float k, int l, int m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix operations to keep values live */
    double t1 = a * b - c;
    float t2 = d / e + f;
    int t3 = g ^ h | i;
    
    /* Function call within recursion */
    double sin_val = sin(t1);
    
    /* Inline assembly clobbering caller-saved registers */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r"(t3) : "r"(depth) : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    
    return recursive_helper(depth - 1, 
                           t1 + sin_val, b * 0.9, c + 1.1,
                           t2 * 0.5f, e - 0.3f, f * 1.7f,
                           t3, h << 1, i >> 1,
                           j * 1.1, k * 0.9f, l + depth, m - depth);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7;
    
    srand(time(NULL));
    
    /* Loop creating register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables before call */
        d1 = d1 * 1.1 + sin(d2);
        d2 = d2 * 0.9 + cos(d3);
        d3 = d3 * 1.05 + tan(d4);
        d4 = d4 * 0.95 + pow(d1, 1.5);
        
        f1 = f1 * 1.2f + f2;
        f2 = f2 * 0.8f + f3;
        f3 = f3 * 1.1f + f4;
        f4 = f4 * 0.9f + f5;
        f5 = f5 * 1.3f + f1;
        
        i1 = i1 * 3 + i2;
        i2 = i2 * 5 + i3;
        i3 = i3 * 7 + i4;
        i4 = i4 * 11 + i5;
        i5 = i5 * 13 + i6;
        i6 = i6 * 17 + i7;
        i7 = i7 * 19 + i1;
        
        /* First function call with many live variables */
        printf("Iteration %d: d1=%.3f, f1=%.3f, i1=%d\n", 
               outer, d1, f1, i1);
        
        /* Inline assembly clobbering caller-saved registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(i1) : "r"(i2), "r"(i3) : "%eax", "%ecx", "%edx"
        );
        
        /* Conditional branch - both paths contain calls */
        if (d1 > 2.0) {
            /* Path A: Call at end of basic block */
            d2 = pow(d1, d3) + sin(d4);
            f3 = f2 * f4 / f5;
            i4 = i3 * i5 - i6;
            
            /* Function call just before goto */
            double rand_val = (double)rand() / RAND_MAX;
            d3 += rand_val * 0.1;
            
            goto alternate_path;
        } else {
            /* Path B: Multiple calls in sequence */
            d2 = log(fabs(d1) + 1.0);
            printf("log result: %.3f\n", d2);
            
            f3 = sqrtf(fabs(f2) + 1.0f);
            printf("sqrt result: %.3f\n", f3);
            
            /* Call at block end */
            i4 = abs(i3 * 2 - i5);
        }
        
        /* Nested loop with more pressure */
        for (int inner = 0; inner < 2; inner++) {
            d4 = d4 * 1.01 + d1 * 0.1;
            f5 = f5 * 0.99f + f2 * 0.2f;
            i5 = i5 + i2 * inner;
            
            /* Another function call */
            double sin_d1 = sin(d1);
            double cos_d2 = cos(d2);
            
            /* More inline assembly */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(i6) : "r"(i7) : "%eax", "%edx"
            );
            
            /* Recursive call with many arguments */
            double rec_result = recursive_helper(
                2, d1, d2, d3, f1, f2, f3, 
                i1, i2, i3, d4, f4, i4, i5
            );
            
            d1 += rec_result * 0.01;
        }
        
        alternate_path:
        /* More arithmetic after label */
        d3 = d3 * 1.5 - d2;
        f4 = f4 / 2.0f + f3;
        i6 = i6 | i5 & i4;
        
        /* Another call with different arguments */
        printf("d3=%.3f, f4=%.3f, i6=%d\n", d3, f4, i6);
        
        /* Call to external math function */
        d4 = exp(d3 * 0.1) + pow(d2, 1.1);
    }
    
    /* Final computation using all variables */
    double checksum = d1 + d2 + d3 + d4 + 
                     f1 + f2 + f3 + f4 + f5 + 
                     i1 + i2 + i3 + i4 + i5 + i6 + i7;
    
    /* One more call before return */
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)(checksum * 1000) % 1000;
}
