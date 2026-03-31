#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, float c, float d,
                       int e, int f, double g, float h, int i,
                       double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix operations to keep values live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * j + k * l;
    
    /* Call external functions within recursion */
    double s1 = sin(a + depth);
    double s2 = pow(b, 1.0 / depth);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r"(e), "+r"(f)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    return recursive_helper(depth - 1,
                           t1 + s1, t1 - s2, t2 + s1, t2 - s2,
                           e + depth, f - depth, t1 * s1, t2 * s2,
                           t3, j * depth, k * depth, l * depth,
                           m * s1, n * s2);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop creating register pressure */
    for (int iter = 0; iter < 4; iter++) {
        /* Arithmetic on all variables before calls */
        v1 = v2 * v3 + iter;
        v2 = v3 - v4 * iter;
        v3 = v4 + v5 / (iter + 1);
        v4 = v5 ^ v1;
        v5 = v1 | v2;
        
        f1 = f2 * f3 + iter;
        f2 = f3 - f4 * iter;
        f3 = f4 + f5 / (iter + 1);
        f4 = f5 * f1;
        f5 = f1 / (f2 + 0.001f);
        
        d1 = d2 * d3 + iter;
        d2 = d3 - d4 * iter;
        d3 = d4 + d5 / (iter + 1);
        d4 = d5 * d1;
        d5 = d1 / (d2 + 0.001);
        
        /* Call external function with many live variables */
        printf("Iter %d: v1=%d f1=%.2f d1=%.2f\n", 
               iter, v1, f1, d1);
        
        /* Inline assembly clobbering caller-saved registers */
        asm volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "imul %3, %%ecx"
            : "=r"(v1), "+r"(v2)
            : "r"(iter), "r"(v3)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Conditional branch with calls in both paths */
        if (iter % 2 == 0) {
            /* Path 1: Multiple function calls */
            double s1 = sin(d1 + d2);
            double p1 = pow(d3, d4);
            
            /* Call at end of basic block before goto */
            printf("Even: sin=%.3f pow=%.3f\n", s1, p1);
            goto process_block;
        } else {
            /* Path 2: Different call pattern */
            float rand_val = (float)rand() / RAND_MAX;
            double log_val = log(fabs(d5) + 1.0);
            
            /* Another call at block end */
            printf("Odd: rand=%.3f log=%.3f\n", rand_val, log_val);
            /* Fall through */
        }
        
        /* Label creating basic block boundary */
        mid_block:
        /* More arithmetic keeping variables live */
        v1 = v1 + v2 * v3;
        f1 = f1 + f2 * f3;
        d1 = d1 + d2 * d3;
        
        /* Call recursive function with many arguments */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, f5
        );
        
        printf("Recursive result: %.3f\n", rec_result);
        
        /* Skip the process_block sometimes */
        if (iter < 2) {
            continue;
        }
        
        process_block:
        /* Function call right before another label (potential BB_END) */
        double cos_val = cos(d1 * d2);
        printf("Cos: %.3f\n", cos_val);
        after_call:
        
        /* More operations after call */
        v5 = v5 + (int)(cos_val * 100);
        f5 = f5 + (float)cos_val;
        d5 = d5 + cos_val;
        
        /* Another inline assembly block */
        asm volatile (
            "add %1, %0\n\t"
            "sub %2, %0"
            : "+r"(v3), "+r"(v4)
            : "r"(iter)
            : "%eax", "%ecx", "cc"
        );
        
        /* Jump back to create interesting control flow */
        if (iter == 1) {
            goto mid_block;
        }
    }
    
    /* Compute final checksum using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* Final printf call with many live variables */
    printf("Final checksum: %.6f\n", checksum);
    printf("v1=%d v2=%d v3=%d v4=%d v5=%d\n", v1, v2, v3, v4, v5);
    printf("f1=%.2f f2=%.2f f3=%.2f f4=%.2f f5=%.2f\n", f1, f2, f3, f4, f5);
    printf("d1=%.2f d2=%.2f d3=%.2f d4=%.2f d5=%.2f\n", d1, d2, d3, d4, d5);
    
    return 0;
}
