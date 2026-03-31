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
    
    /* Mix operations to keep values live */
    r1 = r1 * r2 + r3;
    r2 = r2 ^ r4 | r1;
    r3 = r3 + r1 - r2;
    r4 = r4 * 7 + r3;
    
    /* Call external function with many live values */
    external_func1(&r1);
    
    /* Continue using the values */
    r2 = r2 + r1;
    r3 = r3 * r4;
    external_func2(&r2);
    
    r4 = r4 ^ r3;
    r1 = r1 | r2;
    external_func3(&r3);
    
    /* Final combination */
    return r1 + r2 + r3 + r4 + e + f + g + h;
}

/* Function using explicit register variables */
#ifdef __x86_64__
__attribute__((noinline)) int register_pressure_test(int *arr, int n) {
    /* Explicit register variables for call-clobbered registers */
    register long r10 asm("r10") = arr[0];
    register long r11 asm("r11") = arr[1];
    register long r8 asm("r8") = arr[2];
    register long r9 asm("r9") = arr[3];
    register long rcx asm("rcx") = arr[4];
    register long rdx asm("rdx") = arr[5];
    
    int sum = 0;
    
    /* Nested loops with function calls */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            /* Keep values in registers across calls */
            r10 = r10 + arr[i] + j;
            r11 = r11 ^ arr[i] ^ j;
            
            /* Function pointer to create indirect call */
            void (*fp)(int*) = (j & 1) ? external_func1 : external_func2;
            fp(&arr[i]);
            
            r8 = r8 * 3 + r10;
            r9 = r9 - 2 + r11;
            
            /* Another call with different function */
            fp = (j & 2) ? external_func2 : external_func3;
            fp(&arr[i + 1]);
            
            rcx = rcx | r8;
            rdx = rdx & r9;
            
            /* Volatile to prevent reordering */
            volatile int temp = r10 + r11;
            sum += temp;
        }
        
        /* Mix in another external call */
        external_func3(&sum);
        
        /* More register operations */
        r10 = r10 ^ rcx;
        r11 = r11 | rdx;
        r8 = r8 + r10;
        r9 = r9 - r11;
    }
    
    /* Final result using all register values */
    return sum + r10 + r11 + r8 + r9 + rcx + rdx;
}
#endif

/* Test with indirect calls and complex control flow */
__attribute__((noinline)) int indirect_call_test(int *data, int size) {
    typedef void (*func_ptr_t)(int*);
    func_ptr_t funcs[3] = {external_func1, external_func2, external_func3};
    
    int result = 0;
    volatile int keep_alive = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple live values in registers */
        int a = data[i];
        int b = data[i + 1];
        int c = data[i + 2];
        int d = data[i + 3];
        
        /* Force register usage before call */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
        
        /* Indirect call with many live values */
        funcs[i % 3](&data[i]);
        
        /* Use values after call */
        a = a * b + c;
        b = b ^ d;
        c = c + a - b;
        d = d * 7;
        
        /* Another call */
        funcs[(i + 1) % 3](&data[i + 1]);
        
        /* More operations */
        a = a | b;
        b = b & c;
        c = c ^ d;
        
        /* Final call */
        funcs[(i + 2) % 3](&data[i + 2]);
        
        result += a + b + c + d;
        keep_alive = result;  /* Volatile write */
    }
    
    return result;
}

int main() {
    const int SIZE = 256;
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i + 1;
    }
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test 1: Complex calculation with many parameters */
    result1 = complex_calculation(1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Test 2: Register pressure test (x86-64 specific) */
#ifdef __x86_64__
    result2 = register_pressure_test(data, 50);
#endif
    
    /* Test 3: Indirect calls */
    result3 = indirect_call_test(data, 100);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    /* Use the result */
    for (int i = 0; i < 10; i++) {
        final_result ^= data[i];
    }
    
    printf("Result: %d\n", final_result);
    
    free(data);
    return 0;
}
