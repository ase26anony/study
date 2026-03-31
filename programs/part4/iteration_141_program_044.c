/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __GNUC__
__attribute__((noinline, noclone))
#else
__declspec(noinline)
#endif
void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers via inline asm */
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

#ifdef __GNUC__
__attribute__((noinline, noclone))
#else
__declspec(noinline)
#endif
void external_func2(volatile long* p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile void*);

/* Force values into specific registers using explicit register variables */
#ifdef __x86_64__
#define FORCE_REGISTER(var, reg) register long var asm(reg)
#elif defined(__i386__)
#define FORCE_REGISTER(var, reg) register long var asm(reg)
#else
#define FORCE_REGISTER(var, reg) register long var
#endif

/* Main test function with complex register usage */
long test_caller_save_scenario(int* data, int size, func_ptr_t fp1, func_ptr_t fp2) {
    volatile int barrier = 0;
    long result = 0;
    
    /* Use explicit register variables to create pressure on call-clobbered regs */
    FORCE_REGISTER(r10_val, "r10") = data[0];
    FORCE_REGISTER(r11_val, "r11") = data[1];
    FORCE_REGISTER(rcx_val, "rcx") = data[2];
    FORCE_REGISTER(rdx_val, "rdx") = data[3];
    
    /* Mix of call-clobbered and call-saved register usage */
    register long rbx_val asm("rbx") = data[4];  /* Call-saved */
    register long rbp_val asm("rbp") = data[5];  /* Call-saved */
    
    for (int i = 0; i < size; i++) {
        /* Complex live ranges spanning calls */
        FORCE_REGISTER(temp1, "rax") = data[i];
        FORCE_REGISTER(temp2, "rsi") = data[(i + 1) % size];
        
        /* Create artificial dependencies with volatile and inline asm */
        asm volatile("" : "+r"(temp1), "+r"(temp2));
        
        /* Keep values live across function calls */
        r10_val += temp1;
        r11_val ^= temp2;
        rcx_val *= (temp1 + 1);
        rdx_val -= temp2;
        
        /* Use volatile to prevent optimization */
        barrier = i;
        
        /* Indirect call via function pointer - inhibits optimizations */
        func_ptr_t fp = (i & 1) ? fp1 : fp2;
        fp((volatile void*)&barrier);
        
        /* More operations keeping registers live */
        rbx_val += r10_val;
        rbp_val ^= r11_val;
        
        /* Another call with different register pressure */
        if (i % 3 == 0) {
            external_func1(&barrier);
        } else {
            external_func2((volatile long*)&barrier);
        }
        
        /* Use all register values in computation */
        result += r10_val + r11_val + rcx_val + rdx_val + rbx_val + rbp_val;
        
        /* Force register spill/reload with array access */
        data[i] = (int)(result & 0xFFFFFFFF);
        
        /* Inline asm to create barriers and prevent reordering */
        asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(rcx_val), "r"(rdx_val));
    }
    
    /* Final computation using all registers */
    result = r10_val * r11_val + rcx_val / (rdx_val ? rdx_val : 1) + rbx_val - rbp_val;
    
    /* Ensure all values are used */
    asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(rcx_val), "r"(rdx_val),
                       "r"(rbx_val), "r"(rbp_val));
    
    return result;
}

/* Nested loop variant with more complex patterns */
long nested_loop_test(int* data, int outer, int inner) {
    long total = 0;
    
    for (int i = 0; i < outer; i++) {
        FORCE_REGISTER(acc1, "r8") = data[i];
        FORCE_REGISTER(acc2, "r9") = data[outer - i - 1];
        
        for (int j = 0; j < inner; j++) {
            /* Register-intensive calculations */
            FORCE_REGISTER(tmp1, "rax") = data[j] * i;
            FORCE_REGISTER(tmp2, "rcx") = data[inner - j - 1] + j;
            
            /* Function call in inner loop */
            external_func1((volatile int*)&data[j]);
            
            /* Keep values live across call */
            acc1 += tmp1;
            acc2 ^= tmp2;
            
            /* Another call with different pattern */
            if ((i + j) % 2 == 0) {
                external_func2((volatile long*)&acc1);
            }
            
            /* Use inline asm to force specific register usage */
            asm volatile("add %1, %0" : "+r"(acc1) : "r"(tmp2));
            asm volatile("xor %1, %0" : "+r"(acc2) : "r"(tmp1));
            
            total += acc1 + acc2;
        }
        
        /* Store results back, creating more register pressure */
        data[i] = (int)(acc1 & 0x7FFFFFFF);
    }
    
    return total;
}

int main() {
    const int DATA_SIZE = 256;
    int* data = (int*)malloc(DATA_SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Create function pointers to inhibit optimizations */
    func_ptr_t funcs[2];
    funcs[0] = (func_ptr_t)external_func1;
    funcs[1] = (func_ptr_t)external_func2;
    
    /* Run multiple test patterns to increase coverage probability */
    long result1 = test_caller_save_scenario(data, DATA_SIZE / 2, funcs[0], funcs[1]);
    long result2 = nested_loop_test(data + DATA_SIZE / 2, 16, 8);
    
    /* Final computation using results */
    long final_result = result1 ^ result2;
    
    /* Use volatile to ensure all computations are kept */
    volatile long print_result = final_result;
    
    printf("Result: %ld\n", print_result);
    
    /* Verify data was modified */
    int checksum = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        checksum ^= data[i];
    }
    printf("Checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
