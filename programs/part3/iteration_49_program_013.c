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
    
    /* Mix operations to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * l + (int)(j * k);
    double t4 = m * n + t1;
    
    /* Function call within recursion to create more caller-save opportunities */
    printf("Depth %d: t1=%.3f t2=%.3f t3=%d t4=%.3f\n", 
           depth, t1, t2, t3, t4);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "movq %0, %%rax\n\t"
        "movq %1, %%rcx\n\t"
        "addq %%rcx, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "+r"(t4)
        : "r"(t1)
        : "%rax", "%rcx", "%rdx"
    );
    
    /* Recursive call with shuffled arguments to prevent optimization */
    return recursive_helper(depth - 1, 
                           b, a, d, c, 
                           f, e, h, g, 
                           l, i, k, j, 
                           n, m, t2) + t4;
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double total = 0.0;
    
    /* First loop with function calls and arithmetic */
    for (int i = 0; i < 4; i++) {
        /* Arithmetic operations before function call */
        v1 = v2 * v3 + i;
        v4 = v5 ^ v6;
        f1 = f2 * f3 + i;
        f4 = f5 / (i + 1);
        d1 = d2 * d3 + sin(d4);
        d5 = pow(d1, 1.0 + i * 0.1);
        
        /* Function call with many arguments - creates caller-save pressure */
        printf("Loop %d: v1=%d v4=%d f1=%.3f f4=%.3f d1=%.3f d5=%.3f\n",
               i, v1, v4, f1, f4, d1, d5);
        
        /* Inline assembly clobbering multiple caller-saved registers */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ecx\n\t"
            "addl %%ecx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            "movq %2, %%xmm0\n\t"
            "addsd %3, %%xmm0\n\t"
            "movq %%xmm0, %2\n\t"
            : "+r"(v1), "+r"(v2), "+r"(d1)
            : "r"(d2)
            : "%eax", "%ecx", "%edx", "%xmm0", "%xmm1", "%xmm2"
        );
        
        /* Conditional branch creating basic block boundary */
        if (i % 2 == 0) {
            /* Path with function call at end of basic block */
            double result = sin(d1) + cos(d2) + tan(d3);
            total += result;
            
            /* Function call right before goto - may be at BB end */
            printf("Even iteration: result=%.3f total=%.3f\n", result, total);
            goto process_data;  /* Creates unusual BB structure */
        } else {
            /* Different path with different call pattern */
            float fresult = f1 * f2 + f3 * f4;
            total += fresult;
            
            /* Another function call */
            int rand_val = rand() % 100;
            printf("Odd iteration: fresult=%.3f rand=%d\n", fresult, rand_val);
            
            /* Continue normally */
            continue;
        }
        
process_data:
        /* Target of goto - creates interesting BB boundary */
        /* Recursive call with many live variables */
        double rec_result = recursive_helper(2, 
                                           d1, d2, f1, f2,
                                           v1, v2, d3, f3,
                                           v3, d4, f4, v4,
                                           d5, f5);
        total += rec_result;
        
        /* More arithmetic after call */
        v5 = v6 * v1 + v2;
        v6 = v3 ^ v4;
        f5 = f1 * f2 - f3;
        
        /* Another function call */
        double power_result = pow(d1, d2);
        printf("Processed: rec=%.3f power=%.3f\n", rec_result, power_result);
        
        /* Inline assembly with different clobbered registers */
        __asm__ volatile (
            "movq %0, %%r8\n\t"
            "movq %1, %%r9\n\t"
            "subq %%r9, %%r8\n\t"
            "movq %%r8, %0\n\t"
            : "+r"(d3)
            : "r"(d4)
            : "%r8", "%r9", "%r10", "%r11"
        );
    }
    
    /* Second loop with nested structure */
    for (int outer = 0; outer < 3; outer++) {
        for (int inner = 0; inner < 2; inner++) {
            /* Complex arithmetic creating long live ranges */
            d1 = d1 * d2 + d3 * d4 - d5;
            f1 = f1 + f2 * f3 - f4 / f5;
            v1 = (v1 * v2 + v3) ^ (v4 + v5 * v6);
            
            /* Function call in nested loop */
            if (inner == 0) {
                double trig_result = sin(d1) * cos(d2) + tan(d3);
                printf("Nested: outer=%d inner=%d trig=%.3f\n", 
                       outer, inner, trig_result);
                
                /* Call at potential BB end before continue */
                v2 = rand() % 50;
                continue;  /* Creates BB boundary after call */
            }
            
            /* More operations */
            d2 = sqrt(d1 * d1 + d3 * d3);
            f2 = fabsf(f1 - f3 * f4);
            
            /* Another function call */
            printf("Inner loop end: d2=%.3f f2=%.3f\n", d2, f2);
        }
        
        /* Function call at end of outer loop iteration */
        total += d1 + d2 + d3 + f1 + f2 + v1 + v2;
        printf("Outer loop %d: total=%.3f\n", outer, total);
    }
    
    /* Final computation using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 + v6 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5 + total;
    
    printf("Final checksum: %.15f\n", checksum);
    
    /* Prevent dead code elimination */
    if (checksum > 1000.0) {
        printf("Large checksum detected!\n");
    } else {
        printf("Normal checksum range\n");
    }
    
    return (int)(checksum) % 256;
}
