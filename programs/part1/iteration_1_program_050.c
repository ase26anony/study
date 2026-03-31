#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx");
    return result;
}

/* Function with pointer arguments and float operations */
float __attribute__((noinline)) 
process_floats(float f1, float f2, float f3, float f4, float *out) {
    volatile float temp = f1 * f2 + f3 / f4;
    /* Force register clobbering */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    *out = temp;
    return temp;
}

/* Function that takes mixed types and uses alloca to affect frame pointer */
static void* __attribute__((noinline)) 
create_buffer(int size, int init_val) {
    /* alloca forces frame pointer usage */
    int* buffer = (int*)alloca(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        buffer[i] = init_val + i;
    }
    /* Clobber more registers */
    __asm__ volatile ("" : : : "r10", "r11", "r12", "r13");
    return buffer; /* Note: returning alloca pointer is unsafe in real code */
}

/* Function with many live values across calls */
int __attribute__((noinline)) 
complex_calculation(int iter) {
    volatile int v1 = iter * 2;
    volatile int v2 = iter + 100;
    volatile int v3 = iter / 3;
    volatile int v4 = iter - 50;
    volatile int v5 = iter % 7;
    
    /* Force register pressure by using all variables */
    int sum = v1 + v2 + v3 + v4 + v5;
    
    /* Inline assembly that clobbers specific registers */
    __asm__ volatile (
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        : 
        : "r" (v1), "r" (v2)
        : "eax", "memory"
    );
    
    return sum;
}

/* Recursive function to create more call depth */
static int __attribute__((noinline)) 
recursive_helper(int n, int acc) {
    if (n <= 0) return acc;
    
    volatile int local1 = n * 2;
    volatile int local2 = acc + n;
    
    /* More register clobbering */
    __asm__ volatile ("" : : : "rbx", "rcx", "rdx");
    
    return recursive_helper(n - 1, acc + local1 + local2);
}

/* Main function with high register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile float fa = 1.1f, fb = 2.2f, fc = 3.3f, fd = 4.4f;
    volatile int* ptr1 = &a;
    volatile float* ptr2 = &fa;
    volatile long long ll1 = 1000, ll2 = 2000;
    
    int result = 0;
    float fresult = 0.0f;
    
    /* Control flow to create basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        a = a + iteration;
        b = b * (iteration + 1);
        fa = fa * 1.5f;
        fb = fb / 1.1f;
        
        /* Call with many arguments - will exceed register passing on most ABIs */
        int sum1 = compute_sum(a, b, c, d, e, f, g, h, i, j);
        
        /* More computations keeping values live */
        c = c + sum1;
        d = d - iteration;
        
        /* Inline assembly that clobbers call-clobbered registers */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            : 
            : "r" (a), "r" (b)
            : "eax", "ebx", "memory"
        );
        
        if (iteration % 2 == 0) {
            /* Second basic block */
            float temp;
            fresult = process_floats(fa, fb, fc, fd, &temp);
            
            /* More register pressure */
            e = e + (int)temp;
            f = f * 2;
            
            /* Another inline assembly barrier */
            __asm__ volatile ("" : : : "ecx", "edx", "esi", "edi");
            
            /* Take address of local to influence frame pointer */
            volatile int* addr = &e;
            *addr = *addr + 1;
        } else {
            /* Third basic block with different call pattern */
            void* buffer = create_buffer(5, iteration);
            (void)buffer; /* Use to avoid unused warning */
            
            /* Complex calculation with many live values */
            int calc = complex_calculation(iteration);
            
            /* Keep values live across calls */
            g = g + calc;
            h = h - iteration;
            
            /* More register clobbering */
            __asm__ volatile ("" : : : "r8", "r9", "r10", "r11");
        }
        
        /* Call to recursive function */
        int rec_result = recursive_helper(2, iteration);
        
        /* Final computations mixing all values */
        i = i + rec_result;
        j = j * (iteration + 2);
        ll1 = ll1 + a + b + c;
        ll2 = ll2 - d - e - f;
        
        /* Another conditional creating basic block boundary */
        if (iteration > 0) {
            /* Use pointer operations */
            *ptr1 = *ptr1 + g + h;
            *ptr2 = *ptr2 + fresult;
            
            /* Final inline assembly */
            __asm__ volatile (
                "movq %0, %%rax\n\t"
                "addq %1, %%rax\n\t"
                : 
                : "r" (ll1), "r" (ll2)
                : "rax", "memory"
            );
        }
    }
    
    /* Compute checksum of all variables */
    result = a + b + c + d + e + f + g + h + i + j;
    result += (int)fa + (int)fb + (int)fc + (int)fd;
    result += (int)ll1 + (int)ll2;
    result += (int)fresult;
    
    printf("Result checksum: %d\n", result);
    
    /* Additional loop to increase instruction density */
    volatile int final = 0;
    for (int k = 0; k < 100; k++) {
        final += complex_calculation(k % 5);
        
        /* Insert calls at different points in the loop */
        if (k % 3 == 0) {
            float temp;
            process_floats(1.0f, 2.0f, 3.0f, 4.0f, &temp);
            final += (int)temp;
        }
        
        if (k % 4 == 0) {
            compute_sum(k, k+1, k+2, k+3, k+4, k+5, k+6, k+7, k+8, k+9);
        }
        
        /* More register clobbering in loop */
        __asm__ volatile ("" : : : "xmm4", "xmm5", "xmm6", "xmm7");
    }
    
    printf("Final result: %d\n", final + result);
    
    return 0;
}
