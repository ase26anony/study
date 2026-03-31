/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))
#else
#define NOINLINE
#define USED
#endif

/* Function that clobbers many registers */
NOINLINE USED void external_func1(volatile int* p) {
    /* Inline assembly to clobber specific registers */
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    *p += 1;
}

NOINLINE USED void external_func2(volatile int* p) {
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    *p *= 2;
}

NOINLINE USED void external_func3(volatile int* p) {
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    *p -= 3;
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Main test function with complex register usage */
int test_caller_save_insertion(int* data, int size) {
    volatile int result = 0;
    
    /* Array of function pointers to force indirect calls */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    int func_index = 0;
    
    /* Use explicit register variables to create pressure on call-clobbered regs */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    
    /* Also use call-saved registers */
    register long r12_val asm("r12") = 0;
    register long r13_val asm("r13") = 0;
    register long r14_val asm("r14") = 0;
    register long r15_val asm("r15") = 0;
    
    /* Outer loop to create multiple basic blocks */
    for (int outer = 0; outer < 3; ++outer) {
        /* Inner loop with function calls and register-intensive calculations */
        for (int i = 0; i < size; ++i) {
            /* Load values into call-clobbered registers */
            r10_val = data[i] + outer;
            r11_val = data[(i + 1) % size] * 2;
            r8_val = data[(i + 2) % size] - 3;
            r9_val = data[(i + 3) % size] ^ 0xFF;
            
            /* Keep values in call-saved registers live across calls */
            r12_val += r10_val;
            r13_val += r11_val;
            r14_val += r8_val;
            r15_val += r9_val;
            
            /* Create artificial dependencies with inline assembly */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r8_val), "+r"(r9_val));
            
            /* Call external function - forces caller-save for live call-clobbered regs */
            funcs[func_index](&result);
            
            /* Use the values after the call - they must be restored */
            int temp = (r10_val + r11_val) * (r8_val - r9_val);
            
            /* More calculations keeping call-saved registers live */
            r12_val += temp;
            r13_val -= temp;
            
            /* Another call with different function */
            func_index = (func_index + 1) % 3;
            funcs[func_index](&result);
            
            /* More register-intensive operations */
            r14_val ^= temp;
            r15_val |= temp;
            
            /* Store result back using call-clobbered registers */
            r10_val = r12_val + r13_val;
            r11_val = r14_val * r15_val;
            data[i] = (r10_val ^ r11_val) & 0xFFFF;
            
            /* Prevent loop unrolling */
            asm volatile("" : : : "memory");
        }
        
        /* Switch function pointer pattern */
        func_index = (func_index + outer) % 3;
    }
    
    /* Final computation using all registers */
    long final_result = r12_val + r13_val + r14_val + r15_val;
    return (final_result + result) & 0x7FFFFFFF;
}

/* Another test function with different pattern */
int test_nested_calls(int* data, int size) {
    volatile int accumulator = 0;
    
    for (int i = 0; i < size; i += 4) {
        /* Multiple live values in registers */
        int a = data[i];
        int b = data[i + 1];
        int c = data[i + 2];
        int d = data[i + 3];
        
        /* Chain of calculations with intervening calls */
        for (int j = 0; j < 4; ++j) {
            /* Use explicit register variables */
            register int x asm("r10") = a + j;
            register int y asm("r11") = b - j;
            register int z asm("r8") = c * j;
            register int w asm("r9") = d ^ j;
            
            /* Force these to be live across call */
            asm volatile("" : "+r"(x), "+r"(y), "+r"(z), "+r"(w));
            
            /* Call that clobbers registers */
            if (j % 2 == 0) {
                external_func1(&accumulator);
            } else {
                external_func2(&accumulator);
            }
            
            /* Use values after call */
            a = x + y;
            b = z - w;
            c = x * z;
            d = y ^ w;
            
            /* Another call */
            external_func3(&accumulator);
        }
        
        /* Store results back */
        data[i] = a;
        data[i + 1] = b;
        data[i + 2] = c;
        data[i + 3] = d;
    }
    
    return accumulator;
}

int main() {
    const int SIZE = 256;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with pattern */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i * 3 + 1;
    }
    
    /* Run first test */
    int result1 = test_caller_save_insertion(data, SIZE);
    
    /* Re-initialize for second test */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i * 5 - 2;
    }
    
    /* Run second test */
    int result2 = test_nested_calls(data, SIZE);
    
    /* Final computation */
    int final_result = result1 + result2;
    
    /* Verify by computing a checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; ++i) {
        checksum ^= data[i];
    }
    
    printf("Result1: %d\n", result1);
    printf("Result2: %d\n", result2);
    printf("Final: %d\n", final_result);
    printf("Checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
