#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, double c, float d, float e, float f,
                        int g, int h, int i, double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix operations and calls */
    double t1 = sin(a) * cos(b);
    float t2 = powf(d, e);
    int t3 = g * h + i;
    
    /* Inline assembly clobbering caller-saved registers */
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "+r"(t3)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "%esi", "%edi"
    );
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1,
                           a * 0.9, b * 1.1, c + t1,
                           d * 0.8f, e * 1.2f, f + t2,
                           g + 1, h - 1, i ^ t3,
                           j * 0.95, k * 1.05f, l << 1,
                           m * 0.85, n * 1.15f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    
    srand(time(NULL));
    
    /* Loop creating register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        d1 = sin(d1) + cos(d2);
        d2 = d3 * d4 - tan(d1);
        f1 = f2 * f3 / (f4 + 1.0f);
        f2 = powf(f1, f3);
        i1 = i2 * i3 + i4;
        i2 = i5 ^ i6 | i1;
        
        /* Inline assembly clobbering multiple caller-saved registers */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(i3)
            : "r"(i1), "r"(i2)
            : "%eax", "%ecx", "%edx", "%esi", "%edi", "memory"
        );
        
        /* Function call with many live variables */
        printf("Iteration %d: d1=%.3f, f1=%.3f, i1=%d\n", 
               outer, d1, f1, i1);
        
        /* More arithmetic */
        d3 = d4 * 1.5 + d2;
        f3 = f4 * 2.0f - f5;
        i4 = i5 * 3 + i6;
        
        /* Call to math library function */
        double pow_result = pow(d1, d2);
        d4 += pow_result * 0.1;
        
        /* Conditional branch - both paths contain calls */
        if (rand() % 2) {
            /* Path 1: Call at end of basic block */
            f4 = sinf(f1) + cosf(f2);
            printf("Path A: f4=%.3f\n", f4);
            /* Function call right before label */
            goto process_block_a;
        } else {
            /* Path 2: Different call pattern */
            f5 = tanf(f3) * 2.0f;
            printf("Path B: f5=%.3f\n", f5);
            /* Another call before goto */
            goto process_block_b;
        }
        
    process_block_a:
        /* Block with call at end */
        i5 = i6 * i1 - i2;
        d1 = recursive_helper(2, d1, d2, d3, f1, f2, f3,
                             i1, i2, i3, d4, f4, i4, d1, f5);
        /* This call is at block end before goto */
        goto common_processing;
        
    process_block_b:
        /* Different arithmetic pattern */
        i6 = i1 + i2 * i3;
        f1 = f2 + f3 * f4;
        /* Call not at block end */
        double sin_result = sin(d2 * d3);
        d2 += sin_result;
        /* Call at block end */
        printf("Block B: i6=%d, d2=%.3f\n", i6, d2);
        
    common_processing:
        /* More operations and calls */
        d3 = log(fabs(d3) + 1.0);
        f2 = expf(fabs(f2) * 0.5f);
        
        /* Inline assembly with different clobbers */
        __asm__ volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=x"(d4)
            : "x"(d3), "x"(d4)
            : "%xmm0", "%xmm1", "%xmm2"
        );
        
        /* Another external call */
        int rand_val = rand();
        i1 = (i1 + rand_val) % 1000;
        
        /* Recursive call with many arguments */
        double rec_result = recursive_helper(1, d4, d1, d2, f3, f4, f5,
                                           i2, i3, i4, d3, f1, i5, d4, f2);
        d1 += rec_result * 0.01;
    }
    
    /* Final computation using all variables */
    double checksum = d1 + d2 + d3 + d4 +
                     f1 + f2 + f3 + f4 + f5 +
                     i1 + i2 + i3 + i4 + i5 + i6;
    
    /* One more call with complex arguments */
    checksum = pow(checksum, 0.5);
    
    printf("Final checksum: %.6f\n", checksum);
    
    /* Additional block with call at end before return */
    if (checksum > 100.0) {
        printf("Large checksum detected: %.2f\n", checksum);
        /* Call at block end */
        return 0;
    } else {
        printf("Normal checksum: %.2f\n", checksum);
        /* Another call at block end */
        return 1;
    }
}
