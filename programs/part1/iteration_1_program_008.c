#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function with many arguments to exceed register passing limits */
__attribute__((noinline))
static int many_args_func(int a, int b, int c, int d, int e, 
                         int f, int g, int h, int i, int j,
                         float k, float l, double m) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    result += (int)(k + l + m);
    return result;
}

/* Function returning a value, potentially using different registers */
__attribute__((noinline))
double compute_value(double base, int multiplier, float factor) {
    volatile double temp = base;
    for (int i = 0; i < multiplier; i++) {
        temp *= factor;
        /* Use alloca to affect frame pointer */
        if (i % 4 == 0) {
            char* buf = alloca(64);
            buf[0] = (char)(temp * 0.01);
        }
    }
    return temp;
}

/* Static function that might be inlined or not depending on optimization */
static __attribute__((noinline)) 
int static_noinline_func(int x, int* ptr) {
    volatile int local = x * 2;
    *ptr += local;
    
    /* Inline assembly that clobbers specific registers */
    __asm__ volatile (
        "# Force clobber\n"
        : 
        : 
        : "rax", "r10", "r11", "xmm0", "xmm1"
    );
    
    return local;
}

/* Function with pointer arguments to create address pressure */
__attribute__((noinline))
void pointer_heavy_func(int* a, float* b, double* c, 
                       int** d, volatile int* e) {
    *a = (*a) * 2 + 1;
    *b = (*b) * 3.14f;
    *c = (*c) / 2.0;
    
    if (d && *d) {
        **d = (**d) ^ 0x55AA55AA;
    }
    
    if (e) {
        *e = (*e) | 0x0000FFFF;
    }
    
    /* More inline assembly clobbering */
    __asm__ volatile (
        "# More clobbers\n"
        : 
        : 
        : "rbx", "r12", "xmm2", "xmm3", "xmm4"
    );
}

/* Function that creates control flow complexity */
__attribute__((noinline))
int control_flow_func(int seed, int iterations) {
    volatile int result = seed;
    int* dynamic_ptr = NULL;
    
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            /* Create basic block boundary with call inside */
            int temp = many_args_func(i, i+1, i+2, i+3, i+4,
                                     i+5, i+6, i+7, i+8, i+9,
                                     i*0.1f, i*0.2f, i*0.3);
            result += temp;
            
            /* Use alloca to force frame pointer usage */
            if (i % 5 == 0) {
                dynamic_ptr = alloca(sizeof(int));
                *dynamic_ptr = i;
            }
        } 
        else if (i % 3 == 1) {
            double val = compute_value(result, i % 10, 1.1f);
            result += (int)val;
            
            /* Inline assembly between computations */
            __asm__ volatile (
                "# Intermediate clobber\n"
                : 
                : 
                : "r13", "r14", "xmm5", "xmm6"
            );
        }
        else {
            static_noinline_func(result, &result);
            
            /* Complex expression to keep values live */
            result = (result * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    return result;
}

/* Main function creating maximum register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    int* ptr1 = &v1;
    float* ptr2 = &f1;
    double* ptr3 = &d1;
    int** ptr4 = &ptr1;
    volatile int* ptr5 = &v2;
    
    int arr[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    float farr[6] = {1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f};
    
    /* Initial computations keeping values in registers */
    int sum_int = v1 + v2 + v3 + v4 + v5;
    float sum_float = f1 + f2 + f3 + f4;
    double sum_double = d1 + d2 + d3;
    
    /* First function call with many live values */
    int result1 = many_args_func(v1, v2, v3, v4, v5,
                                arr[0], arr[1], arr[2], arr[3], arr[4],
                                f1, f2, d1);
    
    /* Inline assembly clobbering call-clobbered registers */
    __asm__ volatile (
        "# Major clobber between calls\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* More computations using clobbered values (must be reloaded) */
    sum_int += result1;
    sum_float *= 1.5f;
    sum_double /= 2.0;
    
    /* Complex control flow with calls inside basic blocks */
    for (int i = 0; i < 100; i++) {
        if (i % 7 == 0) {
            /* Call inside basic block */
            double val = compute_value(sum_double, i % 8, farr[i % 6]);
            sum_double += val;
            
            /* Take address of local to affect frame pointer */
            int* local_ptr = &arr[i % 8];
            static_noinline_func(*local_ptr, &sum_int);
        }
        else if (i % 7 == 1 || i % 7 == 2) {
            /* Multiple calls in sequence */
            pointer_heavy_func(&arr[i % 8], &farr[i % 6], &d3, ptr4, ptr5);
            
            __asm__ volatile (
                "# Selective clobber\n"
                : 
                : 
                : "rax", "rdx", "xmm0", "xmm1"
            );
            
            arr[i % 8] = many_args_func(arr[0], arr[1], arr[2], arr[3], arr[4],
                                       arr[5], arr[6], arr[7], v1, v2,
                                       farr[0], farr[1], d2);
        }
        else {
            /* Nested control flow */
            int temp = control_flow_func(i, 5 + (i % 3));
            
            if (temp % 2 == 0) {
                sum_int += temp;
                farr[i % 6] = farr[i % 6] * 0.9f + 0.1f;
            } else {
                sum_int -= temp;
                d3 = d3 * 0.95 + 0.05;
            }
            
            /* Another alloca to affect stack */
            if (i % 11 == 0) {
                char* buffer = alloca(128);
                for (int j = 0; j < 128; j++) {
                    buffer[j] = (char)((i + j) & 0xFF);
                }
            }
        }
        
        /* Loop-carried dependency to keep values live */
        sum_int = (sum_int * 6364136223846793005ULL + 1442695040888963407ULL) & 0x7FFFFFFF;
    }
    
    /* Final complex call sequence */
    int final_result = control_flow_func(sum_int, 25);
    
    /* Mix all results */
    int checksum = final_result ^ sum_int ^ (int)sum_float ^ (int)sum_double;
    
    for (int i = 0; i < 8; i++) {
        checksum += arr[i];
        checksum ^= (int)(farr[i % 6] * 1000);
    }
    
    checksum += (int)(d1 * 100) + (int)(d2 * 100) + (int)(d3 * 100);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify execution */
    if (checksum != 0) {
        printf("Program executed successfully with non-zero checksum\n");
    }
    
    return checksum == 0 ? 1 : 0;
}
