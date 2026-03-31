/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func3(unsigned long long *p) {
    *p ^= 0xAAAAAAAAAAAAAAAAULL;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_seed = 123456789;

/* Main test function with complex register usage */
long __attribute__((noinline)) test_caller_save_insertion(int *data, int size) {
    /* Explicit register variables targeting call-clobbered registers */
    register long r10_val asm("r10") = global_seed;
    register long r11_val asm("r11") = global_seed * 2;
    register int rcx_val asm("ecx") = size;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    
    /* Mix of call-saved and call-clobbered register usage */
    long rbx_val = 0;  /* Call-saved register */
    long r12_val = 0;  /* Call-saved register */
    long r13_val = 0;  /* Call-saved register */
    
    /* Array of function pointers for indirect calls */
    func_ptr_t funcs[3] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Nested loops with calls to create complex live ranges */
    for (int i = 0; i < size; i++) {
        /* Load values into registers - creating live ranges */
        int val = data[i];
        r10_val ^= val;
        r11_val += val;
        
        /* Complex arithmetic keeping values live in registers */
        for (int j = 0; j < 3; j++) {
            /* More register-intensive calculations */
            rcx_val = (rcx_val * 1103515245 + 12345) & 0x7fffffff;
            r8_val = r10_val * r11_val + rcx_val;
            r9_val = r8_val ^ r10_val ^ r11_val;
            
            /* Call-saved registers also used */
            rbx_val += r8_val;
            r12_val ^= r9_val;
            r13_val = r13_val * 3 + rcx_val;
            
            /* Inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(rcx_val));
            
            /* Indirect function call - inhibits optimizations */
            int idx = (rcx_val >> 16) % 3;
            
            /* Save values that must survive the call */
            long pre_call_r10 = r10_val;
            long pre_call_r11 = r11_val;
            int pre_call_rcx = rcx_val;
            
            /* Make the call with different argument types */
            if (idx == 0) {
                external_func1(&data[i]);
            } else if (idx == 1) {
                external_func2(&r8_val);
            } else {
                external_func3(&r9_val);
            }
            
            /* Restore and continue using the values */
            r10_val = pre_call_r10 + 1;
            r11_val = pre_call_r11 - 1;
            rcx_val = pre_call_rcx ^ 0x5555;
            
            /* More calculations after the call */
            r8_val = r8_val + rbx_val + r12_val;
            r9_val = r9_val * 2 + r13_val;
            
            /* Another inline assembly to prevent reordering */
            asm volatile("" : : "r"(r8_val), "r"(r9_val));
        }
        
        /* Store results back, creating register pressure */
        data[i] = (r10_val ^ r11_val ^ rcx_val ^ r8_val ^ r9_val) & 0x7fffffff;
        
        /* Update call-saved registers */
        rbx_val = (rbx_val + r12_val) >> 1;
        r12_val ^= r13_val;
        r13_val += data[i];
    }
    
    /* Final computation using all registers */
    long result = r10_val + r11_val + rcx_val + r8_val + r9_val + 
                  rbx_val + r12_val + r13_val;
    
    /* Ensure all values are used */
    asm volatile("" : : "r"(result));
    
    return result;
}

/* Another test function with different pattern */
long __attribute__((noinline)) test_pattern2(int *data, int size) {
    register long rax_val asm("rax") = 1;
    register long rdx_val asm("rdx") = 1;
    register int esi_val asm("esi") = size;
    
    long r14_val = 0;
    long r15_val = 0;
    
    for (int i = 0; i < size; i += 2) {
        /* Interleaved loads and calculations */
        int val1 = data[i];
        int val2 = data[i + 1];
        
        rax_val = rax_val * val1 + 1;
        rdx_val = rdx_val * val2 + 1;
        
        /* Multiple calls in sequence */
        external_func1(&data[i]);
        
        /* Values must be saved across this call */
        long saved_rax = rax_val;
        long saved_rdx = rdx_val;
        
        external_func2(&data[i + 1]);
        
        rax_val = saved_rax ^ 0x12345678;
        rdx_val = saved_rdx ^ 0x87654321;
        
        /* Another call */
        external_func3((unsigned long long*)&rax_val);
        
        /* Complex live range spanning basic blocks */
        for (int j = 0; j < 2; j++) {
            esi_val = (esi_val * 1664525 + 1013904223) & 0xffffffff;
            r14_val += esi_val;
            r15_val ^= esi_val;
            
            /* Conditional call */
            if (esi_val & 1) {
                external_func1(&global_counter);
            }
        }
        
        data[i] = rax_val & 0x7fffffff;
        data[i + 1] = rdx_val & 0x7fffffff;
    }
    
    return rax_val + rdx_val + esi_val + r14_val + r15_val;
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Run multiple test patterns to increase coverage */
    long result1 = test_caller_save_insertion(data, SIZE);
    
    /* Re-initialize for second test */
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    long result2 = test_pattern2(data, SIZE);
    
    /* Final computation using results */
    long final_result = result1 ^ result2;
    
    /* Use the results to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        final_result += data[i];
    }
    
    printf("Final result: %ld\n", final_result);
    
    free(data);
    return 0;
}
