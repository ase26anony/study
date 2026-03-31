#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                       int f, int g, int h, double i, float j, int k, double l, float m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix arithmetic and function calls in recursion */
    double temp1 = sin(a) * cos(b);
    float temp2 = powf(d, e);
    int temp3 = f * g - h;
    
    /* Inline assembly clobbering caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r"(temp3)
        : "r"(k), "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Call external function within recursion */
    double rand_val = (double)rand() / RAND_MAX;
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                          b * temp1, 
                          c + rand_val, 
                          a - temp1,
                          d * 0.5f,
                          e + temp2,
                          f + temp3,
                          g ^ h,
                          h + depth,
                          i * sin(l),
                          j * cosf(m),
                          k * 2,
                          l * 1.1,
                          m * 0.9f);
}

int main() {
    srand(time(NULL));
    
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    double checksum = 0.0;
    int iteration_count = 0;
    
    /* Start of complex control flow with goto */
    start_loop:
    
    /* Loop with 4 iterations */
    for (int outer = 0; outer < 4; outer++) {
        iteration_count++;
        
        /* Nested loop creating register pressure */
        for (int inner = 0; inner < 3; inner++) {
            /* Arithmetic on all variables - making them all live */
            v1 = v2 * v3 + inner;
            v2 = v4 ^ v5;
            v3 = v1 - v2 * outer;
            v4 = v5 + v1 * inner;
            v5 = v3 | v4;
            
            f1 = f2 * 1.1f + sinf(f3);
            f2 = f4 / (f5 + 0.5f);
            f3 = f1 * f2 - cosf(f4);
            f4 = f5 * 2.0f + tanf(f1);
            f5 = f3 / f4 * 3.14f;
            
            d1 = d2 * 1.01 + sin(d3);
            d2 = d4 / (d5 + 0.01);
            d3 = d1 * d2 - cos(d4);
            d4 = d5 * 2.0 + tan(d1);
            d5 = d3 / d4 * 3.14159;
            
            /* Inline assembly clobbering multiple caller-saved registers */
            __asm__ volatile (
                "mov %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                "imul %3, %%ecx"
                : "=r"(v1)
                : "r"(v2), "r"(inner), "r"(outer)
                : "%eax", "%ecx", "%edx", "cc"
            );
            
            /* Function call with many live variables - placed before conditional */
            printf("Iteration %d.%d: v1=%d, f1=%.2f, d1=%.2f\n", 
                   outer, inner, v1, f1, d1);
            
            /* Conditional branch where both paths contain function calls */
            if ((v1 + v2 + v3) % 3 == 0) {
                /* Path 1: Multiple function calls with live variables */
                double power_result = pow(d1, d2);
                float trig_result = sinf(f1) * cosf(f2);
                
                /* Call at end of basic block (just before label) */
                checksum += sin(d3) + cos(d4);
                
                /* This call is at block end before the label */
                printf("Path A: power=%.3f, trig=%.3f\n", power_result, trig_result);
                goto update_vars;  /* Creates basic block boundary */
            } else {
                /* Path 2: Different function call pattern */
                double log_result = log(fabs(d3) + 1.0);
                int rand_val = rand() % 100;
                
                /* Another call at block end */
                checksum += tan(d5) * 0.5;
                
                printf("Path B: log=%.3f, rand=%d\n", log_result, rand_val);
                goto update_vars;  /* Another basic block boundary */
            }
            
            update_vars:
            /* More arithmetic to extend live ranges across calls */
            v1 = (v1 * 17) % 123;
            v2 = (v2 + v3) ^ v4;
            
            /* Call recursive function with all live variables */
            double rec_result = recursive_helper(
                2,  /* depth */
                d1, d2, d3, f1, f2, 
                v1, v2, v3, d4, f3, 
                v4, d5, f4
            );
            
            checksum += rec_result;
            
            /* Another external function call */
            double trig_sum = sin(d1) + cos(d2) + tan(d3 * 0.1);
            printf("Recursive result: %.3f, Trig sum: %.3f\n", rec_result, trig_sum);
            
            /* Inline assembly between function calls */
            __asm__ volatile (
                "addl $1, %0\n\t"
                "subl $1, %1"
                : "+r"(v5), "+r"(v4)
                :
                : "%eax", "cc"
            );
        }
        
        /* Function call at the end of outer loop iteration */
        checksum += pow(d4, 1.5) * 0.01;
    }
    
    /* Unusual control flow with goto creating special block structure */
    if (iteration_count < 8) {
        /* Reset some variables and jump back */
        v1 = 1;
        f1 = 1.1f;
        d1 = 1.11;
        printf("Jumping back to start_loop\n");
        goto start_loop;
    }
    
    /* Final computation using all variables */
    double final_result = 
        v1 + v2 + v3 + v4 + v5 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5 +
        checksum;
    
    /* Prevent dead code elimination */
    printf("Final checksum: %.15f\n", final_result);
    printf("All variables: v1=%d, v2=%d, v3=%d, v4=%d, v5=%d\n", v1, v2, v3, v4, v5);
    printf("f1=%.3f, f2=%.3f, f3=%.3f, f4=%.3f, f5=%.3f\n", f1, f2, f3, f4, f5);
    printf("d1=%.3f, d2=%.3f, d3=%.3f, d4=%.3f, d5=%.3f\n", d1, d2, d3, d4, d5);
    
    return (int)(final_result * 1000) % 1000;
}
