/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_LIST "eax", "ecx", "edx"
#endif

/* Force noinline to ensure actual calls */
__attribute__((noinline)) void external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func2(int *p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func3(int *p) {
    *p -= 3;
    asm volatile("" : : : CLOBBER_LIST);
}

/* Complex calculation using many registers */
__attribute__((noinline)) int compute_value(int a, int b, int c, int d) {
    /* Use volatile to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    
    /* Force register usage with inline asm */
    int result;
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        "imul %3, %0\n\t"
        "sub %4, %0"
        : "=r"(result)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4)
        : "cc"
    );
    
    return result;
}

int main() {
    /* Large array to create register pressure */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Function pointer array to force indirect calls */
    void (*funcs[3])(int *) = {external_func1, external_func2, external_func3};
    
    int sum = 0;
    
    /* Nested loops with calls and register-intensive calculations */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables to target specific registers */
        #ifdef __x86_64__
        register long r10 asm("r10") = outer * 100;
        register long r11 asm("r11") = outer * 200;
        #else
        register int r10 asm("eax") = outer * 100;
        register int r11 asm("ecx") = outer * 200;
        #endif
        
        for (int inner = 0; inner < 100; inner++) {
            /* Load multiple values into registers */
            int idx1 = (inner * 7) % 256;
            int idx2 = (inner * 13) % 256;
            int idx3 = (inner * 17) % 256;
            int idx4 = (inner * 23) % 256;
            
            /* Keep values live in registers across function calls */
            int val1 = data[idx1] + r10;
            int val2 = data[idx2] + r11;
            int val3 = data[idx3] - r10;
            int val4 = data[idx4] - r11;
            
            /* Complex computation that uses many registers */
            int temp = compute_value(val1, val2, val3, val4);
            
            /* Call external function - forces caller-save for live registers */
            funcs[inner % 3](&temp);
            
            /* Use the values after the call - they need to be restored */
            sum += temp + val1 + val2 + val3 + val4;
            
            /* Update array with results */
            data[idx1] = val1 ^ temp;
            data[idx2] = val2 ^ temp;
            
            /* More register pressure */
            r10 += inner;
            r11 -= inner;
            
            /* Another call with different pattern */
            if (inner % 5 == 0) {
                external_func1(&sum);
            } else if (inner % 5 == 1) {
                external_func2(&sum);
            } else {
                external_func3(&sum);
            }
            
            /* More computation after call */
            sum = compute_value(sum, val1, val2, temp);
        }
        
        /* Mix of direct and indirect calls */
        for (int i = 0; i < 20; i++) {
            int idx = (outer * 31 + i) % 256;
            
            /* Force values into registers */
            volatile int v = data[idx];
            volatile int w = sum;
            
            /* Inline asm to create artificial dependencies */
            asm volatile (
                "add %1, %0\n\t"
                "ror $3, %0"
                : "+r"(v)
                : "r"(w)
                : "cc"
            );
            
            /* Call with live values */
            external_func2(&v);
            
            /* Use result */
            sum += v;
            data[idx] = v;
            
            /* Another computation */
            asm volatile (
                "imul %1, %0"
                : "+r"(sum)
                : "r"(i)
                : "cc"
            );
        }
    }
    
    printf("Final sum: %d\n", sum);
    
    /* Verify result isn't optimized away */
    volatile int check = sum;
    if (check == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
