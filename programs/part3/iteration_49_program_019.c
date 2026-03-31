#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to create register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, double l, float m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + j;
    int t3 = f ^ g | h;
    double t4 = sin(i) * cos(l);
    float t5 = m * 2.0f;
    
    /* Function call within recursion to force caller-save */
    printf("Depth %d: t1=%.3f t2=%.3f\n", depth, t1, t2);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r"(t3)
        : "r"(k), "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    return recursive_helper(depth - 1, 
                           t1 + 1.0, b * 0.9, c - t4,
                           d + t2, e * 1.1f,
                           f + t3, g ^ depth, h << 1,
                           i * 0.8, j + 0.3f, k * 2,
                           l * 0.95, m + 0.5f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    
    srand(time(NULL));
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables before calls */
        d1 = d1 * 1.5 + sin(d2);
        d2 = d2 * 0.9 + cos(d3);
        d3 = d3 * 1.1 + tan(d4);
        d4 = d4 * 0.8 + pow(d1, 1.5);
        
        f1 = f1 * 1.2f + f2;
        f2 = f2 * 0.85f + f3;
        f3 = f3 * 1.15f + f4;
        f4 = f4 * 0.9f + f1 * 0.5f;
        
        i1 = i1 + i2 * 2;
        i2 = i2 ^ i3;
        i3 = i3 | i4;
        i4 = i4 & i5;
        i5 = i5 << 1;
        i6 = i6 >> 1;
        
        /* First function call with many live variables */
        printf("Outer=%d: d1=%.3f f1=%.3f i1=%d\n", outer, d1, f1, i1);
        
        /* Inline assembly clobbering caller-saved registers */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "imul %3, %0"
            : "=r"(i1)
            : "r"(i2), "r"(i3), "r"(outer)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (d1 > 5.0) {
            /* Path 1: Call at end of basic block */
            d2 = pow(d1, d3) + sin(d4);
            f3 = f1 * f2 + f4;
            /* Function call right before goto (potential BB_END) */
            printf("Path1: d2=%.3f f3=%.3f\n", d2, f3);
            goto special_block;
        } else {
            /* Path 2: Multiple calls in sequence */
            d3 = log(fabs(d1)) + exp(d2);
            printf("Path2: d3=%.3f\n", d3);
            
            /* Another call with mixed arguments */
            double result = pow(d1, 2.0) + pow(d2, 2.0);
            printf("Result=%.3f\n", result);
            
            /* Call external math function */
            d4 = sin(d3) * cos(d2);
        }
        
        /* More arithmetic to keep variables live across calls */
        f1 = f1 + sin(d1) * 2.0f;
        f2 = f2 + cos(d2) * 1.5f;
        
        /* Call recursive function with many arguments */
        double rec_result = recursive_helper(2, d1, d2, d3, f1, f2, 
                                           i1, i2, i3, d4, f3, i4, 
                                           d1 + d2, f4);
        
        /* Use goto to create unusual control flow */
        if (rec_result > 100.0) {
            goto compute_more;
        }
        
        special_block:
        /* Block entered via goto - has function call at end */
        i5 = i5 + rand() % 100;
        i6 = i6 - rand() % 50;
        /* Function call potentially at BB_END */
        printf("Special: i5=%d i6=%d\n", i5, i6);
        
        compute_more:
        /* More operations and calls */
        d1 = d1 * 0.99;
        d2 = d2 + d3 * 0.1;
        
        /* Another inline assembly block */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %%eax, %%eax\n\t"
            "sub %2, %%eax\n\t"
            "mov %%eax, %0"
            : "=r"(i2)
            : "r"(i3), "r"(i4)
            : "%eax", "cc"
        );
        
        /* Final call in loop iteration */
        f4 = sqrt(fabs(f1 + f2 + f3));
        printf("Loop end: f4=%.3f rec=%.3f\n", f4, rec_result);
    }
    
    /* Compute final checksum from all variables */
    double checksum = d1 + d2 + d3 + d4 + 
                     f1 + f2 + f3 + f4 + 
                     i1 + i2 + i3 + i4 + i5 + i6;
    
    printf("Final checksum: %.6f\n", checksum);
    
    /* One more conditional with call at block end */
    if (checksum > 500.0) {
        printf("Large checksum!\n");
        return 1;
    } else {
        printf("Small checksum\n");
        /* Call at end of basic block before return */
        printf("Exiting...\n");
        return 0;
    }
}
