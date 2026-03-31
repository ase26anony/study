/* caller-save-test.c
 * Designed to trigger uncovered lines in caller-save.cc (lines 905-913)
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -lm
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
    
    /* Mix arithmetic and function calls to create complex live ranges */
    double temp1 = a * b + sin(c) * cos(d);
    float temp2 = c * d + log(fabs(e) + 1.0f);
    int temp3 = e * f + (int)(pow(g, 2.0));
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "addl %1, %0\n\t"
        "movl %0, %%eax\n\t"
        "movl %2, %%ecx\n\t"
        "movl %3, %%edx"
        : "+r"(temp3)
        : "r"(i), "r"(l), "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Call external functions with live variables */
    printf("Depth %d: temp1=%.3f, temp2=%.3f\n", depth, temp1, temp2);
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1,
                           temp1, j, k, n,
                           temp3, l, m, h, i,
                           a + g, b + c, d + e, f + j, k + l);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Outer loop to create extended live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables to keep them live */
        v1 = v2 * v3 + v4 - v5;
        v2 = v3 ^ v4 | v5;
        v3 = v4 % (v5 + 1) + outer;
        
        f1 = f2 * f3 + sinf(f4) * cosf(f5);
        f2 = f3 / (f4 + 0.1f) + tanf(f5);
        f3 = f4 + f5 * outer;
        
        d1 = d2 * d3 + sin(d4) * cos(d5);
        d2 = d3 / (d4 + 0.1) + tan(d5);
        d3 = d4 + d5 * outer;
        
        /* Conditional branch with function call at end of basic block */
        if (outer % 2 == 0) {
            /* Path 1: Multiple operations then function call at block end */
            v4 = v5 * 2 + rand() % 100;
            f4 = f5 * 3.14f + (float)rand() / RAND_MAX;
            d4 = d5 * 3.14159 + (double)rand() / RAND_MAX;
            
            /* Inline assembly clobbering caller-saved registers */
            __asm__ volatile (
                "movl %0, %%eax\n\t"
                "addl %1, %%eax\n\t"
                "movl %%eax, %0\n\t"
                "movl %2, %%ecx\n\t"
                "movl %3, %%edx"
                : "+r"(v4)
                : "r"(v1), "r"(v2), "r"(v3)
                : "%eax", "%ecx", "%edx", "cc"
            );
            
            /* Function call at the end of basic block (potential BB_END) */
            checksum += pow(d1, d2) + pow(d3, d4);
            goto call_block;  /* Unusual control flow */
        } else {
            /* Path 2: Different pattern with call in middle */
            v5 = v1 * v2 - v3;
            f5 = f1 * f2 - f3;
            d5 = d1 * d2 - d3;
            
            /* Call external function */
            printf("Iteration %d: v5=%d, f5=%.3f\n", outer, v5, f5);
            
            /* More arithmetic after call */
            v1 += v5;
            f1 += f5;
            d1 += d5;
            
            /* Another function call */
            checksum += sin(d2) + cos(d3);
            continue;  /* Skip the goto block */
        }
        
call_block:
        /* Block reached via goto with different call pattern */
        {
            /* Inline assembly with different clobbered registers */
            __asm__ volatile (
                "movq %0, %%xmm0\n\t"
                "addq %1, %%xmm0\n\t"
                "movq %%xmm0, %0"
                : "+r"(*(long long*)&d1)
                : "r"(*(long long*)&d2)
                : "%xmm0", "%xmm1", "cc"
            );
            
            /* Call recursive function with many live arguments */
            double rec_result = recursive_helper(
                2,  /* depth */
                d1, d2, f1, f2,
                v1, v2, d3, f3, v3,
                d4, f4, v4, d5, f5
            );
            
            checksum += rec_result;
            
            /* Function call just before label (potential BB_END) */
            printf("Goto block: rec_result=%.3f\n", rec_result);
        }
        
        /* Label creating basic block boundary */
        after_goto:
        /* More arithmetic and calls */
        v1 = v2 + v3 * v4;
        f1 = f2 + f3 * f4;
        d1 = d2 + d3 * d4;
        
        /* Final call in loop iteration */
        checksum += log(fabs(d1) + 1.0) + sqrt(fabs(f1) + 1.0f);
    }
    
    /* Additional complex block with calls at the end */
    {
        int temp = v1 + v2 + v3 + v4 + v5;
        float ftemp = f1 + f2 + f3 + f4 + f5;
        double dtemp = d1 + d2 + d3 + d4 + d5;
        
        /* Multiple operations then call at block end */
        for (int i = 0; i < 2; i++) {
            temp *= 2;
            ftemp *= 1.5f;
            dtemp *= 1.5;
            
            /* Inline assembly between operations */
            __asm__ volatile (
                "imull %1, %0\n\t"
                "movl %0, %%eax\n\t"
                "movl %2, %%ecx"
                : "+r"(temp)
                : "r"(i + 1), "r"(v1)
                : "%eax", "%ecx", "%edx", "cc"
            );
            
            /* Call at potential basic block end */
            if (i == 0) {
                checksum += pow(dtemp, 2.0) + pow(ftemp, 2.0f);
            }
        }
        
        /* Another call sequence */
        printf("Final temp: %d, ftemp=%.3f\n", temp, ftemp);
        checksum += sin(dtemp) * cos(checksum);
    }
    
    /* Print final checksum to prevent dead code elimination */
    printf("Final checksum: %.15f\n", checksum);
    
    /* Conditional return to create another basic block boundary */
    if (checksum > 100.0) {
        return 0;
    } else {
        /* Function call just before return */
        printf("Low checksum: %.3f\n", checksum);
        return 1;
    }
}
