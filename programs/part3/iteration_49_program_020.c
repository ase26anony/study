#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, float f,
                        int g, int h, int i, double j, float k, int l, int m, int n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix arithmetic operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + f;
    int t3 = g ^ h | i;
    double t4 = j + k * 2.0;
    int t5 = l * m - n;
    
    /* Call external functions within recursion */
    double s1 = sin(t1);
    double s2 = pow(t4, 1.5);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0"
        : "+r" (t5)
        : "r" (depth)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    /* Recursive call with mixed arguments */
    return recursive_helper(depth - 1, t1 + s1, b - s2, c * 2.0,
                           t2 * 1.5f, e + 0.5f, f - 0.3f,
                           t3 ^ 0xAA, h + 1, i * 2,
                           j / 2.0, k + 1.0f, t5, m % 7, n + depth);
}

int main() {
    /* Declare and initialize many local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Arithmetic on all variables before calls */
        v1 = v2 * v3 + outer;
        v4 = v5 ^ v6;
        v2 = v3 | v4;
        
        f1 = f2 * 1.1f - f3;
        f4 = f5 / 2.0f + outer * 0.5f;
        f3 = sinf(f4) + f1;
        
        d1 = d2 * 1.5 - d3;
        d4 = pow(d5, 1.1) + outer * 0.1;
        d3 = d1 / 2.0 + d4;
        
        /* First function call with mixed arguments */
        printf("Iteration %d: v1=%d, f1=%.2f, d1=%.2f\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly clobbering multiple caller-saved registers */
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %3, %%ecx\n\t"
            "add %%eax, %%ecx\n\t"
            "mov %%ecx, %3"
            : "+r" (v1), "+r" (v4)
            : "r" (v2), "r" (v3)
            : "%eax", "%ecx", "%edx", "memory"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block before goto */
            d2 = sin(d1) * cos(d3);
            f2 = powf(f3, 1.5f);
            v5 = rand() % 100 + v1;
            
            /* Function call right before label (potential BB_END) */
            printf("Even: d2=%.3f, f2=%.3f\n", d2, f2);
            goto process_block;  /* Creates unusual BB structure */
        } else {
            /* Path 2: Multiple calls in sequence */
            d2 = tan(d1 + d3);
            f2 = logf(f3 * 2.0f);
            v5 = abs(v1 - v3) * 2;
            
            printf("Odd: d2=%.3f\n", d2);
            d4 = pow(d2, 2.0) + d3;
        }
        
        /* Target label for goto */
        process_block:
        
        /* More arithmetic to keep variables live across calls */
        v6 = v1 * v2 - v3 + v4;
        f5 = f1 + f2 * f3 - f4;
        d5 = d1 * 0.3 + d2 * 0.7 - d3;
        
        /* Call external math function */
        double trig_sum = sin(d1) + cos(d2) + tan(d3 * 0.5);
        
        /* Another inline assembly block */
        __asm__ volatile (
            "mov %0, %%eax\n\t"
            "add %1, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %2, %%edx\n\t"
            "sub %%eax, %%edx\n\t"
            "mov %%edx, %2"
            : "+r" (v6), "+r" (v5)
            : "r" (v4)
            : "%eax", "%edx", "memory"
        );
        
        /* Call recursive function with many live arguments */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1 + trig_sum, d2, d3, 
            f1, f2, f3,
            v1, v2, v3,
            d4, f4, v4, v5, v6
        );
        
        /* Function call with complex arguments */
        printf("Recursive result: %.4f\n", rec_result);
        
        /* Update checksum with all variables */
        checksum += v1 + v2 + v3 + v4 + v5 + v6
                  + f1 + f2 + f3 + f4 + f5
                  + d1 + d2 + d3 + d4 + d5
                  + rec_result;
        
        /* Another conditional with call at block end */
        if (checksum > 1000.0) {
            d1 = sqrt(checksum);
            printf("Large checksum: %.2f\n", d1);
            /* Call at potential BB_END */
        } else {
            d1 = log(fabs(checksum) + 1.0);
            v1 = (int)d1 * 2;
        }
        
        /* Final call in iteration */
        printf("Running checksum: %.2f\n", checksum);
    }
    
    /* Compute final result using all variables */
    double final_result = 
        (v1 * 0.01) + (v2 * 0.02) + (v3 * 0.03) +
        (v4 * 0.04) + (v5 * 0.05) + (v6 * 0.06) +
        f1 + f2 * 2.0f + f3 * 3.0f + f4 * 4.0f + f5 * 5.0f +
        d1 / 10.0 + d2 / 20.0 + d3 / 30.0 + d4 / 40.0 + d5 / 50.0;
    
    printf("Final result: %.10f\n", final_result);
    
    return (int)(final_result * 1000) % 256;
}
