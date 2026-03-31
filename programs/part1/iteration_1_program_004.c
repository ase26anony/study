#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with varying attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx");
    return result;
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float *arr, int count, float scale) {
    volatile float sum = 0.0f;
    for (int i = 0; i < count; i++) {
        sum += arr[i] * scale;
        /* Force register pressure with inline assembly */
        __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    }
    return sum;
}

/* Function that uses alloca to affect frame pointer */
static void* __attribute__((noinline))
create_buffer(int size) {
    /* Taking address of alloca result forces frame pointer usage */
    void *ptr = alloca(size + 16);
    volatile int *vptr = (volatile int*)ptr;
    for (int i = 0; i < size / sizeof(int); i++) {
        vptr[i] = i * 2;
    }
    return ptr;
}

/* Function with mixed types in arguments */
static double __attribute__((noinline))
mixed_calculation(int a, float b, double c, int *d, float *e) {
    volatile double result = (double)a + (double)b + c;
    *d = (int)result;
    *e = (float)(result * 2.0);
    
    /* Clobber multiple register classes */
    __asm__ volatile ("" : : : "rax", "r10", "r11", "xmm3", "xmm4", "xmm5");
    
    return result;
}

/* Recursive function to create complex call graph */
static int __attribute__((noinline))
recursive_helper(int n, int *counter) {
    if (n <= 0) return 1;
    
    volatile int local = n * 2;
    (*counter) += local;
    
    /* Force register pressure between recursive calls */
    __asm__ volatile ("" : : : "rbx", "r12", "r13");
    
    return recursive_helper(n - 1, counter) * local;
}

/* Main computation with high register pressure */
static int __attribute__((noinline))
complex_sequence(int seed) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = seed * 1;
    volatile int v2 = seed * 2;
    volatile int v3 = seed * 3;
    volatile int v4 = seed * 4;
    volatile int v5 = seed * 5;
    volatile float f1 = seed * 1.1f;
    volatile float f2 = seed * 2.2f;
    volatile float f3 = seed * 3.3f;
    volatile double d1 = seed * 1.11;
    volatile double d2 = seed * 2.22;
    int *ptr1 = (int*)&v1;
    float *ptr2 = &f1;
    double *ptr3 = &d1;
    
    /* Create control flow with basic blocks containing calls */
    int result = 0;
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            /* First call site - many arguments */
            v1 = compute_sum(v1, v2, v3, v4, v5, 
                           v1 + 1, v2 + 2, v3 + 3, v4 + 4, v5 + 5);
            
            /* Inline assembly between calls to create live ranges */
            __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            
            /* Second call - pointer arguments */
            float arr[8] = {f1, f2, f3, f1*2, f2*2, f3*2, f1*3, f2*3};
            f1 = process_floats(arr, 8, 1.5f);
            
            /* Force register spill/reload */
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
        } else {
            /* Third call - mixed types */
            d1 = mixed_calculation(v1, f1, d1, &v2, &f2);
            
            /* More register clobbering */
            __asm__ volatile ("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            
            /* Fourth call - uses alloca */
            void *buffer = create_buffer(64);
            volatile int *buf_int = (volatile int*)buffer;
            v3 = buf_int[0] + buf_int[1];
        }
        
        /* Interleave computations between calls */
        v4 = v1 * v2 + v3;
        f2 = f1 * 3.14f;
        d2 = d1 * 2.71828;
        
        result += v4 + (int)f2 + (int)d2;
    }
    
    return result;
}

/* Function with loop creating multiple basic blocks */
static int __attribute__((noinline))
loop_with_calls(int iterations) {
    volatile int acc = 0;
    volatile float f_acc = 0.0f;
    volatile double d_acc = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create a basic block boundary with conditional */
        if (i > 0 && i % 5 == 0) {
            /* Call in middle of loop with register pressure */
            int temp[10];
            for (int j = 0; j < 10; j++) temp[j] = i + j;
            
            acc += compute_sum(temp[0], temp[1], temp[2], temp[3], temp[4],
                             temp[5], temp[6], temp[7], temp[8], temp[9]);
            
            /* Force caller-save around this call */
            __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                                            "r8", "r9", "r10", "r11");
        } else if (i % 3 == 0) {
            /* Different call pattern */
            float arr[4] = {f_acc, f_acc * 2, f_acc * 3, f_acc * 4};
            f_acc += process_floats(arr, 4, (float)i);
            
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                                            "xmm4", "xmm5", "xmm6", "xmm7");
        }
        
        /* Keep values live across calls */
        acc += i;
        f_acc += (float)acc * 0.1f;
        d_acc += (double)f_acc * 0.01;
    }
    
    return acc + (int)f_acc + (int)d_acc;
}

int main(void) {
    printf("Starting caller-save test...\n");
    
    volatile int checksum = 0;
    
    /* Test 1: Complex sequence with high register pressure */
    for (int i = 0; i < 10; i++) {
        checksum += complex_sequence(i);
        
        /* Recursive call to create deep call stack */
        int counter = 0;
        checksum += recursive_helper(3, &counter);
    }
    
    /* Test 2: Loop with multiple call sites */
    checksum += loop_with_calls(20);
    
    /* Test 3: Nested calls with varying signatures */
    {
        volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
        volatile float f = 1.5f, g = 2.5f;
        volatile double h = 3.14159;
        
        for (int i = 0; i < 5; i++) {
            /* Chain of calls keeping values live */
            int sum = compute_sum(a, b, c, d, e, a+i, b+i, c+i, d+i, e+i);
            
            float arr[6] = {f, g, f*2, g*2, f*3, g*3};
            float fsum = process_floats(arr, 6, (float)sum);
            
            double dresult = mixed_calculation(sum, fsum, h, &a, &f);
            
            checksum += (int)dresult;
            
            /* Force register shuffling */
            __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                                        "r8", "r9", "r10", "r11", "r12", "r13",
                                        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
                                        "xmm5", "xmm6", "xmm7", "xmm8", "xmm9");
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify execution */
    if (checksum != 0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Test failed - checksum is zero.\n");
        return 1;
    }
}
