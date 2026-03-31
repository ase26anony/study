#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
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
    
    /* Function call within recursion */
    printf("Depth %d: t1=%.3f t2=%.3f t3=%d\n", depth, t1, t2, t3);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r"(t3)
        : "r"(f), "r"(g)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    return recursive_helper(depth - 1, 
                           t1 + a, t4 + b, c * 0.5, 
                           t2 + d, e * 1.1f,
                           t3, g + 1, h - 1,
                           sin(i), cos(j), k * 2,
                           l * 0.9, t5) * 0.95;
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    
    srand(time(NULL));
    
    /* Label for goto to create unusual basic block structure */
    loop_start:
    
    for (int outer = 0; outer < 3; outer++) {
        /* Perform arithmetic on all variables before calls */
        d1 = d1 * 1.1 + sin(d2);
        d2 = d2 * 0.9 + cos(d3);
        d3 = d3 * 1.05 + tan(d4);
        d4 = d4 * 0.95 + pow(d1, 1.5);
        
        f1 = f1 * 1.2f + f2;
        f2 = f2 * 0.8f + f3;
        f3 = f3 * 1.1f + f4;
        f4 = f4 * 0.9f + f1;
        
        i1 = i1 + i2 * 2;
        i2 = i2 ^ i3;
        i3 = i3 | i4;
        i4 = i4 & i5;
        i5 = i5 - i6;
        i6 = i6 * 3 + 1;
        
        /* Function call with many live variables - may be at block end */
        printf("Iteration %d: d1=%.3f d2=%.3f f1=%.3f i1=%d i2=%d\n", 
               outer, d1, d2, f1, i1, i2);
        
        /* Conditional branch where both paths have function calls */
        if (d1 > 5.0) {
            /* Path 1: Call at end of basic block */
            d1 = pow(d1, f1);
            d2 = sin(d2) * cos(d3);
            /* Function call right before label */
            printf("d1 > 5 path: %.3f\n", d1);
            goto mid_block;  /* Creates block boundary after call */
        } else {
            /* Path 2: Multiple calls with arithmetic between */
            f3 = sqrtf(fabsf(f3));
            f4 = powf(f4, 1.5f);
            /* External function call */
            double rand_val = (double)rand() / RAND_MAX;
            d3 = d3 * rand_val;
            
            /* Inline assembly clobbering caller-saved registers */
            asm volatile (
                "mov %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                "imul %3, %0"
                : "=r"(i4)
                : "r"(i1), "r"(i2), "r"(i3)
                : "%eax", "%ecx", "%edx", "cc"
            );
            
            /* Another function call */
            d4 = sin(d4) + cos(d1);
        }
        
        mid_block:
        
        /* More arithmetic to keep variables live */
        d1 = d1 + 0.1;
        d2 = d2 - 0.1;
        f1 = f1 * 1.01f;
        f2 = f2 / 1.01f;
        
        /* Call recursive function with many arguments */
        double rec_result = recursive_helper(2, d1, d2, d3, f1, f2, 
                                            i1, i2, i3, d4, f3, i4, 
                                            d1 * 2.0, f4);
        
        /* Use result in further computation */
        d1 = d1 + rec_result * 0.1;
        
        /* Another external function call */
        double trig_result = sin(d2) + cos(d3) + tan(d4 * 0.01);
        
        /* Inline assembly between function calls */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0"
            : "+r"(i5)
            : "r"(i3), "r"(i4)
            : "%eax", "cc"
        );
        
        /* Function call with mixed arguments */
        printf("Mixed: d1=%.3f trig=%.3f i5=%d\n", d1, trig_result, i5);
        
        /* Conditional with call at end of block */
        if (f4 > 10.0f) {
            f4 = f4 / 2.0f;
            /* This call might be at BB_END */
            printf("Reducing f4 to %.3f\n", f4);
        }
    }
    
    /* Final computation using all variables */
    double checksum = d1 + d2 + d3 + d4 + 
                     f1 + f2 + f3 + f4 + 
                     i1 + i2 + i3 + i4 + i5 + i6;
    
    /* One more call before return */
    printf("Final checksum: %.6f\n", checksum);
    
    /* Use goto to create another block boundary opportunity */
    if (checksum < 1000.0) {
        goto loop_start;
    }
    
    return (int)checksum % 256;
}
