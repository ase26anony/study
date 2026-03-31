#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                        int e, int f, double g, float h, int i, 
                        double j, float k, int l, int m, double n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix arithmetic operations with function calls */
    double temp1 = sin(a) * cos(b);
    float temp2 = c * d + h * k;
    int temp3 = e * f + i * l * m;
    
    /* Call external functions with live variables */
    printf("Depth %d: %f %f\n", depth, temp1, (double)temp2);
    
    /* More arithmetic to extend live ranges */
    temp1 += pow(j, 2.0) + sqrt(n);
    temp2 += fabs(c - d) * tan(h);
    temp3 += rand() % 100;
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           temp1, b * 0.9, c * 1.1, d * 0.8,
                           e + 1, f - 1, g * 1.05, h * 0.95, i * 2,
                           j * 0.99, k * 1.01, l + 2, m - 2, n * 1.1);
}

int main() {
    /* Declare and initialize many local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    /* Variable to control goto flow */
    int use_alternative_path = 0;
    
    /* Main loop creating register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        v1 = v2 * v3 + v4;
        v2 = v5 - v6 * outer;
        f1 = f2 * f3 + sinf(f4);
        f2 = cosf(f5) * outer;
        d1 = d2 * d3 + pow(d4, 1.5);
        d2 = sqrt(d5) * outer;
        
        /* Inline assembly clobbering caller-saved registers */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r"(v1) : "r"(v2) : "%eax", "%ecx", "%edx", "%esi", "%edi"
        );
        
        /* Conditional branch creating basic block boundary */
        if (outer % 2 == 0) {
            /* Path 1: Function call at end of basic block */
            d3 = sin(d1) * cos(d2);
            f3 = tanf(f1) * atan2(f2, f4);
            v3 = v1 * v2 + v4 * v5;
            
            /* Call external function with many arguments */
            printf("Iteration %d: %d %f %f\n", 
                   outer, v3, d3, (double)f3);
            
            /* This call is at the end of a basic block */
            d4 = pow(d1, d2) + exp(d3);
            
            /* Label for goto target */
            alternative_path:
            /* More arithmetic after call */
            v4 = v3 * 2 + v6;
            f4 = f3 * 3.14f + f5;
            
            /* Another function call */
            d5 = fmod(d4, 2.0) + log(d3);
            
        } else {
            /* Path 2: Different call pattern */
            use_alternative_path = 1;
            v5 = v6 * v1 - v2;
            f5 = f1 / f2 * f4;
            
            /* Call with different arguments */
            printf("Alt path %d: %d %f\n", 
                   outer, v5, (double)f5);
            
            /* Recursive call creating more pressure */
            double rec_result = recursive_helper(
                2, d1, d2, f1, f2, v1, v2, d3, f3, v3,
                d4, f4, v4, v5, d5
            );
            
            /* Arithmetic after recursive call */
            d1 += rec_result * 0.1;
            v1 += (int)rec_result % 100;
            
            /* Jump to create unusual block structure */
            if (use_alternative_path) {
                goto alternative_path;
            }
        }
        
        /* Nested inner loop extending live ranges */
        for (int inner = 0; inner < 2; inner++) {
            /* More arithmetic mixing all variables */
            v6 = v1 + v2 * inner - v3 / (v4 + 1);
            f1 = f2 + f3 * inner - f4 / (f5 + 1.0f);
            d2 = d3 + d4 * inner - d5 / (d1 + 1.0);
            
            /* Another inline assembly block */
            __asm__ volatile (
                "movq %0, %%xmm0\n\t"
                "addq %1, %%xmm0\n\t"
                "movq %%xmm0, %0\n\t"
                : "+r"(*(long long*)&d2) 
                : "r"(*(long long*)&d3) 
                : "%xmm0", "%xmm1", "%xmm2"
            );
            
            /* Function call inside nested loop */
            f2 = sinf(f1 * inner) + cosf(f3);
            d3 = atan2(d2, d4 * (inner + 1));
            
            /* Call external math function */
            v5 = abs(v6 * inner - v4);
        }
        
        /* Another conditional with call at block end */
        if (outer == 1) {
            v2 = v3 * v4 + v5 * v6;
            f3 = f4 * f5 + f1 * f2;
            d4 = d5 * d1 + d2 * d3;
            
            /* Call at the end before loop continues */
            printf("Midpoint: %d %f %f\n", v2, (double)f3, d4);
        }
    }
    
    /* Final computation using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 + v6 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* Prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    
    return (int)checksum % 100;
}
