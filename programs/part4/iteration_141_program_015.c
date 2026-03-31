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
    /* Use explicit register variables to pressure call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    
    /* Volatile variables to prevent optimization */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    
    /* Main computation loop with nested loops */
    for (int outer = 0; outer < 3; ++outer) {
        /* Load values into call-clobbered registers */
        r10_val = data[outer * 4 + 0];
        r11_val = data[outer * 4 + 1];
        r8_val = data[outer * 4 + 2];
        r9_val = data[outer * 4 + 3];
        
        /* Inner loop with arithmetic operations */
        for (int inner = 0; inner < 100; ++inner) {
            /* Complex arithmetic to keep values live in registers */
            rcx_val = r10_val * r11_val + inner;
            rdx_val = r8_val ^ r9_val - inner;
            
            /* Mix with volatile to prevent reordering */
            rcx_val += v1;
            rdx_val += v2;
            
            /* Inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(rcx_val), "+r"(rdx_val));
            
            /* Function call that clobbers registers */
            if (inner % 3 == 0) {
                fp(&r10_val);
            } else if (inner % 3 == 1) {
                external_func2(&r11_val);
            } else {
                external_func3((unsigned long*)&r8_val);
            }
            
            /* More arithmetic after call - values must be restored */
            rcx_val = rcx_val * rdx_val + r10_val;
            rdx_val = rdx_val ^ rcx_val + r11_val;
            
            /* Use results */
            data[outer * 4 + 0] = (int)(r10_val + rcx_val);
            data[outer * 4 + 1] = (int)(r11_val + rdx_val);
            
            /* Update volatile to prevent loop elimination */
            v3 = inner;
        }
        
        /* Cross-iteration dependencies */
        r10_val += r11_val + r8_val + r9_val;
        asm volatile("" : "+r"(r10_val));
    }
    
    /* Final computation using all register values */
    long result = r10_val + r11_val + r8_val + r9_val + rcx_val + rdx_val;
    
    /* Prevent tail-call optimization */
    asm volatile("" : : "r"(result));
    return result;
}

/* Another test with indirect calls */
long __attribute__((noinline)) test_indirect_calls(int *data, int size) {
    register long rax_val asm("rax") = 0;
    register long rbx_val asm("rbx") = 0;  /* Call-saved */
    register long r12_val asm("r12") = 0;  /* Call-saved */
    register long r13_val asm("r13") = 0;  /* Call-saved */
    
    /* Array of function pointers */
    func_ptr_t funcs[3] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Initialize call-saved registers */
    rbx_val = 0x12345678;
    r12_val = 0x9ABCDEF0;
    r13_val = 0x55555555;
    
    for (int i = 0; i < size; ++i) {
        /* Load into call-clobbered register */
        rax_val = data[i];
        
        /* Complex computation */
        for (int j = 0; j < 10; ++j) {
            rax_val = rax_val * 1103515245 + 12345;
            
            /* Mix with call-saved registers */
            rax_val ^= rbx_val;
            rax_val += r12_val;
            rax_val &= r13_val;
            
            /* Indirect call - compiler doesn't know which registers it clobbers */
            funcs[i % 3]((void*)&rax_val);
            
            /* More computation after call */
            rax_val = (rax_val << 5) | (rax_val >> 59);
            rax_val += rbx_val * r12_val;
            
            /* Store intermediate result */
            data[i] = (int)rax_val;
        }
        
        /* Update call-saved registers */
        rbx_val ^= rax_val;
        r12_val += rax_val;
        r13_val &= rax_val;
    }
    
    return rax_val + rbx_val + r12_val + r13_val;
}

int main() {
    /* Initialize test data */
    int data[256];
    for (int i = 0; i < 256; ++i) {
        data[i] = i * 3 + 7;
    }
    
    long result1 = 0, result2 = 0;
    
    /* Test with direct function pointer */
    result1 = test_function(data, 256, (func_ptr_t)external_func1);
    
    /* Test with indirect calls */
    int data2[128];
    for (int i = 0; i < 128; ++i) {
        data2[i] = i * 5 + 11;
    }
    result2 = test_indirect_calls(data2, 128);
    
    /* Use results to prevent elimination */
    printf("Result1: %ld\n", result1);
    printf("Result2: %ld\n", result2);
    
    /* Final check */
    long final_sum = 0;
    for (int i = 0; i < 256; ++i) final_sum += data[i];
    for (int i = 0; i < 128; ++i) final_sum += data2[i];
    
    printf("Final sum: %ld\n", final_sum);
    
    return (final_sum > 0) ? 0 : 1;
}
