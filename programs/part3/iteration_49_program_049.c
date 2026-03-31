#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to create register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, float f,
                        int g, int h, int i, double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix up the arguments to prevent optimization */
    double temp1 = a * b - c;
    float temp2 = d / e + f;
    int temp3 = g ^ h | i;
    
    /* Function call within recursion to create more caller-save opportunities */
    double sin_val = sin(j + k + l);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "movq %0, %%rax\n\t"
        "addq %1, %%rax\n\t"
        "movq %%rax, %0"
        : "+r" (temp1)
        : "r" (sin_val)
        : "%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"
    );
    
    /* Recursive call with shuffled arguments */
    return recursive_helper(depth - 1,
                           b, c, a,          /* Rotate doubles */
                           e, f, d,          /* Rotate floats */
                           h, i, g,          /* Rotate ints */
                           m, n, temp3, j, k, l) + temp1 + temp2;
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4, d5 = 5.5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Loop to create extended live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables to keep them live */
        d1 = d1 * 1.1 + outer;
        d2 = d2 / 1.2 - outer;
        d3 = d3 + sin(d4) * outer;
        d4 = pow(d4, 1.0 + outer * 0.1);
        d5 = d5 - cos(d1) + outer;
        
        f1 = f1 * 1.1f + outer;
        f2 = f2 / 1.2f - outer;
        f3 = f3 + sqrtf(f4) * outer;
        f4 = f4 * f4 - outer;
        f5 = f5 - f1 + outer;
        
        i1 = i1 + outer * 11;
        i2 = i2 - outer * 13;
        i3 = i3 * (outer + 1);
        i4 = i4 / (outer + 1);
        i5 = i5 ^ outer;
        i6 = i6 | (outer << 3);
        i7 = i7 & ~outer;
        i8 = i8 % (outer + 2);
        
        /* Function call with many arguments - will need caller-save */
        printf("Iteration %d: d1=%.2f, f1=%.2f, i1=%d\n", 
               outer, d1, f1, i1);
        
        /* Inline assembly that clobbers multiple caller-saved registers */
        asm volatile (
            "mov %0, %%eax\n\t"
            "add %1, %%eax\n\t"
            "imul %2, %%eax\n\t"
            "mov %%eax, %0"
            : "+r" (i1)
            : "r" (i2), "r" (i3)
            : "%eax", "%ecx", "%edx", "%esi", "%edi", "%r8", "%r9", "%r10", "%r11", "cc"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Multiple function calls */
            double pow_result = pow(d1, d2);
            checksum += pow_result;
            
            /* Function call at the end of basic block (before goto) */
            printf("Even iteration: pow=%.4f\n", pow_result);
            goto special_path;  /* Creates unusual BB structure */
        } else {
            /* Path 2: Different function calls */
            float sin_result = sinf(f1);
            checksum += sin_result;
            
            /* Another function call */
            printf("Odd iteration: sinf=%.4f\n", sin_result);
            
            /* Continue normally */
            special_return:
            i8 = rand() % 100;  /* External function call */
        }
        
        /* Call recursive function with many arguments */
        double rec_result = recursive_helper(2, d1, d2, d3, f1, f2, f3,
                                            i1, i2, i3, d4, f4, i4, d5, f5);
        checksum += rec_result;
        
        /* More arithmetic after calls to extend live ranges */
        d1 = d1 + rec_result * 0.01;
        f1 = f1 + rec_result * 0.02f;
        i1 = i1 + (int)rec_result;
        
        /* Another external function call */
        double log_result = log(fabs(d1) + 1.0);
        checksum += log_result;
        
        continue;  /* Skip the special path on normal execution */
        
        special_path:
        /* This block is only reached via goto */
        /* Function call at the end of this basic block */
        printf("Special path taken! i1=%d\n", i1);
        
        /* Inline assembly with different clobbers */
        asm volatile (
            "movsd %0, %%xmm0\n\t"
            "addsd %1, %%xmm0\n\t"
            "movsd %%xmm0, %0"
            : "+r" (d1)
            : "r" (d2)
            : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7",
              "%xmm8", "%xmm9", "%xmm10", "%xmm11", "%xmm12", "%xmm13", "%xmm14", "%xmm15"
        );
        
        goto special_return;
    }
    
    /* Final computation using all variables */
    double final_result = d1 + d2 + d3 + d4 + d5 +
                         f1 + f2 + f3 + f4 + f5 +
                         i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 +
                         checksum;
    
    /* Final printf to prevent dead code elimination */
    printf("Final result: %.10f\n", final_result);
    
    return (int)final_result % 100;
}
