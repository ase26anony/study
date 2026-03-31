/* caller-save-test.c
 * Designed to trigger uncovered lines in caller-save.cc (lines 905-913)
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -lm -o caller-save-test
 */

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
    
    /* Mix of arithmetic operations */
    double t1 = a * b + c / d;
    float t2 = e * f - g + h;
    int t3 = i ^ j + k * l;
    double t4 = m / n + depth;
    
    /* Function call within recursion */
    double result = sin(t1) + cos(t2) + pow(t3, 2.0);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "add %1, %0\n\t"
        "imul %2, %0\n\t"
        : "+r"(t3), "+r"(t4)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with shuffled arguments */
    return result + recursive_helper(depth - 1,
                                     b, t1, d, t2,
                                     f, t3, t4, h,
                                     l, j, k, i,
                                     m, n);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Label for goto to create unusual basic block structure */
    start_loop:
    
    /* Loop creating register pressure */
    for (int iter = 0; iter < 4; iter++) {
        /* Arithmetic operations on all variables before calls */
        v1 = v2 * v3 + iter;
        v2 = v3 ^ v4 - v5;
        v3 = v4 + v5 * iter;
        v4 = v5 / (v1 + 1);
        v5 = v1 % (v2 + 1);
        
        f1 = f2 * f3 + sinf(f4);
        f2 = f3 / f4 - cosf(f5);
        f3 = f4 + f5 * iter;
        f4 = f5 - f1 * 0.5f;
        f5 = f1 + f2 * 2.0f;
        
        d1 = d2 * d3 + sin(d4);
        d2 = d3 / d4 - cos(d5);
        d3 = d4 + d5 * iter;
        d4 = d5 - d1 * 0.5;
        d5 = d1 + d2 * 2.0;
        
        /* Inline assembly clobbering multiple caller-saved registers */
        asm volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r"(v1)
            : "r"(v2), "r"(v3)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Function call 1 - external function with many arguments */
        printf("Iter %d: v1=%d, f1=%.2f, d1=%.2f\n", 
               iter, v1, f1, d1);
        
        /* More arithmetic between calls */
        double temp = d1 * d2 + d3 - d4 / d5;
        float ftemp = f1 * f2 + f3 - f4 / f5;
        
        /* Function call 2 - math function */
        double sin_result = sin(temp) + cos(ftemp);
        
        /* Conditional branch where both paths contain function calls */
        if (iter % 2 == 0) {
            /* Path 1: Function call at end of basic block (before goto) */
            double pow_result = pow(d1, d2) + powf(f1, f2);
            checksum += pow_result;
            
            /* This call is at the end of the basic block */
            printf("Even iter pow result: %.4f\n", pow_result);
            goto special_path;  /* Creates basic block boundary after call */
        } else {
            /* Path 2: Multiple calls in sequence */
            int rand_val = rand() % 100;
            checksum += rand_val;
            
            /* Call at end of this basic block too */
            printf("Odd iter rand: %d\n", rand_val);
            
            /* Another call - this creates different insertion points */
            double log_result = log(fabs(d3) + 1.0);
            checksum += log_result;
        }
        
        /* Label to jump back from special path */
        continue_loop:
        
        /* Function call 3 - recursive function with many live variables */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, f1, f2,
            v1, v2, d3, f3,
            v3, d4, f4, v4,
            d5, f5
        );
        
        checksum += rec_result;
        
        /* More arithmetic after recursive call */
        v1 = (v1 + v2) * (v3 - v4);
        f1 = (f1 + f2) * (f3 - f4);
        d1 = (d1 + d2) * (d3 - d4);
        
        /* Inline assembly between calls */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "+r"(v5)
            : "r"(v1), "r"(iter)
            : "%eax", "cc"
        );
        
        /* Function call 4 - another external call */
        printf("Recursive result: %.4f\n", rec_result);
        
        continue;  /* Skip special_path for normal iterations */
        
        /* Special path label */
        special_path:
        /* This block has a different call pattern */
        double special_val = tan(d1) * atan(d2);
        checksum += special_val;
        printf("Special path value: %.4f\n", special_val);
        
        /* Jump back to main loop */
        goto continue_loop;
    }
    
    /* Final computation and output to prevent dead code elimination */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    printf("Final checksum: %.10f\n", checksum);
    
    /* One more conditional with call at block end */
    if (checksum > 1000.0) {
        printf("Large checksum detected!\n");
        /* Call at end of basic block */
        return 0;
    } else {
        printf("Normal checksum\n");
        /* Different call at end of this basic block */
        return 1;
    }
}
