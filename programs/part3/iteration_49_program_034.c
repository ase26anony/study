#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, int l, int m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / (e + 1.0f);
    int t3 = (f ^ g) | (h & m);
    double t4 = i + sin(j) * 2.0;
    
    /* Function call within recursion */
    printf("Depth %d: t1=%.3f t2=%.3f t3=%d t4=%.3f\n", 
           depth, t1, t2, t3, t4);
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           t1, b + 1.0, c * 0.9, 
                           t2, e * 1.1f,
                           t3, g << 1, h >> 1,
                           t4, j + 0.5f,
                           k + l, m - 1, f % 7, g ^ h) * 0.95;
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.234567, d2 = 2.345678, d3 = 3.456789;
    float f1 = 4.56f, f2 = 5.67f, f3 = 6.78f, f4 = 7.89f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60, i7 = 70, i8 = 80;
    
    srand(time(NULL));
    
    /* Loop to create register pressure */
    for (int outer = 0; outer < 4; outer++) {
        /* Arithmetic operations before function call */
        d1 = d1 * 1.1 + sin(d2) * 0.1;
        d2 = d2 * 0.9 + cos(d3) * 0.2;
        d3 = d3 * 1.05 + tan(d1) * 0.05;
        
        f1 = f1 * 1.2f + f2 * 0.3f;
        f2 = f2 * 0.8f + f3 * 0.4f;
        f3 = f3 * 1.1f + f4 * 0.2f;
        f4 = f4 * 0.9f + f1 * 0.1f;
        
        i1 = i1 + i2 * 2 - i3;
        i2 = i2 ^ i4 | i5;
        i3 = i3 * 3 + i6 / 2;
        i4 = i4 << 1 | i7 >> 1;
        i5 = i5 + i8 % 7;
        i6 = i6 * 5 - i1;
        i7 = i7 ^ i2 & i3;
        i8 = i8 | i4 ^ i5;
        
        /* Inline assembly that clobbers caller-saved registers */
        __asm__ volatile (
            "addl %1, %0\n\t"
            "movl %0, %%eax\n\t"
            "movl %2, %%ecx\n\t"
            "imull %%ecx, %%eax\n\t"
            "movl %%eax, %0"
            : "+r"(i1)
            : "r"(i2), "r"(i3)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Function call with many live variables */
        printf("Iteration %d: d1=%.6f d2=%.6f f1=%.3f i1=%d i2=%d i3=%d\n",
               outer, d1, d2, f1, i1, i2, i3);
        
        /* Conditional branch creating basic block boundary */
        if (outer % 2 == 0) {
            /* Path 1: Function call at end of basic block */
            double result = pow(d1, d2) + pow(d3, 1.5);
            printf("Even: pow result=%.6f\n", result);
            
            /* More arithmetic to extend live ranges */
            d1 += result * 0.1;
            d2 -= result * 0.05;
            
            /* Another function call */
            float rand_val = (float)rand() / RAND_MAX;
            f3 = f3 * rand_val + sin(f4) * 0.5f;
            
            /* Call at end of basic block before goto */
            printf("Pre-goto: f3=%.3f f4=%.3f\n", f3, f4);
            goto special_block;
        } else {
            /* Path 2: Different call pattern */
            double trig_sum = sin(d1) + cos(d2) + tan(d3 * 0.1);
            printf("Odd: trig sum=%.6f\n", trig_sum);
            
            /* Inline assembly with different clobbers */
            __asm__ volatile (
                "movsd %1, %%xmm0\n\t"
                "movsd %2, %%xmm1\n\t"
                "addsd %%xmm1, %%xmm0\n\t"
                "movsd %%xmm0, %0"
                : "=r"(d3)
                : "r"(d1), "r"(d2)
                : "%xmm0", "%xmm1"
            );
            
            /* Function call at block end */
            printf("d3 updated=%.6f\n", d3);
            continue;  /* Creates BB boundary */
        }
        
special_block:
        /* Unusual basic block structure via goto */
        /* Recursive call with many arguments - high register pressure */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, d3,
            f1, f2,
            i1, i2, i3, i4,
            d1 * 0.5,  /* i */
            f3,        /* j */
            i5, i6, i7
        );
        
        printf("Recursive result=%.6f\n", rec_result);
        
        /* More arithmetic after call */
        i8 = i8 + (int)(rec_result * 1000);
        f4 = f4 + (float)rec_result * 0.01f;
        
        /* Another call to external function */
        double log_sum = log(fabs(d1) + 1.0) + log(fabs(d2) + 1.0);
        printf("Log sum=%.6f\n", log_sum);
    }
    
    /* Final computation using all variables */
    double checksum = d1 + d2 + d3 + f1 + f2 + f3 + f4 +
                     i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8;
    
    /* Mix operations for final value */
    checksum = checksum * sin(checksum * 0.01) + cos(checksum * 0.005);
    
    /* Final printf call */
    printf("Final checksum: %.12f\n", checksum);
    
    return (int)(checksum * 1000) % 1000;
}
