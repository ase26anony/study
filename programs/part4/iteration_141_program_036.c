/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func3(unsigned long *p) {
    *p ^= 0xAAAAAAAA;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Main test function with intensive register usage */
long __attribute__((noinline)) test_function(int *data, int size, func_ptr_t fp) {
    /* Use explicit register variables to pressure specific call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    
    /* Volatile variables to prevent optimization */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile long v3 = 3;
    
    /* Complex loop with multiple live values across calls */
    for (int i = 0; i < size; i++) {
        /* Load values into call-clobbered registers */
        r10_val = data[i] + v1;
        r11_val = data[i + 1] * v2;
        r8_val = r10_val ^ r11_val;
        r9_val = r8_val << 2;
        
        /* Create artificial dependencies with inline asm */
        asm volatile("" : "+r"(r10_val), "+r"(r11_val));
        
        /* Mix with call-saved register usage */
        long rbx_val = r10_val + r11_val;  /* rbx is call-saved */
        long r12_val = r8_val * r9_val;    /* r12 is call-saved */
        
        /* Call external function - forces caller-save for live values */
        fp(&data[i]);
        
        /* Use values after call - they must be restored */
        rcx_val = r10_val + rbx_val;
        rdx_val = r11_val + r12_val;
        
        /* Another artificial dependency */
        asm volatile("" : "+r"(rcx_val), "+r"(rdx_val));
        
        /* Store results back using mixed registers */
        data[i] = (int)(rcx_val ^ rdx_val);
        
        /* Nested loop to increase complexity */
        for (int j = 0; j < 3; j++) {
            /* More register pressure */
            r10_val += data[j] * v3;
            r11_val ^= data[j + 1];
            
            /* Another call in nested loop */
            if (j % 2 == 0) {
                external_func2(&r10_val);
            } else {
                external_func3((unsigned long*)&r11_val);
            }
            
            /* Keep values live across calls */
            r8_val = r10_val - r11_val;
            asm volatile("" : "+r"(r8_val));
        }
        
        /* Accumulate results in call-clobbered registers */
        rcx_val += r8_val;
        rdx_val += r9_val;
    }
    
    /* Final computation using all registers */
    long result = r10_val + r11_val + r8_val + r9_val + rcx_val + rdx_val;
    
    /* Prevent tail call optimization */
    asm volatile("" : : "r"(result));
    
    return result;
}

/* Another test with indirect calls */
long __attribute__((noinline)) test_indirect_calls(int *data, int size, int selector) {
    register long rax_val asm("rax") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    
    /* Array of function pointers */
    func_ptr_t funcs[3] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Volatile to prevent constant propagation */
    volatile int vol_idx = selector;
    
    for (int i = 0; i < size; i += 2) {
        /* Compute values in call-clobbered registers */
        rax_val = data[i] * 3;
        rcx_val = data[i + 1] * 7;
        rdx_val = rax_val ^ rcx_val;
        
        /* Artificial dependency chain */
        asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val));
        
        /* Indirect call - compiler can't analyze clobbers easily */
        funcs[vol_idx % 3]((void*)&data[i]);
        
        /* Use values after indirect call */
        rax_val += rdx_val;
        rcx_val -= rdx_val;
        
        /* Another call */
        external_func1(&data[i + 1]);
        
        /* More computations */
        rdx_val = rax_val * rcx_val;
        
        /* Store with volatile write */
        volatile int *vol_ptr = &data[i];
        *vol_ptr = (int)rdx_val;
    }
    
    return rax_val + rcx_val + rdx_val;
}

int main() {
    /* Initialize data array */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i * 3 + 7;
    }
    
    /* Test with direct function pointer */
    long result1 = test_function(data, 128, (func_ptr_t)external_func1);
    
    /* Test with indirect calls */
    long result2 = test_indirect_calls(data, 128, 1);
    
    /* Final computation to use results */
    long final_result = result1 ^ result2;
    
    /* Print to prevent optimization */
    printf("Result: %ld\n", final_result);
    
    /* Verify some data was modified */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
