#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, int l, int m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix arithmetic operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + j;
    int t3 = f ^ g | h;
    double t4 = sin(i) * cos(a);
    
    /* Function call within recursion to create more caller-save opportunities */
    printf("Depth %d: t1=%.3f t2=%.3f t3=%d t4=%.3f\n", 
           depth, t1, t2, t3, t4);
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           t1, b + 1.0, c * 0.9,
                           t2, e * 1.1f,
                           t3, g + 1, h * 2,
                           t4, j * 0.8f,
                           k + l, m - 1, depth, rand() % 100);
}

int main() {
    srand(time(NULL));
    
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 5.0f, f2 = 6.0f, f3 = 7.0f, f4 = 8.0f;
    int i1 = 9, i2 = 10, i3 = 11, i4 = 12, i5 = 13, i6 = 14, i7 = 15;
    
    /* Variable to track loop iterations */
    int loop_counter = 0;
    
    /* First loop with high register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Arithmetic operations on all variables before function calls */
        d1 = d1 * 1.1 + sin(d2);
        d2 = d2 * 0.9 - cos(d3);
        d3 = d3 + tan(d4);
        d4 = pow(d4, 1.01);
        
        f1 = f1 * 1.05f + f2;
        f2 = f2 / 1.03f - f3;
        f3 = f3 + sqrtf(f4);
        f4 = f4 * 0.97f;
        
        i1 = i1 * 3 + i2;
        i2 = i2 ^ i3;
        i3 = i3 | i4;
        i4 = i4 & i5;
        i5 = i5 + i6 * 2;
        i6 = i6 - i7;
        i7 = i7 * 2 + outer;
        
        /* Inline assembly that clobbers multiple caller-saved registers */
        /* Clobber eax, ecx, edx (x86) - adjust for your architecture if needed */
        asm volatile (
            "add %[val1], %[val2]"
            : [val2] "+r" (i2)
            : [val1] "r" (i1)
            : "%eax", "%ecx", "%edx"
        );
        
        /* Function call with many live variables */
        printf("Loop %d: d1=%.3f d2=%.3f f1=%.3f i1=%d i2=%d\n", 
               outer, d1, d2, f1, i1, i2);
        
        /* Conditional branch creating basic block boundary */
        if (outer % 2 == 0) {
            /* Path 1: Function call at the end of basic block */
            d1 = sin(d1) * cos(d2);
            f1 = powf(f1, 1.5f);
            i1 = rand() % 100 + i1;
            
            /* Call at block end - may trigger BB_END logic */
            printf("Even iteration: %f %f %d\n", d1, f1, i1);
            goto process_data;  /* Unusual control flow */
        } else {
            /* Path 2: Different pattern with calls in middle */
            d2 = log(d2 + 1.0);
            f2 = expf(f2 * 0.5f);
            
            /* Another function call */
            double result = pow(d3, d4);
            printf("Odd iteration: result=%.3f\n", result);
            
            /* Continue in this block */
            i2 = abs(i2 * 2 - i3);
        }
        
        /* Label for goto target */
        process_data:
        
        /* More arithmetic to keep variables live */
        d3 = d3 * 2.0 - d4 / 3.0;
        f3 = f3 + f4 * 0.25f;
        i3 = i3 * 7 - i4;
        
        /* Another inline assembly block */
        asm volatile (
            "imul %[val1], %[val2]"
            : [val2] "+r" (i4)
            : [val1] "r" (i3)
            : "%eax", "%edx"
        );
        
        /* Call recursive function with many arguments */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, d3, f1, f2, 
            i1, i2, i3, d4, f3, 
            i4, i5, i6, i7
        );
        
        printf("Recursive result: %.3f\n", rec_result);
        
        /* Another conditional with call at end of block */
        if (rec_result > 100.0) {
            d4 = sqrt(rec_result);
            f4 = (float)log(rec_result);
            i5 = (int)rec_result % 1000;
            
            /* Function call right before block end */
            printf("Large result processed: %.3f\n", d4);
        } else {
            /* Different path with multiple calls */
            double s1 = sin(rec_result);
            double s2 = cos(rec_result);
            printf("Small result: sin=%.3f cos=%.3f\n", s1, s2);
            
            /* Another call */
            i6 = rand() % 50 + i6;
        }
        
        /* Final arithmetic in loop */
        d1 += rec_result * 0.1;
        f1 += (float)rec_result * 0.05f;
        i7 += (int)rec_result;
        
        loop_counter++;
    }
    
    /* Compute final checksum from all variables */
    double checksum = d1 + d2 + d3 + d4 + 
                     f1 + f2 + f3 + f4 + 
                     i1 + i2 + i3 + i4 + i5 + i6 + i7 + 
                     loop_counter;
    
    /* Final function call */
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)checksum % 100;
}
