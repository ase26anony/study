#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, double c, 
                        float d, float e, float f,
                        int g, int h, int i,
                        double j, float k, int l, int m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix arithmetic with function calls */
    double temp1 = a * b - c;
    float temp2 = d / e + f;
    int temp3 = g ^ h | i;
    
    /* Call external function */
    temp1 = sin(temp1) * cos(temp2);
    
    /* Inline assembly clobbering caller-saved registers */
    asm volatile (
        "movq %0, %%rax\n\t"
        "addq %1, %%rax\n\t"
        "movq %%rax, %0"
        : "+r" (temp3)
        : "r" (depth)
        : "%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"
    );
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           temp1, b + 1.0, c - 0.5,
                           d * 1.1f, e / 1.2f, f + 0.3f,
                           g + 2, h - 1, i * 3,
                           j + temp1, k + temp2, l + temp3, m + depth);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    
    srand(time(NULL));
    
    /* Start of complex control flow */
    int iteration = 0;
    
loop_start:
    if (iteration >= 3) goto final_computation;
    
    /* Arithmetic operations before function call */
    d1 = d1 * 1.1 + d2;
    d2 = d2 / 1.2 - d3;
    d3 = d3 + sin(d4);
    f1 = f1 * 2.0f + f2;
    f2 = f2 / 1.5f - f3;
    i1 = i1 + i2 * 2;
    i2 = i2 ^ i3;
    
    /* Function call with many live variables */
    printf("Iteration %d: d1=%.3f, f1=%.3f, i1=%d\n", 
           iteration, d1, f1, i1);
    
    /* More arithmetic */
    d4 = pow(d1, 2.0) + d3;
    f3 = sqrtf(f2) * f1;
    i3 = i3 | i4 & i5;
    
    /* Inline assembly clobbering caller-saved registers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "imul %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0"
        : "+r" (i4)
        : "r" (i5), "r" (i6)
        : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    
    /* Conditional branch creating basic block boundary */
    if (d1 > 5.0) {
        /* Path with function call at end of basic block */
        f4 = (float)sin(d2) * cos(d3);
        f5 = tanf(f4) + f3;
        i5 = rand() % 100 + i4;
        
        /* Function call right before goto (potential BB_END) */
        printf("d1 > 5: f4=%.3f, i5=%d\n", f4, i5);
        goto after_condition;  /* This creates a basic block ending with call */
    } else {
        /* Alternative path */
        d3 = log(d4) * exp(d1);
        i6 = abs(i5 - i4) * 2;
        
        /* Another function call */
        double result = pow(d2, d3);
        printf("d1 <= 5: pow result=%.3f\n", result);
    }
    
after_condition:
    /* More arithmetic mixing variables */
    d1 = d1 + d4 * 0.3;
    d2 = d2 - d3 / 1.7;
    f1 = f1 + f5 * 0.5f;
    f2 = f2 - f4 / 2.0f;
    i1 = i1 + i6;
    i2 = i2 * i5;
    
    /* Call recursive function with many arguments */
    double rec_result = recursive_helper(
        2,  /* depth */
        d1, d2, d3, 
        f1, f2, f3,
        i1, i2, i3,
        d4, f4, i4, i5
    );
    
    printf("Recursive result: %.3f\n", rec_result);
    
    /* Another inline assembly block */
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        "mov %3, %%eax\n\t"
        "add %4, %%eax\n\t"
        "mov %%eax, %5"
        : "=r" (d1), "+r" (i6)
        : "r" (d2), "r" (i1), "r" (i2), "r" (i6)
        : "%eax", "%ecx", "%edx", "%xmm0", "%xmm1", "%xmm2"
    );
    
    /* Call external math function */
    d4 = sin(d1) + cos(d2) * tan(d3);
    
    /* Update and loop */
    iteration++;
    goto loop_start;
    
final_computation:
    /* Final checksum computation using all variables */
    double checksum = d1 + d2 + d3 + d4 + 
                     f1 + f2 + f3 + f4 + f5 + 
                     i1 + i2 + i3 + i4 + i5 + i6;
    
    /* One more function call */
    checksum = fabs(checksum) + log(checksum > 0 ? checksum : 1.0);
    
    printf("Final checksum: %.6f\n", checksum);
    
    /* Additional control flow with goto */
    if (checksum > 100.0) {
        goto large_result;
    } else {
        goto small_result;
    }
    
large_result:
    printf("Large result - calling pow()\n");
    checksum = pow(checksum, 1.5);  /* Function call at BB end */
    goto print_final;
    
small_result:
    printf("Small result - calling sqrt()\n");
    checksum = sqrt(checksum);  /* Another function call at BB end */
    /* Fall through */
    
print_final:
    printf("Adjusted checksum: %.6f\n", checksum);
    
    return (int)(checksum * 1000) % 1000;
}
