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
    long call_saved1 = 0;  /* Should go to rbx/r12/r13/r14/r15 */
    long call_saved2 = 0;
    long call_saved3 = 0;
    
    /* Nested loops with calls to create complex live ranges */
    for (int i = 0; i < size; i++) {
        /* Load values into call-clobbered registers */
        r10_val = data[i] * v1;
        r11_val = data[i + 1] * v2;
        r8_val = data[i + 2] * v3;
        
        /* Complex arithmetic keeping values live */
        for (int j = 0; j < 3; j++) {
            /* Use inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r8_val));
            
            /* Mix with call-saved registers */
            call_saved1 = r10_val + r11_val;
            call_saved2 = r8_val * call_saved1;
            call_saved3 = call_saved2 ^ r10_val;
            
            /* Call external function - forces caller-save for live values */
            if (j == 1) {
                fp(&call_saved3);
            }
            
            /* More operations after call - values must be restored */
            r9_val = call_saved3 + r11_val;
            rcx_val = r9_val * r10_val;
            rdx_val = rcx_val ^ r8_val;
            
            /* Another call with different pattern */
            if (j == 2) {
                external_func2(&rdx_val);
            }
            
            /* Store results back using the live values */
            data[i] = (int)(r10_val + rdx_val);
        }
        
        /* Function call in outer loop with indirect call */
        if (i % 2 == 0) {
            external_func1(&data[i]);
        } else {
            external_func3((unsigned long*)&data[i]);
        }
        
        /* More register-intensive calculations */
        r10_val += call_saved1;
        r11_val += call_saved2;
        r8_val += call_saved3;
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(r8_val));
    }
    
    /* Final computation using all registers */
    long result = r10_val + r11_val + r8_val + r9_val + rcx_val + rdx_val +
                  call_saved1 + call_saved2 + call_saved3;
    
    /* Force use of result to prevent elimination */
    asm volatile("" : "+r"(result));
    return result;
}

/* Alternate test with different pattern */
long __attribute__((noinline)) test_function2(int *data, int size) {
    register long rax_val asm("rax") = 0;
    register long rbx_val asm("rbx") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    
    volatile int trigger = 0;
    
    for (int i = 0; i < size; i += 4) {
        /* Load and compute in call-clobbered registers */
        rax_val = data[i];
        rcx_val = data[i + 1];
        rdx_val = data[i + 2];
        
        /* Multiple calls in sequence with live values */
        for (int k = 0; k < 2; k++) {
            rax_val = rax_val * rcx_val + rdx_val;
            external_func1(&data[i + k]);
            
            rcx_val = rcx_val ^ rax_val;
            external_func2((long*)&data[i + 1]);
            
            rdx_val = rdx_val + rcx_val * 7;
            external_func3((unsigned long*)&data[i + 2]);
            
            /* Inline assembly to create register pressure */
            asm volatile("mov %1, %0\n\t"
                         "add %2, %0"
                         : "=r"(rbx_val)
                         : "r"(rax_val), "r"(rcx_val)
                         : "cc");
        }
        
        /* Store results */
        data[i] = (int)(rax_val + rbx_val + rcx_val + rdx_val);
        trigger = data[i];  /* Volatile use */
    }
    
    return rax_val + rbx_val + rcx_val + rdx_val + trigger;
}

int main() {
    /* Initialize test data */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i * 3 + 7;
    }
    
    /* Array of function pointers for indirect calls */
    func_ptr_t funcs[] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    long total_result = 0;
    
    /* Multiple test runs with different patterns */
    for (int run = 0; run < 10; run++) {
        /* Test with indirect calls */
        total_result += test_function(data, 128, funcs[run % 3]);
        
        /* Test with direct calls */
        total_result += test_function2(data + 128, 128);
        
        /* Modify data slightly each run */
        for (int i = 0; i < 256; i++) {
            data[i] = data[i] * 1103515245 + 12345;
        }
    }
    
    printf("Final result: %ld\n", total_result);
    
    /* Verify some computation was done */
    int check = 0;
    for (int i = 0; i < 256; i++) {
        check ^= data[i];
    }
    printf("Data checksum: %d\n", check);
    
    return 0;
}
