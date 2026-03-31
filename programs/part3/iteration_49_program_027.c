#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                        int e, int f, double g, float h, int i, 
                        double j, float k, int l, double m, int n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix arithmetic operations to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * l + n * depth;
    double t4 = j * m + k * depth;
    
    /* Call external functions within recursion */
    double s1 = sin(t1);
    double s2 = pow(t4, 1.5);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "mov %0, %%eax\n\t"
        "mov %1, %%ecx\n\t"
        "add %%ecx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "+r"(t3)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    /* Recursive call with mixed arguments */
    return recursive_helper(depth - 1, 
                           t1 + s1, b, t2, d, 
                           t3, f, g + s2, h, i, 
                           j, k, l, m, n) * 0.9;
}

int main() {
    /* Declare and initialize many local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables before calls */
        v1 = v2 * v3 + outer;
        v2 = v4 - v5 * outer;
        v3 = v1 ^ v2;
        f1 = f2 * f3 + outer;
        f2 = f4 / (f5 + 0.1f);
        f3 = f1 - f2 * outer;
        d1 = d2 * d3 + sin(outer);
        d2 = d4 / (d5 + 0.01);
        d3 = d1 * d2 - outer;
        
        /* First function call with subset of variables */
        printf("Iteration %d: v1=%d, f1=%.2f, d1=%.2f\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly clobbering multiple caller-saved registers */
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "mov %1, %%ecx\n\t"
            "mov %2, %%edx\n\t"
            "imul %%ecx, %%eax\n\t"
            "add %%edx, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "+r"(v1)
            : "r"(v2), "r"(outer)
            : "%eax", "%ecx", "%edx", "memory"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d4 = pow(d1, d2) + cos(f1);
            /* Function call right before label (potential BB_END) */
            printf("Even: pow=%.3f\n", d4);
            goto process_block;
        } else {
            /* Path 2: Different call pattern */
            f4 = tan(f2) * f3;
            printf("Odd: tan=%.3f\n", f4);
            /* Another call before goto */
            d5 = fmod(d3, 2.5);
        }
        
        /* Second external function call */
        double rand_val = (double)rand() / RAND_MAX;
        d1 = d1 * rand_val + d2;
        
        /* Call recursive function with many live arguments */
        double rec_result = recursive_helper(2, d1, d2, f1, f2,
                                            v1, v2, d3, f3, v3,
                                            d4, f4, v4, d5, v5);
        
        /* More arithmetic after call */
        v4 = (int)(rec_result * 100) % 100;
        f5 = f1 * f2 - f3 + f4;
        
        /* Third function call */
        printf("Recursive result: %.3f\n", rec_result);
        
        /* Use goto to create unusual basic block structure */
        if (outer == 1) {
            goto special_case;
        }
        
    process_block:
        /* Block with function call at the end */
        v5 = v1 + v2 * v3 - v4;
        /* This call could be at BB_END */
        printf("Process: v5=%d\n", v5);
        continue;
        
    special_case:
        /* Different block with its own call pattern */
        d3 = sqrt(d1 * d2);
        printf("Special: sqrt=%.3f\n", d3);
        /* Another potential BB_END call */
        f3 = fabs(f1 - f2);
    }
    
    /* Final computation using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* Final function call */
    printf("Final checksum: %.6f\n", checksum);
    
    /* Additional arithmetic after final call to extend live ranges */
    checksum = checksum * 1.01;
    
    return (int)checksum % 100;
}
