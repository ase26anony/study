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
    
    /* Mix up the arguments to prevent optimization */
    double temp1 = a * b - c / d + e % (f + 1);
    float temp2 = c * d - h * k + n;
    int temp3 = e ^ f ^ i ^ l;
    double temp4 = g + j + m + temp1;
    
    /* Function call in the middle of computation */
    printf("Depth %d: temp1=%.3f temp2=%.3f\n", depth, temp1, temp2);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "+r"(temp3)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    
    /* Recursive call with shuffled arguments */
    return recursive_helper(depth - 1, 
                           b, a, d, c, 
                           f, e, j, k, 
                           l, g, h, i, 
                           m, n) + temp4 + temp2 + temp3;
}

int main() {
    /* Declare and initialize many local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double total = 0.0;
    
    /* Complex loop structure to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Label for goto jumps to manipulate basic block structure */
        loop_start:
        
        /* Arithmetic operations before function call */
        v1 = v2 * v3 + outer;
        v4 = v5 ^ v6;
        f1 = f2 * f3 - f4;
        f5 = sinf(f1) + cosf(f2);
        d1 = d2 * d3 / (d4 + 1.0);
        d5 = pow(d1, 2.0) + sqrt(d2);
        
        /* Function call with many live variables */
        printf("Iteration %d: v1=%d v4=%d f1=%.3f d1=%.3f\n", 
               outer, v1, v4, f1, d1);
        
        /* More arithmetic mixing all variables */
        v2 = v1 + v3 * v4 - v5;
        v6 = abs(v2 - v4) % 7;
        f2 = f1 + f3 * f4 - f5;
        f3 = tanf(f2) * atanf(f1);
        d2 = d1 + d3 * d4 - d5;
        d3 = log(d2) + exp(d1);
        
        /* Inline assembly that clobbers caller-saved registers */
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %%eax, %2\n\t"
            : "+r"(v1), "+r"(v2)
            : "r"(v3)
            : "%eax", "%ecx", "%edx", "%esi", "%edi", "cc"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Function call at the end of basic block */
            d4 = sin(d3) * cos(d2);
            /* Call just before label (potential BB_END candidate) */
            printf("Even iteration: d4=%.3f\n", d4);
            goto mid_block;  /* Creates unusual BB structure */
        } else {
            /* Path 2: Multiple operations after function call */
            d4 = asin(d3 / 10.0) * acos(d2 / 10.0);
            printf("Odd iteration: d4=%.3f\n", d4);
            /* Arithmetic after call in same basic block */
            d5 = d4 * 2.0 - d3;
        }
        
        mid_block:
        /* More arithmetic to extend live ranges */
        v3 = v2 * v6 + rand() % 100;
        v5 = v4 ^ v1 ^ v3;
        f4 = f2 * f5 - f3;
        d5 = d3 * d4 / (d1 + 1.0);
        
        /* Call to recursive function with many arguments */
        double rec_result = recursive_helper(2, d1, d2, f1, f2,
                                           v1, v2, d3, f3, v3,
                                           d4, f4, v4, d5, f5);
        
        /* Function call after recursive call */
        printf("Recursive result: %.3f\n", rec_result);
        
        /* Another inline assembly block */
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "add %1, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %2, %%ecx\n\t"
            "sub %%ecx, %3\n\t"
            : "+r"(v5), "+r"(v6)
            : "r"(outer), "r"(v4)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Another conditional with function call at block end */
        if (v5 > 100) {
            f5 = f4 * 1.5f + f3;
            /* Function call right before return from block */
            printf("Large v5=%d, adjusting f5=%.3f\n", v5, f5);
            /* goto creates another basic block boundary */
            goto loop_end;
        }
        
        /* Final arithmetic in the loop */
        total += rec_result + v1 + v2 + v3 + v4 + v5 + v6 
                + f1 + f2 + f3 + f4 + f5 
                + d1 + d2 + d3 + d4 + d5;
        
        loop_end:
        /* Shuffle variables for next iteration */
        int tmp_i = v1; v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6; v6 = tmp_i;
        float tmp_f = f1; f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = tmp_f;
        double tmp_d = d1; d1 = d2; d2 = d3; d3 = d4; d4 = d5; d5 = tmp_d;
    }
    
    /* Final computation and output to prevent dead code elimination */
    double checksum = total + v1 + v2 + v3 + v4 + v5 + v6 
                     + f1 + f2 + f3 + f4 + f5 
                     + d1 + d2 + d3 + d4 + d5;
    
    printf("Final checksum: %.6f\n", checksum);
    return (int)checksum % 256;
}
