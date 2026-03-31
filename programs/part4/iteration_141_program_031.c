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
__attribute__((noinline)) int complex_calculation(int a, int b, int c, int d, 
                                                  int e, int f, int g, int h) {
    /* Use volatile to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    
    /* Force register usage with inline asm */
    int r1, r2, r3, r4;
    asm volatile("mov %1, %0" : "=r"(r1) : "r"(v1));
    asm volatile("mov %1, %0" : "=r"(r2) : "r"(v2));
    asm volatile("mov %1, %0" : "=r"(r3) : "r"(v3));
    asm volatile("mov %1, %0" : "=r"(r4) : "r"(v4));
    
    return r1 * r2 + r3 * r4 + e * f - g * h;
}

int main() {
    int data[256];
    int sum = 0;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Function pointer array to force indirect calls */
    void (*func_array[3])(int *) = {
        external_func1,
        external_func2,
        external_func3
    };
    
    /* Use explicit register variables to increase pressure */
#ifdef __x86_64__
    register long r10 asm("r10");
    register long r11 asm("r11");
    register long r12 asm("r12");  /* Call-saved register */
    register long r13 asm("r13");  /* Call-saved register */
#else
    register int r10 asm("eax");
    register int r11 asm("ecx");
    register int r12 asm("ebx");   /* Call-saved register */
    register int r13 asm("esi");   /* Call-saved register */
#endif
    
    /* Nested loops with function calls and register-intensive calculations */
    for (int outer = 0; outer < 10; outer++) {
        r12 = outer * 100;
        r13 = outer * 200;
        
        for (int inner = 0; inner < 100; inner++) {
            /* Load values into call-clobbered registers */
            r10 = data[inner % 256];
            r11 = data[(inner + 1) % 256];
            
            /* Keep values live across computation */
            int temp1 = r10 + r11;
            int temp2 = r10 - r11;
            
            /* Mix with call-saved registers */
            temp1 += r12;
            temp2 += r13;
            
            /* Complex calculation that uses many registers */
            int result = complex_calculation(
                temp1, temp2, r10, r11,
                r12, r13, inner, outer
            );
            
            /* Call external function via pointer - forces caller-save */
            func_array[inner % 3](&result);
            
            /* Use values after call - they need to be restored */
            sum += result + r10 + r11 + r12 + r13;
            
            /* More arithmetic to create live ranges */
            r10 = (r10 * 3) / 2;
            r11 = (r11 * 5) / 4;
            r12 += inner;
            r13 += outer;
            
            /* Another call with different pattern */
            if (inner % 5 == 0) {
                external_func1(&sum);
            } else if (inner % 5 == 1) {
                external_func2(&sum);
            } else {
                external_func3(&sum);
            }
            
            /* Store back using all registers */
            data[inner % 256] = (r10 + r11 + r12 + r13) % 1000;
        }
        
        /* Additional computation between loops */
        for (int i = 0; i < 50; i++) {
            r10 = data[i] * 2;
            r11 = data[i + 50] * 3;
            
            /* Call within computation */
            external_func1(&r10);
            
            data[i] = r10 + r11;
            
            /* Force spill/reload with volatile */
            volatile int barrier = r10;
            r10 = barrier + r11;
            
            external_func2(&r11);
            
            data[i + 50] = r10 - r11;
        }
    }
    
    printf("Final sum: %d\n", sum);
    
    /* Verify computation */
    int verify = 0;
    for (int i = 0; i < 256; i++) {
        verify += data[i];
    }
    printf("Data sum: %d\n", verify);
    
    return 0;
}
