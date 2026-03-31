#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, double l, float m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix arithmetic operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + j;
    int t3 = f ^ g | h;
    double t4 = sin(i) * cos(l);
    float t5 = m * 2.0f - j;
    
    /* Call external function within recursion */
    double ext = pow(t1, 2.0) + sin(t4);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r"(t3)
        : "r"(k), "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with mixed arguments */
    return ext + recursive_helper(depth - 1, 
           t1 + 1.0, b * 0.5, c - t4, 
           t2, t5, 
           t3, g >> 1, h << 1, 
           i + l, j * 0.8f, 
           k ^ 0xFF, l * 0.9, m + 0.1f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables before calls */
        d1 = d1 * 1.1 + sin(d2);
        d2 = d2 * 0.9 + cos(d3);
        d3 = d3 * 1.05 + tan(d4);
        d4 = d4 * 0.95 + atan(d1);
        
        f1 = f1 * 1.05f + 0.1f;
        f2 = f2 * 0.95f + 0.2f;
        f3 = f3 * 1.1f + f4;
        f4 = f4 * 0.9f + f1;
        
        i1 = i1 * 3 + i2;
        i2 = i2 * 5 - i3;
        i3 = i3 * 7 ^ i4;
        i4 = i4 * 11 | i5;
        i5 = i5 * 13 & i6;
        i6 = i6 * 17 + outer;
        
        /* First function call - printf with many arguments */
        printf("Iteration %d: d1=%.3f f1=%.3f i1=%d\n", 
               outer, d1, f1, i1);
        
        /* Inline assembly clobbering caller-saved registers */
        asm volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "imul %3, %%eax"
            : "=r"(i1)
            : "r"(i2), "r"(i3), "r"(i4)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (d1 > 2.0) {
            /* Path 1: Call at end of basic block */
            d2 = pow(d1, d3) + sin(d4);
            f2 = fabsf(f3 - f4) * 2.0f;
            
            /* Function call right before goto (potential BB_END) */
            i3 = rand() % 100;
            goto compute_block;  /* This creates a basic block ending with rand() call */
        } else {
            /* Path 2: Different call pattern */
            d2 = log(d1) * exp(d3);
            f2 = sqrtf(f3 * f4);
            
            /* Another function call */
            i4 = abs(i5 - i6) * 2;
        }
        
        /* Label to create unusual BB structure */
        compute_block:
        
        /* Second function call - math function */
        double trig_sum = sin(d1) + cos(d2) + tan(d3);
        
        /* More arithmetic to keep variables live across calls */
        d4 = d4 + trig_sum * 0.5;
        f4 = f4 + (float)trig_sum * 0.3f;
        
        /* Third function call - recursive helper with many arguments */
        double rec_result = recursive_helper(2, 
                          d1, d2, d3, f1, f2,
                          i1, i2, i3, d4, f3, i4, d1 + d2, f4);
        
        /* Inline assembly between calls */
        asm volatile (
            "xchg %1, %0\n\t"
            "sub %2, %0"
            : "+r"(i5), "+r"(i6)
            : "r"(outer)
            : "cc"
        );
        
        /* Fourth function call - another printf */
        printf("Recursive result: %.3f\n", rec_result);
        
        /* Arithmetic after calls */
        checksum += d1 + d2 + d3 + d4 + f1 + f2 + f3 + f4 + 
                   i1 + i2 + i3 + i4 + i5 + i6 + rec_result;
        
        /* Nested loop for additional pressure */
        for (int inner = 0; inner < 2; inner++) {
            /* More arithmetic */
            d1 = d1 * 0.99 + inner * 0.1;
            f1 = f1 * 1.01f - inner * 0.05f;
            
            /* Function call inside nested loop */
            i2 = rand() % 50 + i1;
            
            /* Inline assembly in nested loop */
            asm volatile (
                "addl $1, %0\n\t"
                "subl $1, %1"
                : "+r"(i3), "+r"(i4)
                :
                : "cc"
            );
        }
        
        /* Another conditional with call at block end */
        if (checksum > 100.0) {
            d3 = pow(d2, 1.5);
            f3 = sinf(f2 * 3.14159f);
            /* Call at potential BB_END */
            i1 = abs(i2 - i3);
            goto finalize_iteration;  /* Basic block ends with abs() call */
        }
        
        /* Default path */
        d3 = sqrt(d2);
        f3 = cosf(f2);
        i1 = i2 + i3;
        
        finalize_iteration:
        /* Continue loop */
    }
    
    /* Final computation and output */
    checksum = fmod(checksum, 1000.0);
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)(checksum * 1000) % 256;
}
