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

/* Force noinline to prevent optimization */
__attribute__((noinline)) void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers */
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

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Main test function with register-intensive operations */
int test_caller_save(int* data, int size) {
    volatile int guard = 0;
    int result = 0;
    
    /* Use explicit register variables to pressure specific registers */
#ifdef __x86_64__
    register int64_t r10_val asm("r10") = 0;
    register int64_t r11_val asm("r11") = 0;
    register int64_t r9_val asm("r9") = 0;
    register int64_t r8_val asm("r8") = 0;
#else
    register int32_t eax_val asm("eax") = 0;
    register int32_t ecx_val asm("ecx") = 0;
    register int32_t edx_val asm("edx") = 0;
#endif
    
    /* Array of function pointers for indirect calls */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    int func_count = sizeof(funcs)/sizeof(funcs[0]);
    
    /* Nested loops to create complex live ranges */
    for (int outer = 0; outer < 3; ++outer) {
        /* Load values into register variables */
#ifdef __x86_64__
        r10_val = data[outer * 10] + 1;
        r11_val = data[outer * 10 + 1] * 2;
        r9_val = data[outer * 10 + 2] - 3;
        r8_val = data[outer * 10 + 3] ^ 0xFF;
#else
        eax_val = data[outer * 10] + 1;
        ecx_val = data[outer * 10 + 1] * 2;
        edx_val = data[outer * 10 + 2] - 3;
#endif
        
        for (int inner = 0; inner < 100; ++inner) {
            /* Perform arithmetic keeping values live in registers */
#ifdef __x86_64__
            r10_val = (r10_val * 1103515245 + 12345) & 0x7FFFFFFF;
            r11_val = (r11_val * 1103515245 + 12345) & 0x7FFFFFFF;
            r9_val = (r9_val * 1103515245 + 12345) & 0x7FFFFFFF;
            r8_val = (r8_val * 1103515245 + 12345) & 0x7FFFFFFF;
            
            /* Mix with volatile to prevent optimization */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r9_val), "+r"(r8_val));
            
            /* Call external function - values in r10, r11, r9, r8 must be saved */
            funcs[inner % func_count]((int*)&guard);
            
            /* Continue using register values after call */
            r10_val ^= r11_val;
            r9_val += r8_val;
            result += (r10_val & 1) + (r9_val & 1);
#else
            eax_val = (eax_val * 1103515245 + 12345) & 0x7FFFFFFF;
            ecx_val = (ecx_val * 1103515245 + 12345) & 0x7FFFFFFF;
            edx_val = (edx_val * 1103515245 + 12345) & 0x7FFFFFFF;
            
            asm volatile("" : "+r"(eax_val), "+r"(ecx_val), "+r"(edx_val));
            
            funcs[inner % func_count]((int*)&guard);
            
            eax_val ^= ecx_val;
            result += (eax_val & 1) + (edx_val & 1);
#endif
            
            /* Store back to memory periodically */
            if (inner % 25 == 0) {
#ifdef __x86_64__
                data[outer * 10] = r10_val;
                data[outer * 10 + 1] = r11_val;
                data[outer * 10 + 2] = r9_val;
                data[outer * 10 + 3] = r8_val;
#else
                data[outer * 10] = eax_val;
                data[outer * 10 + 1] = ecx_val;
                data[outer * 10 + 2] = edx_val;
#endif
            }
        }
        
        /* Final store */
#ifdef __x86_64__
        data[outer * 10 + 4] = r10_val + r11_val + r9_val + r8_val;
#else
        data[outer * 10 + 4] = eax_val + ecx_val + edx_val;
#endif
    }
    
    return result;
}

/* Another test with different pattern */
int test_caller_save2(int* data, int size) {
    int sum = 0;
    
    /* Use mix of automatic and register variables */
    register int reg1 asm("r10") = 0;
    register int reg2 asm("r11") = 0;
    int stack_var1 = 0, stack_var2 = 0;
    
    for (int i = 0; i < size; i += 4) {
        /* Load and compute in registers */
        reg1 = data[i] * 3;
        reg2 = data[i + 1] * 5;
        stack_var1 = data[i + 2] * 7;
        stack_var2 = data[i + 3] * 11;
        
        /* Force values to be live across call */
        asm volatile("" : "+r"(reg1), "+r"(reg2), "+g"(stack_var1), "+g"(stack_var2));
        
        /* Call with volatile argument */
        volatile int arg = i;
        external_func1(&arg);
        
        /* Complex computation using all values */
        reg1 = (reg1 ^ reg2) + stack_var1;
        reg2 = (reg2 * stack_var2) - reg1;
        
        /* Another call */
        external_func2(&arg);
        
        /* More computation */
        sum += reg1 + reg2 + stack_var1 + stack_var2;
        
        /* Store results back */
        data[i] = reg1;
        data[i + 1] = reg2;
    }
    
    return sum;
}

int main() {
    const int DATA_SIZE = 256;
    int* data = (int*)malloc(DATA_SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < DATA_SIZE; ++i) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Run tests */
    int result1 = test_caller_save(data, DATA_SIZE);
    int result2 = test_caller_save2(data, DATA_SIZE);
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < DATA_SIZE; ++i) {
        checksum ^= data[i];
    }
    
    printf("Result1: %d, Result2: %d, Checksum: 0x%08x\n", 
           result1, result2, checksum);
    
    free(data);
    return 0;
}
