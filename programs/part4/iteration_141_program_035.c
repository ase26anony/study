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
    
    /* Mix of call-clobbered and call-saved register usage */
    long call_saved1 = 0;  /* Should use rbx, rbp, r12-r15 */
    long call_saved2 = 0;
    long call_saved3 = 0;
    
    /* Nested loops with calls to create complex live ranges */
    for (int i = 0; i < size; i++) {
        /* Load values into call-clobbered registers */
        r10_val = data[i] * v1;
        r11_val = data[i + 1] * v2;
        r8_val = data[i + 2] * v3;
        
        /* Perform arithmetic keeping results live in call-clobbered regs */
        for (int j = 0; j < 3; j++) {
            /* More operations on call-clobbered registers */
            rcx_val = r10_val + r11_val;
            rdx_val = r8_val * rcx_val;
            
            /* Use inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(rcx_val), "+r"(rdx_val));
            
            /* Call external function - forces caller-save for live values */
            if (j == 0) {
                external_func1((int*)&rcx_val);
            } else if (j == 1) {
                external_func2(&rdx_val);
            } else {
                fp(&rdx_val);
            }
            
            /* Use values after call - they need to be restored */
            r9_val = rcx_val + rdx_val;
            call_saved1 += r9_val;
            
            /* More operations mixing register types */
            asm volatile("" : "+r"(r9_val), "+r"(call_saved1));
        }
        
        /* Store results back, creating more register pressure */
        data[i] = (int)(r10_val + call_saved1);
        call_saved2 += r11_val;
        call_saved3 += r8_val;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(call_saved2), "r"(call_saved3));
    }
    
    /* Final computation using all register types */
    long result = r10_val + r11_val + r8_val + r9_val + 
                  rcx_val + rdx_val + call_saved1 + call_saved2 + call_saved3;
    
    /* Force use of all values before return */
    asm volatile("" : : "r"(result), "r"(r10_val), "r"(r11_val), 
                 "r"(r8_val), "r"(r9_val), "r"(rcx_val), "r"(rdx_val));
    
    return result;
}

/* Another test with indirect calls */
long __attribute__((noinline)) test_indirect_calls(int *data, int size) {
    long sum = 0;
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Use multiple call-clobbered registers */
    register long acc1 asm("r10") = 0;
    register long acc2 asm("r11") = 0;
    register long acc3 asm("r8") = 0;
    
    for (int i = 0; i < size; i += 4) {
        /* Load and compute in call-clobbered registers */
        acc1 = data[i] * 3;
        acc2 = data[i + 1] * 5;
        acc3 = data[i + 2] * 7;
        
        /* Volatile to prevent reordering */
        volatile int selector = i % 3;
        
        /* Indirect call - compiler doesn't know which registers are clobbered */
        funcs[selector]((void*)&acc1);
        
        /* More operations keeping values live */
        acc2 = acc1 + acc2;
        acc3 = acc2 * acc3;
        
        /* Another call */
        funcs[(selector + 1) % 3]((void*)&acc2);
        
        /* Use all values */
        sum += acc1 + acc2 + acc3;
        
        /* Store back using different pattern */
        data[i] = (int)acc1;
        data[i + 1] = (int)acc2;
        data[i + 2] = (int)acc3;
    }
    
    return sum;
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i + 1;
    }
    
    /* Test 1: Direct calls with register pressure */
    long result1 = test_function(data, SIZE - 10, (func_ptr_t)external_func3);
    printf("Result 1: %ld\n", result1);
    
    /* Re-initialize */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 3) % 100;
    }
    
    /* Test 2: Indirect calls */
    long result2 = test_indirect_calls(data, SIZE - 4);
    printf("Result 2: %ld\n", result2);
    
    /* Verify some results */
    int check_sum = 0;
    for (int i = 0; i < 10; i++) {
        check_sum += data[i];
    }
    printf("Check sum: %d\n", check_sum);
    
    free(data);
    return 0;
}
