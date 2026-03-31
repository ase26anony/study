/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __x86_64__
#define CLOBBER_REGS "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_REGS "eax", "ecx", "edx", "esi", "edi"
#endif

/* Force noinline to ensure actual calls */
__attribute__((noinline)) void external_func1(int *p) {
    *p += 1;
    /* Clobber call-clobbered registers */
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline)) void external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline)) void external_func3(unsigned *p) {
    *p ^= 0xAAAA;
    asm volatile("" : : : CLOBBER_REGS);
}

/* Mix of direct and indirect calls */
typedef void (*func_ptr_t)(void*);

/* Use volatile to prevent optimizations */
volatile int global_seed = 42;

int main(void) {
    /* Large array to create register pressure */
    int data[256];
    unsigned long results[4] = {0};
    
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Function pointers for indirect calls */
    func_ptr_t funcs[] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Nested loops with calls inside */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables to pressure specific registers */
        register long r10_val asm("r10") = outer * 100;
        register long r11_val asm("r11") = outer * 200;
        
        for (int inner = 0; inner < 50; inner++) {
            /* Multiple live values in call-clobbered registers */
            int idx1 = (inner * 7) & 0xFF;
            int idx2 = (inner * 13) & 0xFF;
            int idx3 = (inner * 19) & 0xFF;
            
            /* Load values into variables that must survive calls */
            int val1 = data[idx1];
            int val2 = data[idx2];
            int val3 = data[idx3];
            
            /* Force values into registers with inline asm */
            asm volatile("" : "+r"(val1), "+r"(val2), "+r"(val3));
            
            /* Mix of arithmetic keeping values live */
            long sum = val1 + val2 + val3 + r10_val;
            long prod = val1 * val2 * (val3 + 1);
            
            /* Use volatile to prevent reordering */
            volatile int temp = global_seed;
            
            /* Indirect call - compiler can't optimize across */
            func_ptr_t fp = funcs[(inner + temp) % 3];
            
            /* Call with live values in registers */
            if ((inner & 1) == 0) {
                fp(&val1);
                sum += val1;
            } else {
                fp(&val2);
                prod += val2;
            }
            
            /* More computations keeping values live */
            long diff = sum - prod;
            long scaled = diff * (inner + 1);
            
            /* Another direct call with different live values */
            external_func1(&val3);
            
            /* Use all computed values to create dependencies */
            results[0] += sum;
            results[1] += prod;
            results[2] += diff;
            results[3] += scaled + r11_val;
            
            /* Store back to array creating more register pressure */
            data[idx1] = val1 + 1;
            data[idx2] = val2 - 1;
            data[idx3] = val3 * 2;
            
            /* Update register variables */
            r10_val += inner;
            r11_val -= inner;
            
            /* Prevent loop unrolling */
            asm volatile("" : : : "memory");
        }
        
        /* Call with complex expression */
        if (outer & 1) {
            external_func2(&results[outer & 3]);
        } else {
            external_func3((unsigned*)&results[outer & 3]);
        }
    }
    
    /* Final computation and output */
    unsigned long final_result = 0;
    for (int i = 0; i < 4; i++) {
        final_result ^= results[i];
        final_result = (final_result << 13) | (final_result >> (64 - 13));
    }
    
    printf("Result: %lu\n", final_result);
    
    /* Verify by also printing a checksum */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += data[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return (final_result > 1000000) ? 0 : 1;
}
