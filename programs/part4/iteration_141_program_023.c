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

/* Main test function with register pressure */
long __attribute__((noinline)) test_caller_save(int *data, int size) {
    volatile int barrier = 0;
    long total = 0;
    
    /* Force values into specific call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r9_val asm("r9") = 0;
    register long r8_val asm("r8") = 0;
    
    /* Function pointers to inhibit optimization */
    func_ptr_t funcs[3] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Nested loops to create complex live ranges */
    for (int i = 0; i < size; i++) {
        /* Load values into call-clobbered registers */
        r10_val = data[i] * 3L;
        r11_val = data[i] + 0x7FFFFFFFL;
        
        /* Create artificial dependencies with inline asm */
        asm volatile("" : "+r"(r10_val), "+r"(r11_val));
        
        for (int j = 0; j < 3; j++) {
            /* More register pressure with call-saved registers */
            long rbx_val = r10_val * 2;
            long r12_val = r11_val / 3;
            long r13_val = rbx_val ^ r12_val;
            long r14_val = r13_val + i;
            long r15_val = r14_val - j;
            
            /* Mix of operations keeping values live */
            r8_val = r10_val + r11_val + rbx_val;
            r9_val = r12_val * r13_val - r14_val;
            
            /* Prevent reordering */
            barrier = i + j;
            
            /* Indirect call that clobbers registers */
            if (barrier & 1) {
                funcs[j % 3]((void*)&r8_val);
            } else {
                funcs[(j + 1) % 3]((void*)&r9_val);
            }
            
            /* Use values after call - forces save/restore */
            r10_val = r8_val ^ r9_val;
            r11_val = r10_val + r15_val;
            
            /* More inline asm to prevent optimization */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r8_val), "+r"(r9_val));
            
            /* Store intermediate results */
            data[(i * 3 + j) % size] = (int)(r10_val ^ r11_val);
        }
        
        /* Accumulate total with mixed operations */
        total += r10_val - r11_val + r8_val * r9_val;
        
        /* Force spill/reload around calls */
        if (i % 4 == 0) {
            external_func1((int*)&total);
        } else if (i % 4 == 1) {
            external_func2(&total);
        } else if (i % 4 == 2) {
            external_func3((unsigned long*)&total);
        }
        
        /* Complex expression with many live values */
        r10_val = total + r11_val * r8_val / (r9_val ? r9_val : 1);
        asm volatile("" : "+r"(r10_val));
    }
    
    return total;
}

/* Another test function with different pattern */
long __attribute__((noinline)) test_caller_save2(int *data, int size) {
    register long rax_val asm("rax") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    register long rsi_val asm("rsi") = 0;
    register long rdi_val asm("rdi") = 0;
    
    long result = 0;
    
    for (int i = 0; i < size; i += 2) {
        /* Load multiple values into call-clobbered registers */
        rax_val = data[i];
        rcx_val = data[i + 1];
        rdx_val = rax_val * rcx_val;
        rsi_val = rax_val + rcx_val;
        rdi_val = rax_val ^ rcx_val;
        
        /* Force all values to be live across call */
        asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val), 
                          "+r"(rsi_val), "+r"(rdi_val));
        
        /* Call that clobbers everything */
        external_func1(&data[i]);
        
        /* Use all values after call */
        rax_val = rax_val + rcx_val - rdx_val;
        rcx_val = rsi_val * rdi_val;
        rdx_val = rax_val ^ rcx_val;
        
        /* Another call with different clobber pattern */
        external_func2((long*)&rdx_val);
        
        /* More operations */
        rsi_val = rdx_val + rax_val;
        rdi_val = rcx_val * 3;
        
        /* Store results */
        data[i] = (int)(rsi_val ^ rdi_val);
        result += rax_val + rcx_val + rdx_val + rsi_val + rdi_val;
    }
    
    return result;
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Run both test functions */
    long result1 = test_caller_save(data, SIZE);
    long result2 = test_caller_save2(data, SIZE);
    
    /* Final computation using results */
    long final_result = result1 ^ result2;
    
    /* Use volatile to prevent dead code elimination */
    volatile long print_me = final_result;
    
    printf("Result: %ld\n", print_me);
    
    /* Verify data was modified */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= data[i];
    }
    printf("Checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
