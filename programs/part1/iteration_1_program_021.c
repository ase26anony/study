#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes and calling conventions */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Force register pressure in callee too */
    volatile int x1 = result * 2;
    volatile int x2 = result / 3;
    volatile int x3 = x1 ^ x2;
    volatile int x4 = x3 << 2;
    return x4;
}

/* Function with pointer arguments */
float __attribute__((noinline)) 
process_floats(float* f1, float* f2, float* f3, float* f4, float* f5) {
    volatile float sum = *f1 + *f2 + *f3 + *f4 + *f5;
    /* More register pressure */
    volatile float t1 = sum * 1.5f;
    volatile float t2 = sum / 2.0f;
    volatile float t3 = t1 - t2;
    volatile float t4 = t3 * t3;
    return t4;
}

/* Function that clobbers specific registers via inline asm */
void __attribute__((noinline))
clobber_registers(void) {
    /* Force clobbering of call-clobbered registers */
    __asm__ volatile (
        "# Clobber caller-save registers\n"
        "mov $0x12345678, %%eax\n"
        "mov $0x87654321, %%ecx\n"
        "mov $0x11111111, %%edx\n"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ecx", "edx", "memory"
    );
}

/* Function using alloca to affect frame pointer */
int* __attribute__((noinline))
create_local_array(int size) {
    /* alloca forces frame pointer usage */
    int* arr = (int*)alloca(size * sizeof(int));
    volatile int i;
    for (i = 0; i < size && i < 8; i++) {
        arr[i] = i * i;
    }
    return arr; /* Note: returning alloca pointer is dangerous in real code */
}

/* Mixed type computation function */
double __attribute__((noinline))
mixed_computation(int a, float b, double c, int* d, float* e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    
    /* Inline asm that clobbers more registers */
    __asm__ volatile (
        "# More register clobbering\n"
        "mov $0xAAAAAAAA, %%r10d\n"
        "mov $0xBBBBBBBB, %%r11d\n"
        : /* no outputs */
        : /* no inputs */
        : "r10", "r11", "memory"
    );
    
    return result * 2.0;
}

/* Main function with high register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    
    /* Take addresses to force stack usage and affect frame pointer */
    volatile int* addr1 = &v1;
    volatile int* addr2 = &v2;
    volatile int* addr3 = &v3;
    
    /* Control flow to create basic blocks */
    int checksum = 0;
    
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic Block 1: Multiple computations keeping values in registers */
        int temp1 = v1 * v2 + v3 - v4;
        float temp2 = f1 * f2 - f3;
        double temp3 = d1 + d2 * d3;
        
        /* Inline asm clobbering call-clobbered registers between computations */
        __asm__ volatile (
            "# Clobber between computations\n"
            "mov $0xDEADBEEF, %%eax\n"
            "mov $0xCAFEBABE, %%ecx\n"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ecx", "memory"
        );
        
        /* Function call with many arguments - forces register pressure */
        int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* Basic Block 2: More computations */
        if (sum1 > 100) {
            /* Conditional creates new basic block */
            temp1 = sum1 / 2;
            temp2 = temp2 * 1.5f;
            
            /* Another function call inside basic block */
            float float_result = process_floats(&f1, &f2, &f3, &f4, &f5);
            
            /* More inline asm */
            __asm__ volatile (
                "# Conditional path clobber\n"
                "mov $0x11112222, %%edx\n"
                : /* no outputs */
                : /* no inputs */
                : "edx", "memory"
            );
            
            temp3 = temp3 + (double)float_result;
        } else {
            /* Alternative path */
            clobber_registers();
            temp1 = temp1 * 3;
        }
        
        /* Basic Block 3: Mixed type function call */
        double mixed_result = mixed_computation(v1, f1, d1, &v2, &f2);
        
        /* Force spill/reload around call */
        __asm__ volatile (
            "# Post-call clobber\n"
            "mov $0x33334444, %%r10d\n"
            : /* no outputs */
            : /* no inputs */
            : "r10", "memory"
        );
        
        /* Use alloca to affect frame pointer decisions */
        int* local_arr = create_local_array(10);
        (void)local_arr; /* Prevent unused warning */
        
        /* More computations to keep values live across calls */
        v1 = v1 + 1;
        v2 = v2 * 2;
        f1 = f1 + 0.5f;
        d1 = d1 * 1.1;
        
        /* Accumulate checksum from all values */
        checksum += temp1 + (int)temp2 + (int)temp3 + sum1 + (int)mixed_result;
        
        /* Another call to create more caller-save opportunities */
        int sum2 = compute_sum(v2, v3, v4, v5, v6, v7, v8, v9, v10, v1);
        checksum += sum2;
    }
    
    /* Final computation and output */
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with expected value */
    volatile int expected = 0; /* Will be different each run */
    if (checksum != expected) {
        printf("Result verification (expected difference)\n");
    }
    
    return 0;
}
