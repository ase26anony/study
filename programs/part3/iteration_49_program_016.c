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
    
    /* Mix up the arguments to prevent optimization */
    double temp1 = a * b - c / d + e % (f + 1);
    float temp2 = c * d - h * k + n;
    int temp3 = e ^ f ^ i ^ l;
    double temp4 = g + j + m + sin(a) + cos(b);
    
    /* Function call within recursion to create more caller-save opportunities */
    if (depth % 2 == 0) {
        printf("Recursion depth %d: %f\n", depth, temp4);
    }
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "mov %0, %%eax\n"
        "add %1, %%eax\n"
        "mov %%eax, %0\n"
        : "+r"(temp3)
        : "r"(depth)
        : "%eax", "%ecx", "%edx"
    );
    
    return recursive_helper(depth - 1, 
                           temp1, b + 1.0, c * 1.1f, d * 0.9f,
                           temp3, f + 2, g * 1.05, h * 0.95f, i * 3,
                           j + sin(temp1), k * 1.2f, l + 5, m * 0.8, n * 1.1f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        v1 = v2 * v3 + v4 - v5;
        f1 = f2 * f3 + f4 - f5;
        d1 = d2 * d3 + d4 - d5;
        
        /* First function call with mixed arguments */
        printf("Outer loop %d: v1=%d, f1=%f, d1=%lf\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly clobbering caller-saved registers */
        __asm__ volatile (
            "mov %0, %%eax\n"
            "imul %1, %%eax\n"
            "add %2, %%eax\n"
            "mov %%eax, %0\n"
            : "+r"(v1)
            : "r"(v2), "r"(v3)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (v1 % 2 == 0) {
            /* Path 1: Call at the end of basic block */
            d2 = sin(d1) + cos(d3);
            f2 = powf(f1, 2.0f);
            v2 = rand() % 100;
            
            /* Function call right before goto (potential BB_END) */
            printf("Even path: sin(%lf)=%lf\n", d1, d2);
            goto process_data;
        } else {
            /* Path 2: Multiple calls in sequence */
            d3 = pow(d1, 2.0) + sqrt(d2);
            f3 = sinf(f1) * cosf(f2);
            v3 = abs(v1 - v2) * v4;
            
            /* External function call */
            double trig_result = sin(d1) * cos(d2) + tan(d3);
            printf("Odd path: trig=%lf\n", trig_result);
            
            /* Another call right before block end */
            v4 = (int)(trig_result * 100) % 50;
        }
        
        /* Label to create unusual basic block structure */
        process_data:
        
        /* Arithmetic to keep variables live */
        v5 = v1 + v2 + v3 + v4;
        f4 = f1 + f2 + f3;
        d4 = d1 + d2 + d3;
        
        /* Nested loop for more pressure */
        for (int inner = 0; inner < 2; inner++) {
            /* More arithmetic */
            v5 = (v5 * 3) / 2;
            f4 = f4 * 1.5f - 0.5f;
            d4 = d4 * 1.7 - 0.7;
            
            /* Function call within nested loop */
            if (inner == 0) {
                double power_result = pow(d4, 1.5);
                printf("Inner loop %d: pow=%lf\n", inner, power_result);
            }
            
            /* Inline assembly with different clobbers */
            __asm__ volatile (
                "mov %0, %%eax\n"
                "add %%eax, %%eax\n"
                "sub %1, %%eax\n"
                "mov %%eax, %0\n"
                : "+r"(v5)
                : "r"(inner)
                : "%eax", "%edx", "cc"
            );
            
            /* Keep all variables live across this call */
            f5 = sinf(f4) + cosf(f3);
        }
        
        /* Recursive call with many live variables */
        double recursive_result = recursive_helper(
            2,  /* depth */
            d1, d2, f1, f2, v1, v2, d3, f3, v3, d4, f4, v4, d5, f5
        );
        
        /* Arithmetic after recursive call */
        v1 = (int)(recursive_result * 100) % 1000;
        f1 = (float)fmod(recursive_result, 10.0);
        d1 = recursive_result * 0.9;
        
        /* Another function call */
        printf("After recursion: v1=%d, recursive=%lf\n", v1, recursive_result);
        
        /* More arithmetic to extend live ranges */
        v2 = v1 * v3 - v4;
        f2 = f1 * f3 - f4;
        d2 = d1 * d3 - d4;
        
        /* Final call in loop iteration */
        if (outer < 2) {
            double final_combine = d1 + d2 + d3 + d4 + d5;
            printf("Loop combine: %lf\n", final_combine);
            
            /* Goto to create another basic block boundary */
            if (final_combine > 100.0) {
                goto adjust_values;
            }
        }
        
        continue;
        
        /* Target of goto - creates unusual flow */
        adjust_values:
        v1 = v1 / 2;
        f1 = f1 / 2.0f;
        d1 = d1 / 2.0;
        printf("Adjusted values\n");
    }
    
    /* Compute final checksum from all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* One last function call */
    printf("Final checksum: %lf\n", checksum);
    
    /* Inline assembly at the very end */
    __asm__ volatile (
        "mov %0, %%eax\n"
        "add $1, %%eax\n"
        "mov %%eax, %0\n"
        : "+r"(v1)
        :
        : "%eax", "cc"
    );
    
    return (int)checksum % 255;
}
