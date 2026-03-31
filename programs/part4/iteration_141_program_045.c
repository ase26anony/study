/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __x86_64__
#define CLOBBER_REGS "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_REGS "eax", "ecx", "edx", "esi", "edi"
#endif

/* Force noinline to ensure actual calls */
__attribute__((noinline)) 
void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers */
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline))
void external_func2(volatile long* p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline))
void external_func3(volatile int* p, int x) {
    *p ^= x;
    asm volatile("" : : : CLOBBER_REGS);
}

/* Mix of direct and indirect calls */
typedef void (*func_ptr_t)(volatile int*);

/* Force register usage with explicit register variables */
#ifdef __x86_64__
register long r10_val asm("r10");
register long r11_val asm("r11");
register long r8_val asm("r8");
register long r9_val asm("r9");
#endif

int main() {
    volatile int data[256];
    volatile long accum = 0;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Function pointers to inhibit optimization */
    func_ptr_t funcs[3] = {external_func1, external_func1, external_func2};
    
    /* Nested loops with register-intensive calculations */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables to increase pressure */
#ifdef __x86_64__
        register long r10 asm("r10") = outer * 100;
        register long r11 asm("r11") = outer * 200;
        register long r8 asm("r8") = outer * 300;
        register long r9 asm("r9") = outer * 400;
        r10_val = r10;
        r11_val = r11;
        r8_val = r8;
        r9_val = r9;
#endif
        
        for (int inner = 0; inner < 100; inner++) {
            /* Multiple live values in registers before call */
            volatile int a = data[inner % 256];
            volatile int b = data[(inner + 1) % 256];
            volatile int c = data[(inner + 2) % 256];
            volatile int d = data[(inner + 3) % 256];
            
            /* Complex arithmetic keeping values live */
            int temp1 = a * b + inner;
            int temp2 = c ^ d ^ inner;
            int temp3 = (a + b) * (c - d);
            
            /* Force values into call-clobbered registers */
            register int t1 asm("eax") = temp1;
            register int t2 asm("ecx") = temp2;
            register int t3 asm("edx") = temp3;
            
            /* Use inline assembly to prevent optimization */
            asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3));
            
            /* Call external function - values must be saved */
            if (inner % 3 == 0) {
                external_func1(&data[inner % 256]);
            } else if (inner % 3 == 1) {
                /* Indirect call */
                funcs[inner % 2](&data[(inner + 1) % 256]);
            } else {
                external_func3(&data[(inner + 2) % 256], inner);
            }
            
            /* Use the live values after call */
            accum += t1 + t2 + t3;
            
#ifdef __x86_64__
            /* Use explicit register variables after call */
            accum += r10_val + r11_val + r8_val + r9_val;
            /* Modify them to keep them live */
            r10_val += t1;
            r11_val += t2;
            r8_val += t3;
            r9_val += accum & 0xFF;
#endif
            
            /* More arithmetic with volatile to prevent reordering */
            volatile int x = data[inner % 256];
            volatile int y = data[(inner + 64) % 256];
            data[(inner + 128) % 256] = x * y + accum;
        }
        
        /* Another call with different register pressure */
        volatile long* ptr = &accum;
        external_func2(ptr);
        
        /* Mix of operations */
        for (int i = 0; i < 50; i++) {
            register int x asm("esi") = data[i];
            register int y asm("edi") = data[i + 50];
            
            asm volatile("" : "+r"(x), "+r"(y));
            
            /* Call with arguments in registers */
            external_func3(&data[i], x + y);
            
            /* Keep using the values */
            data[i + 100] = x - y;
        }
    }
    
    printf("Result: %ld\n", accum);
    
    /* Verify some computation was done */
    int check = 0;
    for (int i = 0; i < 256; i++) {
        check += data[i];
    }
    printf("Check sum: %d\n", check);
    
    return 0;
}
