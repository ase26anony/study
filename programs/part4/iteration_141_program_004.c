/* Compile with: g++ -O2 -fno-omit-frame-pointer -fno-inline -fno-strict-aliasing caller-save-test.cc -o caller-save-test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_REGISTER(type, name, reg) register type name asm(reg)

/* External functions that will clobber registers */
NOINLINE void external_func1(int* p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

NOINLINE void external_func2(int* p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

NOINLINE void external_func3(int* p) {
    *p -= 3;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function that creates register pressure and live values across calls */
NOINLINE int process_data(int* data, int size, void (*func)(int*)) {
    /* Use explicit register variables for call-clobbered registers */
    VOLATILE_REGISTER(long, r10_val, "r10") = 0;
    VOLATILE_REGISTER(long, r11_val, "r11") = 0;
    VOLATILE_REGISTER(long, r9_val, "r9") = 0;
    VOLATILE_REGISTER(long, r8_val, "r8") = 0;
    
    /* Use call-saved registers too */
    register long r12_val asm("r12") = 0;
    register long r13_val asm("r13") = 0;
    register long r14_val asm("r14") = 0;
    register long r15_val asm("r15") = 0;
    
    volatile int temp = 0;
    
    /* Complex loop with multiple live values across calls */
    for (int i = 0; i < size; i++) {
        /* Load values into call-clobbered registers */
        r10_val = data[i] * 3;
        r11_val = data[i] + 7;
        r9_val = data[i] - 2;
        r8_val = data[i] * data[i];
        
        /* Keep values in call-saved registers live */
        r12_val += r10_val;
        r13_val += r11_val;
        r14_val += r9_val;
        r15_val += r8_val;
        
        /* Artificial dependency to prevent optimization */
        asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r9_val), "+r"(r8_val));
        
        /* Call external function - values in r10, r11, r9, r8 must be saved */
        func(&temp);
        
        /* Use the values after call - they need to be restored */
        data[i] = (r10_val + r11_val + r9_val + r8_val) % 1000;
        
        /* More operations to create additional live ranges */
        r10_val = (r10_val * 2) % 997;
        r11_val = (r11_val + 3) % 991;
        r9_val = (r9_val - 1) % 983;
        r8_val = (r8_val / 2) % 977;
        
        /* Nested loop to create complex control flow */
        for (int j = 0; j < 3; j++) {
            /* Another call inside nested loop */
            if (j % 2 == 0) {
                external_func1(&temp);
            } else {
                external_func2(&temp);
            }
            
            /* More register operations */
            r12_val += r10_val * j;
            r13_val += r11_val * j;
            asm volatile("" : "+r"(r12_val), "+r"(r13_val));
        }
    }
    
    /* Final computation using all registers */
    int result = (r12_val + r13_val + r14_val + r15_val) % 1000000;
    result += temp;
    
    return result;
}

/* Another function with different pattern */
NOINLINE int alternate_process(int* data, int size) {
    VOLATILE_REGISTER(long, rax_val, "rax") = 0;
    VOLATILE_REGISTER(long, rcx_val, "rcx") = 0;
    VOLATILE_REGISTER(long, rdx_val, "rdx") = 0;
    VOLATILE_REGISTER(long, rsi_val, "rsi") = 0;
    VOLATILE_REGISTER(long, rdi_val, "rdi") = 0;
    
    volatile int counter = 0;
    
    /* Function pointer array for indirect calls */
    void (*funcs[3])(int*) = {external_func1, external_func2, external_func3};
    
    for (int i = 0; i < size; i += 2) {
        /* Compute multiple values in call-clobbered registers */
        rax_val = data[i] * 5;
        rcx_val = data[i + 1] * 7;
        rdx_val = rax_val + rcx_val;
        rsi_val = rax_val - rcx_val;
        rdi_val = rax_val * rcx_val;
        
        /* Force values to be live across indirect call */
        asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val), 
                          "+r"(rsi_val), "+r"(rdi_val));
        
        /* Indirect call - compiler doesn't know which registers are clobbered */
        funcs[i % 3](&counter);
        
        /* Use values after call */
        data[i] = (rax_val + rcx_val + rdx_val) % 1000;
        data[i + 1] = (rsi_val + rdi_val) % 1000;
        
        /* More operations creating register pressure */
        for (int k = 0; k < 4; k++) {
            rax_val = (rax_val * 3 + k) % 997;
            rcx_val = (rcx_val * 5 + k) % 991;
            
            /* Another call inside inner loop */
            external_func3(&counter);
            
            rdx_val += rax_val * k;
            rsi_val += rcx_val * k;
        }
    }
    
    return counter;
}

int main() {
    const int SIZE = 256;
    int data[SIZE];
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 100;
    }
    
    /* Process data with different function pointers */
    void (*func_ptr)(int*) = external_func1;
    int result1 = process_data(data, SIZE, func_ptr);
    
    func_ptr = external_func2;
    int result2 = process_data(data + 128, SIZE / 2, func_ptr);
    
    int result3 = alternate_process(data, SIZE);
    
    /* Final computation */
    int final_result = (result1 + result2 + result3) % 1000000;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Verify some data was processed */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum % 10000);
    
    return 0;
}
