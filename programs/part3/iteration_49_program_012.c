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
    
    /* Mix in some external function calls */
    double sin_val = sin(a + depth);
    double pow_val = pow(b, 1.5);
    float abs_c = fabsf(c);
    
    /* Inline assembly that clobbers caller-saved registers */
    int temp_e = e;
    int temp_f = f;
    asm volatile (
        "addl %1, %0\n\t"
        "imull $2, %0"
        : "+r"(temp_e)
        : "r"(temp_f)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           a * sin_val, 
                           b + pow_val, 
                           c * abs_c, 
                           d + depth,
                           temp_e, 
                           f * 2,
                           g + sin_val,
                           h * 1.1f,
                           i + depth,
                           j / (depth + 1),
                           k * 0.9f,
                           l ^ depth,
                           m - 0.5,
                           n + 0.2f);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables before calls */
        v1 = v2 * v3 + outer;
        v2 = v4 - v5 * outer;
        v3 = v1 ^ v2;
        v4 = v5 + v1 * outer;
        v5 = v3 | v4;
        
        f1 = f2 * f3 + outer;
        f2 = f4 / (f5 + 0.1f);
        f3 = sinf(f1) * 2.0f;
        f4 = fabsf(f2 - f3);
        f5 = f4 * f1 + outer;
        
        d1 = d2 * d3 + outer;
        d2 = pow(d4, 1.0 + outer * 0.1);
        d3 = sin(d1) * cos(d2);
        d4 = d5 / (d3 + 1.0);
        d5 = d1 + d2 * d3;
        
        /* First function call with subset of variables */
        printf("Iteration %d: v1=%d, f1=%.2f, d1=%.2f\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly clobbering multiple caller-saved registers */
        int asm_temp1 = v1;
        int asm_temp2 = v2;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            "imull $3, %0"
            : "+r"(asm_temp1)
            : "r"(asm_temp2)
            : "%eax", "%ecx", "%edx", "cc"
        );
        v1 = asm_temp1;
        
        /* Conditional branch - path 1 */
        if (outer % 2 == 0) {
            /* Call at end of basic block before goto */
            double result = pow(d1, d2) + sin(d3);
            checksum += result;
            goto compute_block;  /* Creates basic block boundary */
        }
        
        /* Second function call - different arguments */
        float rand_val = (float)rand() / RAND_MAX;
        printf("Random: %.4f, f2=%.2f, f3=%.2f\n", rand_val, f2, f3);
        
        /* More arithmetic */
        v1 = v1 + (int)(rand_val * 100);
        f1 = f1 * rand_val;
        d1 = d1 * rand_val;
        
        compute_block:
        /* Third function call - to math library */
        double trig_result = sin(d4) + cos(d5) + tan(d1 * 0.5);
        checksum += trig_result;
        
        /* Recursive call with many live variables */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, f5
        );
        checksum += rec_result;
        
        /* Inline assembly between calls */
        float asm_float = f3;
        asm volatile (
            "addss %1, %0\n\t"
            "mulss %0, %0"
            : "+x"(asm_float)
            : "x"(f4)
            : "cc"
        );
        f3 = asm_float;
        
        /* Fourth function call - another external */
        printf("Checksum so far: %.6f\n", checksum);
        
        /* Nested inner loop for more pressure */
        for (int inner = 0; inner < 2; inner++) {
            /* All variables used in complex expressions */
            v5 = v5 + v1 * inner - v2 / (inner + 1);
            f5 = f5 * (1.0f + sinf(f3 * inner));
            d5 = d5 + pow(d4, inner * 0.5);
            
            /* Function call inside nested loop */
            if (inner == 0) {
                double mod_result = fmod(d3, 2.0);
                checksum += mod_result;
            }
        }
        
        /* Another conditional with call at block end */
        if (checksum > 100.0) {
            v1 = v1 * 2;
            f1 = f1 * 1.5f;
            d1 = d1 * 1.2;
            /* Call at the end of basic block */
            printf("Large checksum adjustment: %.2f\n", checksum);
        } else {
            v1 = v1 / 2;
            f1 = f1 * 0.8f;
            d1 = d1 * 0.9;
            /* Different call at end of else block */
            double sqrt_val = sqrt(fabs(d1));
            checksum -= sqrt_val;
        }
    }
    
    /* Final computation using all variables */
    double final_result = 
        v1 + v2 + v3 + v4 + v5 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5 +
        checksum;
    
    printf("Final result: %.12f\n", final_result);
    
    return (int)final_result % 256;
}
