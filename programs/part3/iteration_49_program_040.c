#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, float f,
                       int g, int h, int i, double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix arithmetic operations with function calls */
    double temp1 = sin(a) * cos(b);
    float temp2 = powf(d, e);
    int temp3 = g * h + i;
    
    /* Inline assembly clobbering caller-saved registers */
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "+r"(temp3)
        : "r"(l)
        : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    
    /* Call external functions */
    printf("Depth %d: temp1=%.3f, temp2=%.3f, temp3=%d\n", 
           depth, temp1, temp2, temp3);
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1,
                           a * 0.9, b * 1.1, c + temp1,
                           d * 0.8f, e * 1.2f, f + temp2,
                           g + 1, h - 1, i ^ temp3,
                           j * 0.95, k * 1.05f,
                           l << 1, m / 1.5, n * 1.1f);
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
        /* Block with arithmetic before calls */
        v1 = v2 * v3 + v4;
        v2 = v5 ^ v6;
        f1 = f2 * f3 - f4;
        d1 = d2 / d3 + d4;
        
        /* Inline assembly clobbering caller-saved registers */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(v3)
            : "r"(v1), "r"(v2)
            : "%eax", "%ecx", "%edx", "%esi", "%edi"
        );
        
        /* Function call with many arguments - could be at block end */
        if (outer % 2 == 0) {
            checksum += sin(d1) * cos(d2);
            printf("Even iteration: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
            /* This call might be at the end of a basic block */
            goto process_odd;  /* Unusual control flow */
        } else {
            process_odd:
            /* More arithmetic */
            v4 = v5 + v6 * v1;
            f2 = f3 * f4 / f5;
            d2 = d3 * d4 - d5;
            
            /* Another function call */
            double power_result = pow(d1, d2);
            checksum += power_result;
            
            /* Call recursive function - creates more register pressure */
            double rec_result = recursive_helper(2, d1, d2, d3, f1, f2, f3,
                                                v1, v2, v3, d4, f4, v4, d5, f5);
            checksum += rec_result;
            
            /* Inline assembly with different clobbers */
            __asm__ volatile (
                "imull %1, %0\n\t"
                : "+r"(v5)
                : "r"(v6)
                : "%eax", "%edx", "%ecx"
            );
        }
        
        /* Nested loop for more pressure */
        for (int inner = 0; inner < 2; inner++) {
            /* Arithmetic operations mixing all variable types */
            v6 = (v1 + v2) * (v3 - v4) / (v5 + 1);
            f3 = f1 * f2 + f4 * f5;
            d3 = d1 * d2 - d4 * d5;
            
            /* Function call in the middle of computations */
            float rand_val = (float)rand() / RAND_MAX;
            checksum += rand_val;
            
            /* More arithmetic */
            f4 = sinf(f3) * cosf(f2);
            d4 = sqrt(d3) * log(d2 + 1.0);
            
            /* Another external function call */
            if (inner == 0) {
                printf("Inner loop: v6=%d, f3=%.3f, d3=%.3f\n", v6, f3, d3);
                /* This could place a call at block end before the loop increment */
            }
            
            /* Use goto to create unusual block boundaries */
            if (v6 > 100) {
                goto skip_computation;
            }
            
            /* More computations */
            v1 = v2 + v3 * v4;
            f5 = f1 + f2 * f3;
            d5 = d1 + d2 * d3;
            
            skip_computation:
            /* Function call potentially at block end before loop ends */
            checksum += fabs(d4) + fabs(d5);
        }
        
        /* Conditional with function call at the end of one path */
        if (checksum > 100.0) {
            v1 = rand() % 100;
            printf("Large checksum: %.2f, random v1=%d\n", checksum, v1);
            /* Function call at block end */
        } else {
            v1 = v2 + v3;
            f1 = f2 * f3;
            d1 = d2 / d3;
            /* No call at end - different block structure */
        }
        
        /* Another arithmetic section */
        v2 = v3 * v4 - v5;
        f2 = f3 / f4 + f5;
        d2 = d3 * d4 - d5;
        
        /* Final function call in the outer loop */
        checksum += tan(d1) * atan(d2);
    }
    
    /* Compute final checksum using all variables */
    double final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5 +
        checksum;
    
    printf("Final result: %.15f\n", final_result);
    
    return (int)(final_result * 1000) % 1000;
}
