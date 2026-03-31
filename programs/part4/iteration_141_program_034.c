/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that clobber registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi"
#endif

/* Force noinline to ensure actual calls */
__attribute__((noinline)) void external_func1(volatile int* p) {
    *p += 1;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func2(volatile int* p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func3(volatile int* p) {
    *p -= 3;
    asm volatile("" : : : CLOBBER_LIST);
}

/* Complex calculation using many registers */
int compute_value(int a, int b, int c, int d, int e, int f) {
    /* Force values into registers with volatile operations */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    
    /* Mix of operations to create register pressure */
    int result = (v1 * v2) + (v3 / (v4 ? v4 : 1)) - (v5 ^ v6);
    
    /* Inline assembly to prevent optimization and create dependencies */
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

int main() {
    /* Large array to work with */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Function pointer array to force indirect calls */
    void (*funcs[3])(volatile int*) = {
        external_func1,
        external_func2,
        external_func3
    };
    
    int total_sum = 0;
    
    /* Outer loop with register-intensive calculations */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with function calls and live values across calls */
        for (int i = 0; i < 100; i++) {
            /* Explicit register variables to target specific registers */
            #ifdef __x86_64__
            register long r10 asm("r10") = data[i % 256];
            register long r11 asm("r11") = data[(i + 1) % 256];
            register long r12 asm("r12") = data[(i + 2) % 256]; /* Call-saved */
            register long r13 asm("r13") = data[(i + 3) % 256]; /* Call-saved */
            #else
            register int r10 asm("eax") = data[i % 256];
            register int r11 asm("ecx") = data[(i + 1) % 256];
            register int r12 asm("esi") = data[(i + 2) % 256];
            register int r13 asm("edi") = data[(i + 3) % 256];
            #endif
            
            /* Complex computation keeping values live in registers */
            int temp1 = compute_value(r10, r11, r12, r13, i, outer);
            
            /* Volatile to prevent reordering */
            volatile int preserve1 = temp1;
            
            /* Function call that clobbers registers */
            funcs[i % 3]((volatile int*)&data[i % 256]);
            
            /* More computations using the preserved values */
            int temp2 = compute_value(preserve1, r10, r11, r12, r13, i);
            
            /* Another volatile operation */
            volatile int preserve2 = temp2;
            
            /* Second function call */
            funcs[(i + 1) % 3]((volatile int*)&data[(i + 1) % 256]);
            
            /* Use all values after calls */
            int final_val = preserve1 + preserve2 + r10 + r11 + r12 + r13;
            
            /* Store result back, creating memory pressure */
            data[i % 256] = final_val % 1000;
            total_sum += final_val;
            
            /* Inline assembly to create artificial dependencies */
            asm volatile("" : : "r"(r10), "r"(r11), "r"(r12), "r"(r13) : "memory");
        }
        
        /* Nested loop with different pattern */
        for (int j = 0; j < 50; j++) {
            #ifdef __x86_64__
            register long rcx asm("rcx") = data[j * 2];
            register long rdx asm("rdx") = data[j * 2 + 1];
            #else
            register int ecx asm("ecx") = data[j * 2];
            register int edx asm("edx") = data[j * 2 + 1];
            #endif
            
            /* Multiple live values before call */
            int sum = rcx + rdx;
            int product = rcx * rdx;
            
            /* Call with live values */
            external_func1((volatile int*)&data[j * 2]);
            
            /* Use values after call */
            total_sum += sum + product;
            
            /* Force spill/reload */
            asm volatile("" : "+r"(sum), "+r"(product) : : "memory");
        }
    }
    
    printf("Result: %d\n", total_sum % 1000000);
    return 0;
}
