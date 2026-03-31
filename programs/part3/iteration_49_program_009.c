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
    
    /* Mix operations and calls */
    double temp1 = sin(a) * cos(b);
    float temp2 = c * d - h * k;
    int temp3 = e ^ f | i & l;
    
    /* Inline assembly clobbering caller-saved registers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0"
        : "+r"(temp3)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    
    /* Recursive call with shuffled arguments */
    return temp1 + temp2 + temp3 + 
           recursive_helper(depth - 1, b, a, d, c, f, e, 
                           j, k, l, g, n, i, m, h);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Start with a goto to create unusual BB structure */
    goto start_block;
    
loop_body:
    /* Block with function call at the end */
    d1 = sin(d1) + cos(d2);
    f1 = f1 * 1.1f - f2 / 2.0f;
    v1 = (v1 * 3) % 7;
    
    /* Function call at end of basic block */
    printf("Intermediate: d1=%f, f1=%f, v1=%d\n", d1, f1, v1);
    
    /* Jump to another block */
    goto mid_block;
    
start_block:
    for (int iter = 0; iter < 4; iter++) {
        /* Complex arithmetic creating register pressure */
        d1 = d1 * d2 + d3 / d4 - d5;
        d2 = d2 * d3 - d4 / d5 + d1;
        d3 = d3 * d4 + d5 / d1 - d2;
        d4 = d4 * d5 - d1 / d2 + d3;
        d5 = d5 * d1 + d2 / d3 - d4;
        
        f1 = f1 + f2 * f3 - f4 / f5;
        f2 = f2 - f3 * f4 + f5 / f1;
        f3 = f3 + f4 * f5 - f1 / f2;
        f4 = f4 - f5 * f1 + f2 / f3;
        f5 = f5 + f1 * f2 - f3 / f4;
        
        v1 = (v1 + v2) * (v3 - v4) / (v5 + 1);
        v2 = (v2 - v3) * (v4 + v5) % (v1 + 2);
        v3 = (v3 * v4) + (v5 - v1) * (v2 + 3);
        v4 = (v4 / (v5 + 1)) * (v1 - v2) + v3;
        v5 = (v5 % (v1 + 1)) * (v2 - v3) + v4;
        
        /* Inline assembly clobbering multiple caller-saved registers */
        asm volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0"
            : "+r"(v1)
            : "r"(v2), "r"(iter)
            : "%eax", "%ecx", "%edx", "%esi", "%edi", "cc"
        );
        
        /* Another inline assembly block */
        asm volatile (
            "addsd %1, %0\n\t"
            "mulsd %2, %0"
            : "+x"(d1)
            : "x"(d2), "x"(d3)
            : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5"
        );
        
        /* Conditional branch with calls in both paths */
        if (iter % 2 == 0) {
            /* Path 1: Multiple function calls */
            double pow_result = pow(d1, d2);
            printf("pow(%f, %f) = %f\n", d1, d2, pow_result);
            
            float sinf_result = sinf(f1);
            f3 = cosf(f2) * sinf_result;
            
            /* Call at end of basic block before goto */
            checksum += pow_result + sinf_result;
            goto loop_body;
        } else {
mid_block:
            /* Path 2: Different function calls */
            int rand_val = rand() % 100;
            v5 = (v5 + rand_val) % 97;
            
            double log_result = log(fabs(d3) + 1.0);
            printf("log|d3|=%f, rand=%d\n", log_result, rand_val);
            
            /* Call external math function */
            d4 = tan(d4) * atan(d5);
            
            /* Keep variables live */
            f4 = f4 + f5 * 0.5f;
        }
        
        /* Recursive call with many live variables */
        double rec_result = recursive_helper(
            2, d1, d2, f1, f2, v1, v2, d3, f3, v3, 
            d4, f4, v4, d5, f5
        );
        
        checksum += rec_result;
        
        /* More arithmetic after calls */
        v1 = v1 + (int)(rec_result * 100) % 13;
        d1 = d1 * 1.01 + sin(rec_result);
        f1 = f1 * 1.1f + cosf((float)rec_result);
        
        /* Another function call */
        printf("Iteration %d: checksum so far = %f\n", iter, checksum);
    }
    
    /* Final computation using all variables */
    double final_result = 
        d1 + d2 + d3 + d4 + d5 +
        f1 + f2 + f3 + f4 + f5 +
        v1 + v2 + v3 + v4 + v5 +
        checksum;
    
    /* Prevent dead code elimination */
    printf("Final result: %f\n", final_result);
    
    /* Additional calls at end of basic blocks */
    if (final_result > 1000.0) {
        printf("Large result detected: %f\n", final_result);
        return 1;  /* Basic block ends with return */
    } else {
        printf("Normal result: %f\n", final_result);
        /* Another call at block end */
        fflush(stdout);
    }
    
    return 0;
}
