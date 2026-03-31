#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function with many arguments */
double recursive_helper(int depth, double a, double b, float c, float d, 
                       int e, int f, double g, float h, int i, 
                       double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * j + k * l;
    
    /* Function call to create register pressure */
    double sin_val = sin(t1 + t2 + t3);
    
    /* Inline assembly clobbering caller-saved registers */
    asm volatile (
        "movq %0, %%rax\n\t"
        "addq %1, %%rax\n\t"
        : 
        : "r"((long)t1), "r"((long)t2)
        : "%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"
    );
    
    return sin_val + recursive_helper(depth - 1, 
                                     a + 1.1, b * 0.95, c * 1.05f, d + 0.5f,
                                     e ^ 0x55, f + 3, g * 0.99, h * 1.01f,
                                     i >> 1, j * 1.001, k + 0.25f, l << 1,
                                     m / 1.1, n * 0.9f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Complex loop creating register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Arithmetic on all variables before call */
        v1 = v2 * v3 + v4;
        v2 = v5 ^ v6 | v1;
        v3 = v4 + v5 * v6;
        v4 = v1 & v2 ^ v3;
        v5 = v6 + v1 - v2;
        v6 = v3 * v4 / (v5 ? v5 : 1);
        
        f1 = f2 * f3 + f4;
        f2 = f5 / f1 * 2.0f;
        f3 = f4 + f5 - f1;
        f4 = f2 * f3 / f5;
        f5 = f1 + f2 + f3 + f4;
        
        d1 = d2 * d3 + sin(d4);
        d2 = d5 / d1 * 2.0;
        d3 = d4 + d5 - d1;
        d4 = d2 * d3 / d5;
        d5 = d1 + d2 + d3 + d4 + d5;
        
        /* Function call with many live variables */
        printf("Iteration %d: v1=%d, f1=%.2f, d1=%.2f\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly clobbering multiple caller-saved registers */
        asm volatile (
            "addl %1, %0\n\t"
            "imull %2, %0\n\t"
            : "+r"(v1)
            : "r"(v2), "r"(v3)
            : "%eax", "%ecx", "%edx", "%esi", "%edi", "%r8d", "%r9d", "%r10d", "%r11d",
              "cc", "memory"
        );
        
        /* Conditional branch with calls at block ends */
        if (rand() % 2) {
            /* Path 1: Call at end of basic block */
            double pow_result = pow(d1, d2);
            f1 += (float)pow_result;
            d3 += sin(pow_result);
            
            /* Another call right before goto */
            printf("Path A: pow=%.3f\n", pow_result);
            goto merge_point;  /* Creates basic block ending with call */
        } else {
            /* Path 2: Different call pattern */
            float sinf_result = sinf(f2 * f3);
            d4 += cos(sinf_result);
            v2 += (int)(sinf_result * 100);
            
            /* Call at end of this basic block too */
            printf("Path B: sinf=%.3f\n", sinf_result);
            /* Fall through to merge */
        }
        
        /* Merge point after conditional */
        merge_point:
        
        /* More arithmetic to extend live ranges */
        v3 = v1 * v2 - v4;
        f3 = f1 + f2 * f4;
        d5 = d3 * d4 / d2;
        
        /* Recursive call with many arguments */
        double rec_result = recursive_helper(2, d1, d2, f1, f2, v1, v2,
                                           d3, f3, v3, d4, f4, v4, d5, f5);
        
        /* Use result to prevent elimination */
        v5 += (int)rec_result;
        f5 += (float)fmod(rec_result, 10.0);
        d1 += rec_result * 0.1;
        
        /* Another external function call */
        double cos_result = cos(d1 + d2 + d3);
        printf("Cos result: %.3f\n", cos_result);
        
        /* Second inline assembly with different clobbers */
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "mulsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m"(d2)
            : "m"(d1), "m"(d3)
            : "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5",
              "%xmm6", "%xmm7", "%xmm8", "%xmm9", "%xmm10", "%xmm11",
              "%xmm12", "%xmm13", "%xmm14", "%xmm15", "memory"
        );
        
        /* Nested loop for additional pressure */
        for (int inner = 0; inner < 2; inner++) {
            v6 = v5 * v4 + inner;
            f4 = f3 * f2 + inner;
            d4 = d3 * d2 + inner;
            
            /* Call inside nested loop */
            double log_result = log(fabs(d4) + 1.0);
            v6 += (int)log_result;
            
            /* More arithmetic */
            v1 = v2 + v3 * v4 - v5 + v6;
            f1 = f2 * f3 / f4 + f5;
            d1 = d2 + d3 * d4 - d5;
        }
    }
    
    /* Unusual control flow with goto creating special block boundaries */
    if (v1 > 1000) {
        printf("Large v1: %d\n", v1);
        goto final_computation;
    } else {
        printf("Normal v1: %d\n", v1);
        /* Call at end of basic block before goto */
        double sqrt_result = sqrt(fabs(d1));
        v2 += (int)sqrt_result;
        goto final_computation;
    }
    
    /* Dead code to create additional block boundary */
    v3 = 999;
    printf("Unreachable: %d\n", v3);
    
final_computation:
    /* Final checksum using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 + v6 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* Multiple calls in final computation */
    checksum += sin(checksum);
    checksum += cos(checksum * 0.5);
    
    printf("Final checksum: %.6f\n", checksum);
    
    /* One more call right before return */
    printf("Program completed.\n");
    
    return (int)(checksum * 1000) % 256;
}
