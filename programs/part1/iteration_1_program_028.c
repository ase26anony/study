#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with various attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, 
            float fa, float fb, double da, double db) {
    volatile int result = a + b + c + d + e + f + g + h;
    result += (int)(fa + fb);
    result += (int)(da + db);
    
    /* Force register pressure inside callee */
    int t1 = result * 2;
    int t2 = t1 + 7;
    float ft1 = fa * 1.5f;
    double dt1 = da * 2.5;
    
    /* Inline assembly to clobber registers */
    __asm__ volatile (
        "# Clobber in compute_sum\n"
        : 
        : "r"(t1), "r"(t2), "f"(ft1), "f"(dt1)
        : "eax", "rax", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    return result % 1000;
}

/* Function with pointer arguments */
static void __attribute__((noinline))
process_pointers(int *p1, float *p2, double *p3, volatile int *p4) {
    if (p1 && p2 && p3 && p4) {
        *p1 = (*p1) * 3 + 17;
        *p2 = (*p2) * 1.7f - 3.2f;
        *p3 = (*p3) / 2.0 + *p2;
        *p4 = (*p4) ^ 0x55AA55AA;
        
        /* More register pressure */
        int temp = *p1;
        for (int i = 0; i < 4; i++) {
            temp = (temp << 3) | (temp >> 29);
        }
        *p1 = temp;
    }
}

/* Function that uses alloca to affect frame pointer */
static int __attribute__((noinline))
use_alloca_and_calls(int size) {
    /* Taking address forces frame pointer usage */
    int local = size * 2;
    int *dynamic = alloca(size * sizeof(int));
    
    for (int i = 0; i < size && i < 8; i++) {
        dynamic[i] = local + i * 7;
    }
    
    int sum = 0;
    for (int i = 0; i < size && i < 8; i++) {
        sum += dynamic[i];
    }
    
    /* Call another function to create nested calls */
    volatile int vol_sum = sum;
    process_pointers(&local, NULL, NULL, &vol_sum);
    
    return local + sum;
}

/* Variadic-like function using many registers */
static double __attribute__((noinline))
mixed_type_computation(int a, float b, double c, int d, float e, double f,
                       int *g, float *h, double *i) {
    double result = c + f;
    result += (double)a + (double)b + (double)d + (double)e;
    
    if (g) result += *g;
    if (h) result += *h;
    if (i) result += *i;
    
    /* Force floating point register pressure */
    double d1 = result * 1.1;
    double d2 = result * 0.9;
    double d3 = result * 1.01;
    double d4 = result * 0.99;
    
    /* Clobber FP registers */
    __asm__ volatile (
        "# Clobber FP regs\n"
        : 
        : "f"(d1), "f"(d2), "f"(d3), "f"(d4)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    return result;
}

/* Main function with complex control flow and register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;
    int *p1 = &v1;
    float *p2 = &f1;
    double *p3 = &d1;
    
    /* Additional variables for more pressure */
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    float f5 = 5.5f, f6 = 6.6f;
    double d5 = 5.55, d6 = 6.66;
    
    /* Take addresses to force stack frame complexity */
    int *addr_array[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    
    int checksum = 0;
    
    /* Complex control flow creating multiple basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        int temp1 = v1 + v2 * v3 - v4;
        float temp2 = f1 * f2 + f3 / f4;
        double temp3 = d1 * d2 - d3 + d4;
        
        /* Inline assembly clobbering between computations */
        __asm__ volatile (
            "# Main loop clobber 1\n"
            : 
            : "r"(temp1), "r"(v5), "f"(temp2), "f"(temp3)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Function call with many arguments - will need caller-save */
        int sum_result = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8,
                                     f1, f2, d1, d2);
        
        /* Conditional creating new basic block */
        if (sum_result > 500) {
            /* More computations in this block */
            temp1 = sum_result * v9 + v10;
            temp2 = f5 * 2.0f - f6;
            
            /* Another function call */
            process_pointers(&temp1, &temp2, &temp3, &checksum);
            
            /* More inline assembly */
            __asm__ volatile (
                "# Conditional block clobber\n"
                :
                : "r"(temp1), "r"(sum_result)
                : "rax", "r10", "r11", "xmm0", "xmm1"
            );
        } else {
            /* Alternative path with different computations */
            temp3 = d5 * 3.14 + d6;
            double mixed = mixed_type_computation(v1, f1, d1, v2, f2, d2,
                                                  &v3, &f3, &d3);
            temp3 += mixed;
            
            /* Use alloca to affect frame pointer */
            int alloca_result = use_alloca_and_calls(iteration + 2);
            checksum += alloca_result;
        }
        
        /* Loop body continues with more computations */
        v1 = (v1 * 3 + 7) % 100;
        v2 = (v2 + v3 * 2) % 100;
        f1 = f1 * 1.5f - 0.3f;
        f2 = f2 / 1.3f + 0.7f;
        d1 = d1 * 2.0 - 1.0;
        d2 = d2 / 1.8 + 0.2;
        
        /* Another function call at end of loop body */
        double final_mix = mixed_type_computation(v4, f4, d4, v5, f5, d6,
                                                  &v6, &f6, &d5);
        checksum += (int)final_mix;
        
        /* Final clobber in loop */
        __asm__ volatile (
            "# Loop end clobber\n"
            :
            : "r"(v1), "r"(v2), "r"(checksum), "f"(f1), "f"(d1)
            : "rax", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
        );
    }
    
    /* Final computations and output */
    checksum = (checksum * 31 + 17) % 1000000;
    
    /* One more complex call sequence */
    for (int i = 0; i < 2; i++) {
        int temp = compute_sum(checksum, i, i*2, i*3, i*4, i*5, i*6, i*7,
                               1.0f, 2.0f, 3.0, 4.0);
        checksum ^= temp;
        
        /* Small conditional block with call */
        if (temp & 1) {
            process_pointers(&checksum, NULL, NULL, &checksum);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with simple calculation */
    int verify = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    verify += (int)(f1 + f2 + f3 + f4 + f5 + f6);
    verify += (int)(d1 + d2 + d3 + d4 + d5 + d6);
    printf("Verification sum: %d\n", verify % 1000);
    
    return checksum == (verify % 1000) ? 0 : 1;
}
