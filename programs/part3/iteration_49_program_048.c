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
    
    /* Mix up the arguments to create register pressure */
    double t1 = a * b - c / d + e % (f + 1);
    float t2 = c * d - h * k;
    int t3 = e ^ f ^ i ^ l ^ n;
    double t4 = g + j + m + t1;
    
    /* Function call within recursion */
    if (depth % 2 == 0) {
        printf("Depth %d: t1=%.3f t2=%.3f\n", depth, t1, t2);
    }
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "+r" (t3)
        : "r" (depth)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    return recursive_helper(depth - 1,
                           t1, b + 0.5, t2, d * 1.1f,
                           t3, f + depth, t4, h * 0.9f, i ^ depth,
                           j * 1.01, k * 0.99f, l + depth, m - 0.1, n * 2);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop creating register pressure */
    for (int outer = 0; outer < 4; outer++) {
        /* Complex arithmetic on all variables before call */
        v1 = v2 * v3 + outer;
        v2 = v3 ^ v4 ^ outer;
        v3 = v4 % (v5 + 1) + outer;
        v4 = v5 * 2 - outer;
        v5 = v1 + v2 + v3 + v4;
        
        f1 = f2 * 1.1f + outer * 0.1f;
        f2 = f3 / 1.2f - outer * 0.2f;
        f3 = f4 + f5 * outer;
        f4 = f1 * f2 - f3;
        f5 = f4 / (fabs(f3) + 1.0f);
        
        d1 = sin(d2) + outer * 0.01;
        d2 = cos(d3) * 1.01;
        d3 = d4 * d5 - pow(1.1, outer);
        d4 = sqrt(fabs(d1) + fabs(d2));
        d5 = d3 * 2.0 - d4;
        
        /* Function call with many live variables */
        printf("Iteration %d: v1=%d f1=%.3f d1=%.3f\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly clobbering caller-saved regs */
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "+r" (v1)
            : "r" (v2), "r" (outer)
            : "%eax", "%ecx", "%edx", "memory"
        );
        
        /* Conditional branch - both paths have calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d1 = sin(d2) + cos(d3);
            d2 = pow(d4, 1.5);
            /* Function call right before goto */
            printf("Even path: d1=%.3f d2=%.3f\n", d1, d2);
            goto special_block;
        } else {
            /* Path 2: Multiple calls in block */
            f1 = f2 * f3 + f4;
            /* Call to external function */
            double rand_val = (double)rand() / RAND_MAX;
            d3 = d4 * rand_val;
            /* Another call */
            printf("Odd path: f1=%.3f d3=%.3f\n", f1, d3);
            /* Continue normally */
        }
        
        /* More arithmetic after call */
        v1 = v1 + v2 * 3;
        v2 = v2 ^ (v3 << 2);
        
        /* Recursive call with many arguments */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, f1, f2, 
            v1, v2, d3, f3, v3,
            d4, f4, v4, d5, v5
        );
        
        printf("Recursive result: %.3f\n", rec_result);
        
        continue;
        
special_block:
        /* Unusual basic block structure with goto */
        /* Function call at end of this block */
        f5 = tan(f1) * 0.5f;
        v5 = (int)(f5 * 100) % 256;
        printf("Special block: f5=%.3f v5=%d\n", f5, v5);
        /* This call is at block end before jumping back */
        d5 = fmod(d1, 3.14);
    }
    
    /* Another conditional with call at block end */
    int final_check = v1 + v2 + v3;
    if (final_check > 100) {
        d1 = d2 * d3 - d4;
        f1 = f2 + f3 * f4;
        /* Call at the end of basic block */
        printf("High sum: %.3f %.3f\n", d1, f1);
    } else {
        v4 = v5 * 2 + 1;
        /* Different call pattern */
        double power_result = pow(d5, 2.0);
        printf("Low sum: %d %.3f\n", v4, power_result);
    }
    
    /* Compute final checksum using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* Final call with result */
    printf("Final checksum: %.6f\n", checksum);
    
    /* One more inline assembly before return */
    __asm__ volatile (
        "mov %0, %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "+r" (v1)
        :
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    return (int)checksum % 256;
}
