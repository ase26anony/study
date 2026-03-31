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
    
    /* Mix in some external function calls */
    double sin_val = sin(a + depth);
    double pow_val = pow(b, 1.5);
    
    /* Inline assembly that clobbers caller-saved registers */
    int temp_e = e;
    int temp_f = f;
    asm volatile (
        "addl %1, %0\n\t"
        "imull $7, %0\n\t"
        : "+r"(temp_e)
        : "r"(temp_f)
        : "%eax", "%ecx", "%edx", "cc"
    );
    e = temp_e;
    
    /* Call external functions with live variables */
    printf("Depth %d: sin=%.3f pow=%.3f\n", depth, sin_val, pow_val);
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           a * 1.1, b * 0.9, c * 1.05f, d * 0.95f,
                           e + 1, f - 1, g + sin_val, h * 1.1f, i * 2,
                           j / 2.0, k * 1.2f, l + depth, m - 0.5, n * 0.8f);
}

int main() {
    srand(time(NULL));
    
    /* Declare and initialize many local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    /* Variable to track control flow */
    int branch_selector = 0;
    
    /* Loop creating register pressure */
    for (int iteration = 0; iteration < 4; iteration++) {
        /* Perform arithmetic on all variables to extend live ranges */
        v1 = v1 * 2 + v2;
        v2 = v2 + v3 * iteration;
        v3 = v3 ^ v4;
        v4 = v4 * 3 - v5;
        v5 = v5 + rand() % 10;
        
        f1 = f1 * 1.1f + f2;
        f2 = f2 - f3 * 0.5f;
        f3 = f3 * f4;
        f4 = f4 / 1.3f + f5;
        f5 = f5 * 2.0f - 1.0f;
        
        d1 = d1 * 1.05 + d2;
        d2 = d2 + sin(d3);
        d3 = d3 * d4 * 0.9;
        d4 = d4 / 1.7 + cos(d5);
        d5 = d5 * 1.8 - 0.3;
        
        /* Conditional branch - both paths contain function calls */
        if (iteration % 2 == 0) {
            /* Path 1: Call external functions */
            double pow_result = pow(d1, f1);
            printf("Iteration %d, pow=%.3f\n", iteration, pow_result);
            
            /* Inline assembly clobbering caller-saved registers */
            int temp_v1 = v1;
            int temp_v2 = v2;
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                "imull $13, %0\n\t"
                : "+r"(temp_v1)
                : "r"(temp_v2)
                : "%eax", "%ecx", "%edx", "cc"
            );
            v1 = temp_v1;
            
            /* Function call at the end of basic block */
            double sin_val = sin(d2 + d3);
            /* This call is at block end before goto */
            printf("sin(%.3f)=%.3f\n", d2 + d3, sin_val);
            
            /* Use goto to create unusual basic block structure */
            if (v1 > 100) {
                goto special_path;
            }
        } else {
            /* Path 2: Different pattern of function calls */
            /* Call with many live variables as arguments */
            double result = recursive_helper(2, d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, f5);
            
            /* More arithmetic after call */
            v1 = v1 + (int)result;
            f1 = f1 + (float)fmod(result, 10.0);
            
            /* Another external function call */
            double log_val = log(fabs(d1) + 1.0);
            printf("log result=%.3f\n", log_val);
            
            /* This could be BB_END if followed by label */
            if (iteration == 3) {
                goto final_computation;
            }
        }
        
        /* Common code after conditional */
        /* Inline assembly with different clobbered registers */
        float temp_f1 = f1;
        float temp_f2 = f2;
        asm volatile (
            "movss %1, %%xmm0\n\t"
            "addss %%xmm0, %0\n\t"
            "mulss %1, %0\n\t"
            : "+x"(temp_f1)
            : "x"(temp_f2)
            : "%xmm0", "%xmm1", "cc"
        );
        f1 = temp_f1;
        
        /* Another function call */
        double cos_val = cos(d4);
        v2 = v2 + (int)(cos_val * 100);
        
        continue;
        
    special_path:
        /* Unusual basic block reached by goto */
        /* Function call with many arguments */
        printf("Special path: v1=%d v2=%d f1=%.2f d1=%.2f\n", v1, v2, f1, d1);
        
        /* Call that might be at BB_END */
        double tan_val = tan(d3);
        f3 = f3 * (float)tan_val;
        
        /* Jump back */
        if (iteration < 3) {
            continue;
        }
    }
    
final_computation:
    /* Final computations using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* One more external function call */
    printf("Final checksum: %.6f\n", checksum);
    
    /* Recursive call at the end */
    double final_recursive = recursive_helper(1, d1, d2, f1, f2, v1, v2, 
                                             d3, f3, v3, d4, f4, v4, d5, f5);
    
    printf("Final recursive result: %.6f\n", final_recursive);
    
    return (int)(checksum + final_recursive) % 100;
}
