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
    
    /* Mix arithmetic operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + j;
    int t3 = f ^ g | h;
    double t4 = i + l * 2.0;
    float t5 = m * 3.14f;
    
    /* Call external functions within recursion */
    double s1 = sin(t1);
    double s2 = pow(t4, 1.5);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r"(t3)
        : "r"(k), "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with modified arguments */
    return s1 + s2 + recursive_helper(depth - 1, 
                                      t1 + s1, b - s2, c * 0.5,
                                      d + t2, e * 0.8f,
                                      f + t3, g - depth, h ^ 0xFF,
                                      i / 2.0, j * 1.1f, k * 3,
                                      l + s1, m + t5);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.5, d3 = 3.14159, d4 = 4.2, d5 = 5.75;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    
    /* Initialize random seed */
    srand(time(NULL));
    
    double total = 0.0;
    int outer_iterations = 3;
    
    /* Start of complex control flow with goto labels */
    start_loop:
    
    for (int iter = 0; iter < outer_iterations; iter++) {
        /* Create register pressure with arithmetic on all variables */
        d1 = d1 * 1.1 + d2;
        d2 = d2 / 1.05 - d3;
        d3 = sin(d3) + d4;
        d4 = d4 * 0.99 + d5;
        d5 = pow(d5, 1.01);
        
        f1 = f1 + f2 * 0.5f;
        f2 = f2 - f3 / 1.1f;
        f3 = f3 * 1.2f + f4;
        f4 = f4 / 1.3f - f5;
        f5 = f5 * 2.0f;
        
        i1 = i1 + i2;
        i2 = i2 * i3;
        i3 = i3 ^ i4;
        i4 = i4 | i5;
        i5 = i5 & i6;
        i6 = i6 << 2;
        
        /* Conditional branch - both paths contain function calls */
        if (iter % 2 == 0) {
            /* Path 1: Call external functions with mixed arguments */
            printf("Iteration %d: d1=%.3f, f1=%.3f, i1=%d\n", 
                   iter, d1, f1, i1);
            
            /* Call at end of basic block before goto */
            double p1 = pow(d1, f1);
            total += p1;
            
            /* Inline assembly clobbering caller-saved registers */
            __asm__ volatile (
                "mov %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "=r"(i1)
                : "r"(i2), "r"(i3)
                : "%eax", "%ecx", "%edx", "cc"
            );
            
            /* Function call right before label (potential BB_END) */
            double s1 = sin(d2 + d3);
            total += s1;
            goto after_call;  /* Creates basic block boundary */
        } else {
            /* Path 2: Different call pattern */
            /* Call external function with many live variables */
            double r1 = rand() / (double)RAND_MAX;
            d1 = d1 * r1;
            
            /* Multiple arithmetic operations keeping vars live */
            f2 = f2 + (float)sin(d3);
            i2 = i2 * (int)(d4 * 100);
            
            /* Another function call */
            printf("Alt path: r1=%.3f, d1=%.3f\n", r1, d1);
            
            /* Call at end of basic block */
            double p2 = pow(d2, 2.0);
            total += p2;
            continue;  /* Different control flow */
        }
        
        after_call:
        
        /* More arithmetic after label */
        d3 = d3 * 1.01;
        f3 = f3 / 1.02f;
        
        /* Nested loop to extend live ranges */
        for (int inner = 0; inner < 2; inner++) {
            /* More operations using all variables */
            d4 = d4 + sin(d5) * 0.1;
            f4 = f4 + cosf(f5) * 0.2f;
            i3 = i3 + rand() % 100;
            
            /* Call recursive function with many arguments */
            double rec_result = recursive_helper(
                2,  /* depth */
                d1, d2, d3, f1, f2, i1, i2, i3, d4, f3, i4, d5, f4
            );
            total += rec_result;
            
            /* Inline assembly between calls */
            __asm__ volatile (
                "addl $1, %0\n\t"
                "subl $1, %1"
                : "+r"(i5), "+r"(i6)
                :
                : "%eax", "cc"
            );
            
            /* Another external function call */
            double s2 = sin(total);
            total = total * 0.99 + s2;
        }
        
        /* Function call with arguments from many live variables */
        printf("Progress: total=%.3f, d1=%.3f, f1=%.3f, i1=%d, i2=%d\n",
               total, d1, f1, i1, i2);
    }
    
    /* Final computation using all variables */
    double checksum = d1 + d2 + d3 + d4 + d5 +
                     f1 + f2 + f3 + f4 + f5 +
                     i1 + i2 + i3 + i4 + i5 + i6 +
                     total;
    
    printf("Final checksum: %.6f\n", checksum);
    
    /* One more conditional with call at block end */
    if (checksum > 1000.0) {
        printf("Large checksum detected!\n");
        /* Call at the very end of basic block */
        double final_sin = sin(checksum);
        return (int)(final_sin * 100);
    } else {
        printf("Normal checksum\n");
        /* Alternative call at block end */
        double final_cos = cos(checksum);
        return (int)(final_cos * 100);
    }
}
