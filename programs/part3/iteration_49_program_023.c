/* caller-save-test.c */
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
    
    /* Mix arithmetic to keep values live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * l + (int)(j * k);
    double t4 = m * n + depth;
    
    /* External function call within recursion */
    double s1 = sin(t1);
    double s2 = pow(t4, 2.0);
    
    /* Inline assembly that clobbers caller-saved registers */
    int asm_var = t3;
    __asm__ volatile (
        "addl %1, %0\n\t"
        "movl %0, %%eax\n\t"
        "movl $0, %%ecx\n\t"
        "addl %%eax, %%ecx"
        : "+r"(asm_var)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with shuffled arguments */
    return recursive_helper(depth - 1,
                           b, s1, d, t2,
                           f, asm_var, h, s2,
                           l, k, j, i,
                           n, m) + t1 + t4;
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Loop creating register pressure */
    for (int outer = 0; outer < 4; outer++) {
        /* Arithmetic on all variables before calls */
        v1 = v2 * v3 + outer;
        v2 = v4 - v5 * outer;
        v3 = v1 ^ v2;
        v4 = v5 + rand() % 100;
        v5 = v3 * v4 / (outer + 1);
        
        f1 = f2 * f3 + outer;
        f2 = f4 / (f5 + 0.1f);
        f3 = sinf(f1) * cosf(f2);
        f4 = f5 * 1.5f - outer;
        f5 = f3 + f4 * 0.7f;
        
        d1 = d2 * d3 + outer;
        d2 = pow(d4, d5 * 0.1);
        d3 = sin(d1) * cos(d2);
        d4 = d5 * 2.0 - outer;
        d5 = d3 + d4 / 3.14159;
        
        /* Conditional branch - path 1 */
        if (outer % 2 == 0) {
            /* Function call at end of basic block (just before goto) */
            printf("Even iteration: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
            goto compute_block;
        } else {
            /* Different function call pattern */
            double power_result = pow(d2, f1);
            printf("Odd iteration: pow=%.2f\n", power_result);
            /* Another call at block end */
            checksum += sin(power_result);
            goto loop_tail;
        }
        
compute_block:
        /* Block with inline assembly clobbering registers */
        int asm_temp = v1 + v2;
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "addl %%eax, %0\n\t"
            "movl $0, %%ecx\n\t"
            "movl $0, %%edx"
            : "+r"(asm_temp)
            : "r"(v3), "r"(v4)
            : "%eax", "%ecx", "%edx", "cc"
        );
        v5 = asm_temp;
        
        /* Call external function with many arguments */
        double trig_sum = sin(d1) + cos(d2) + tan(d3 * 0.1);
        checksum += trig_sum;
        
        /* Call recursive function with all live variables */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, f1, f2,
            v1, v2, d3, f3,
            v3, d4, f4, v4,
            d5, f5
        );
        checksum += rec_result;
        
        /* Another external call */
        double log_result = log(fabs(d4) + 1.0);
        checksum += log_result;
        
loop_tail:
        /* More arithmetic keeping variables live */
        f1 = f1 * 1.01f + checksum;
        d1 = d1 * 1.01 + checksum;
        v1 = v1 + (int)checksum;
        
        /* Inline assembly clobbering different registers */
        float asm_float = f2;
        __asm__ volatile (
            "movss %1, %%xmm0\n\t"
            "mulss %2, %%xmm0\n\t"
            "addss %%xmm0, %0\n\t"
            "movl $0, %%eax"
            : "+x"(asm_float)
            : "x"(f3), "x"(f4)
            : "%xmm0", "%eax", "cc"
        );
        f5 = asm_float;
        
        /* Function call right before loop end (potential BB_END) */
        if (outer < 3) {
            printf("Progress: %.2f\n", checksum);
            /* This call is at the end of a basic block before loop increment */
        }
    }
    
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    /* One more external call */
    checksum = fabs(checksum);
    
    /* Print final result to prevent dead code elimination */
    printf("Final checksum: %.15f\n", checksum);
    
    return (int)checksum % 100;
}
