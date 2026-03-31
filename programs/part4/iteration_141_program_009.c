/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* External functions that will be called */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func2(int *p) {
    *p *= 2;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func3(int *p) {
    *p -= 3;
    asm volatile("" : : : "memory");
}

/* Function with complex register usage and calls */
int __attribute__((noinline)) compute_with_saves(int *data, int n, void (*callback)(int*)) {
    /* Use explicit register variables to pressure call-clobbered registers */
    register long r10 asm("r10") = data[0];
    register long r11 asm("r11") = data[1];
    register long r12 asm("r12") = data[2];  /* Call-saved on x86-64 */
    register long r13 asm("r13") = data[3];  /* Call-saved on x86-64 */
    register long rax_val asm("rax") = data[4];
    
    volatile int barrier = 0;
    
    /* Nested loops with calls to create complex live ranges */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 3; j++) {
            /* Mix of operations keeping values live in registers */
            r10 += data[i + j] * 2;
            r11 ^= data[i + j] + 1;
            
            /* Force values to be computed before call */
            asm volatile("" : "+r"(r10), "+r"(r11) : : "memory");
            
            /* Call external function - forces caller-save for call-clobbered regs */
            callback(&data[i]);
            
            /* Use the values after call - they need to be restored */
            r12 += r10;
            r13 += r11;
            
            /* More operations mixing call-clobbered and call-saved regs */
            rax_val = r12 * r13;
            data[i] += rax_val;
            
            /* Volatile to prevent reordering */
            barrier++;
        }
        
        /* Function pointer call with different targets */
        void (*fp)(int*) = (i % 2) ? external_func2 : external_func3;
        fp(&data[i]);
        
        /* Keep register values live across the call */
        r10 += barrier;
        r11 ^= barrier;
        
        /* Inline assembly to clobber specific registers */
        asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    }
    
    /* Final computation using all register values */
    int result = (r10 + r11 + r12 + r13 + rax_val) % 1000;
    
    /* Store results back */
    data[0] = r10;
    data[1] = r11;
    data[2] = r12;
    data[3] = r13;
    
    return result;
}

/* Another function with indirect calls */
int __attribute__((noinline)) compute_with_indirect(int *data, int n) {
    int sum = 0;
    
    /* Array of function pointers */
    void (*funcs[3])(int*) = {external_func1, external_func2, external_func3};
    
    for (int i = 0; i < n; i++) {
        /* Multiple live values in registers */
        register int a asm("r10") = data[i];
        register int b asm("r11") = data[i + 1];
        register int c asm("r12") = data[i + 2];  /* Call-saved */
        
        /* Complex computation with intermediate results */
        for (int j = 0; j < 5; j++) {
            a = a * 3 + j;
            b = b ^ (a << 2);
            c = c + b * 7;
            
            /* Call via function pointer - inhibits optimizations */
            funcs[j % 3](&data[i + j % 4]);
            
            /* Values must survive across call */
            sum += a + b + c;
            
            /* Force spill/reload behavior */
            asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : : "memory");
        }
        
        data[i] = a;
        data[i + 1] = b;
        data[i + 2] = c;
    }
    
    return sum;
}

int main() {
    /* Initialize data array */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = (i * 37 + 123) % 1000;
    }
    
    printf("Initial data[0..5]: %d %d %d %d %d %d\n", 
           data[0], data[1], data[2], data[3], data[4], data[5]);
    
    /* First computation with direct calls */
    int result1 = compute_with_saves(data, 50, external_func1);
    printf("Result 1: %d, data[0..2]: %d %d %d\n", 
           result1, data[0], data[1], data[2]);
    
    /* Reset some data */
    for (int i = 100; i < 150; i++) {
        data[i] = (i * 19 + 456) % 1000;
    }
    
    /* Second computation with indirect calls */
    int result2 = compute_with_indirect(data + 100, 30);
    printf("Result 2: %d, data[100..102]: %d %d %d\n", 
           result2, data[100], data[101], data[102]);
    
    /* Final check to ensure computation was meaningful */
    int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += data[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
