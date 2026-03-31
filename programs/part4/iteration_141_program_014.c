/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that clobber registers */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define REGISTER_CLOBBER __attribute__((no_caller_saved_registers))
#else
#define NOINLINE
#define REGISTER_CLOBBER
#endif

/* Function that will be called - declared to clobber registers */
NOINLINE REGISTER_CLOBBER
void external_func1(volatile int* ptr) {
    *ptr += 1;
    /* Inline asm to clobber specific registers */
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

NOINLINE REGISTER_CLOBBER  
void external_func2(volatile int* ptr) {
    *ptr *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Another external function with different signature */
NOINLINE REGISTER_CLOBBER
int external_func3(int a, int b) {
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return a + b;
}

/* Global volatile to prevent optimizations */
volatile int global_counter = 0;

/* Main test function with complex register usage */
int test_function(int* data, int size) {
    /* Explicit register variables for call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    
    /* Regular variables that will use call-saved registers */
    long saved_reg1 = 0, saved_reg2 = 0, saved_reg3 = 0;
    
    /* Function pointer to force indirect call */
    void (*fp)(volatile int*) = NULL;
    
    int sum = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 3; ++outer) {
        /* Initialize function pointer based on condition */
        if (outer % 2 == 0) {
            fp = external_func1;
        } else {
            fp = external_func2;
        }
        
        /* Inner loop with intensive register usage */
        for (int i = 0; i < size; ++i) {
            /* Load values into call-clobbered registers */
            r10_val = data[i] * 3;
            r11_val = data[i] + 7;
            r8_val = data[size - i - 1] * 2;
            r9_val = r10_val ^ r11_val;
            
            /* Use call-saved registers for computations */
            saved_reg1 = r10_val * r11_val;
            saved_reg2 = r8_val | r9_val;
            saved_reg3 = saved_reg1 + saved_reg2;
            
            /* Volatile inline assembly to create dependencies */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r8_val), "+r"(r9_val));
            
            /* Call external function - forces caller-save for live values */
            fp(&global_counter);
            
            /* More computations after call - values must be restored */
            saved_reg1 = r10_val + saved_reg3;
            saved_reg2 = r11_val * saved_reg1;
            saved_reg3 = r8_val - r9_val;
            
            /* Another call with different arguments */
            int tmp = external_func3(saved_reg1, saved_reg2);
            
            /* Use all values in final computation */
            data[i] = (saved_reg3 + tmp) ^ (r10_val * r11_val);
            
            /* Update sum with complex expression */
            sum += data[i] + saved_reg1 - saved_reg2 + saved_reg3;
            
            /* More inline asm to prevent reordering */
            asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(r8_val), "r"(r9_val));
        }
        
        /* Nested loop with different pattern */
        for (int j = 0; j < size / 2; ++j) {
            /* Swap function pointer */
            fp = (fp == external_func1) ? external_func2 : external_func1;
            
            /* Different register usage pattern */
            r10_val = data[j] << 2;
            r11_val = data[size - j - 1] >> 1;
            
            /* Complex chain of operations */
            for (int k = 0; k < 4; ++k) {
                r8_val = r10_val + k;
                r9_val = r11_val - k;
                
                /* Call within innermost loop */
                fp(&global_counter);
                
                /* Update values */
                r10_val = r8_val ^ r9_val;
                r11_val = r10_val * (k + 1);
                
                /* Another external call */
                int tmp2 = external_func3(r10_val, r11_val);
                
                data[j] += tmp2;
                sum ^= data[j];
            }
        }
    }
    
    return sum;
}

/* Another test with different register pressure pattern */
int test_function2(int* data, int size) {
    register long rax_val asm("rax") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    
    int result = 0;
    
    /* Pattern that creates many live values across calls */
    for (int i = 0; i < size; i += 4) {
        /* Load multiple values into registers */
        rax_val = data[i];
        rcx_val = data[i + 1];
        rdx_val = data[i + 2];
        
        /* Create dependencies between them */
        for (int j = 0; j < 3; ++j) {
            rax_val = rax_val * rcx_val + j;
            rcx_val = rdx_val ^ rax_val;
            rdx_val = rax_val - rcx_val;
            
            /* Call between dependent operations */
            external_func1(&global_counter);
            
            /* Continue computations */
            rax_val = rcx_val | rdx_val;
            rcx_val = rax_val << 2;
            rdx_val = rcx_val >> 1;
            
            external_func2(&global_counter);
        }
        
        /* Store results back */
        data[i] = rax_val;
        data[i + 1] = rcx_val;
        data[i + 2] = rdx_val;
        
        result += rax_val + rcx_val + rdx_val;
    }
    
    return result;
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
        data[i] = i * 3 + 7;
    }
    
    /* Run first test */
    int result1 = test_function(data, SIZE);
    printf("Result 1: %d\n", result1);
    
    /* Re-initialize data */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (i * 5) ^ 0x1234;
    }
    
    /* Run second test */
    int result2 = test_function2(data, SIZE);
    printf("Result 2: %d\n", result2);
    
    /* Final computation mixing both results */
    int final_result = result1 ^ result2;
    printf("Final result: %d\n", final_result);
    
    free(data);
    return 0;
}
