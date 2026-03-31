/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that clobber registers */
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
void external_func2(volatile int* p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

int main(void) {
    /* Large array to work with */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + global_seed;
    }
    
    /* Function pointers to inhibit optimizations */
    func_ptr_t funcs[2] = {external_func1, external_func2};
    
    /* Result accumulator */
    int result = 0;
    
    /* Nested loops with register-intensive calculations */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables for call-clobbered registers */
        register long r10_val asm("r10") = data[outer] * 3;
        register long r11_val asm("r11") = data[outer + 1] * 7;
        register long r9_val asm("r9") = data[outer + 2] * 11;
        
        for (int inner = 0; inner < 20; inner++) {
            /* More register variables */
            register long r8_val asm("r8") = data[inner] * 5;
            register long rcx_val asm("rcx") = data[inner + 10] * 13;
            register long rdx_val asm("rdx") = data[inner + 20] * 17;
            
            /* Complex calculation keeping values live in registers */
            r10_val = (r10_val * r8_val + rcx_val) / (rdx_val + 1);
            r11_val = (r11_val ^ rcx_val) | (r8_val & rdx_val);
            r9_val = r9_val * 3 + r10_val - r11_val;
            
            /* Volatile memory operations to create dependencies */
            volatile int temp1 = r10_val;
            volatile int temp2 = r11_val;
            volatile int temp3 = r9_val;
            
            /* Call external function via pointer - forces caller-save */
            funcs[inner & 1](&temp1);
            
            /* Use the values after call - they need to be restored */
            r8_val = temp1 + r10_val;
            rcx_val = temp2 ^ r11_val;
            rdx_val = temp3 * r9_val;
            
            /* More calculations */
            r10_val = r8_val + rcx_val * 2;
            r11_val = rdx_val - r9_val / 3;
            r9_val = (r10_val << 2) | (r11_val >> 1);
            
            /* Store results back to array */
            data[(outer * 20 + inner) % 256] = r9_val;
            
            /* Inline asm to prevent reordering/optimization */
            asm volatile("" : "+r"(r8_val), "+r"(rcx_val), "+r"(rdx_val));
        }
        
        /* Accumulate results */
        result += r10_val + r11_val + r9_val;
        
        /* Another function call with live values */
        external_func1((volatile int*)&result);
        
        /* More register operations */
        register long rax_val asm("rax") = result * 19;
        register long rsi_val asm("rsi") = data[outer] * 23;
        register long rdi_val asm("rdi") = data[outer + 100] * 29;
        
        rax_val = (rax_val + rsi_val) * rdi_val;
        rsi_val = rax_val ^ rdi_val;
        
        /* Force spill/reload around call */
        volatile int save_rax = rax_val;
        external_func2((volatile int*)&save_rax);
        rax_val = save_rax;
        
        result = rax_val + rsi_val;
    }
    
    /* Final computation with mixed register usage */
    {
        register long r10_final asm("r10") = result;
        register long r11_final asm("r11") = global_seed;
        register long r9_final asm("r9") = 0;
        
        for (int i = 0; i < 50; i++) {
            r9_final += data[i] * i;
            
            /* Periodic function calls */
            if (i % 7 == 0) {
                volatile int tmp = r10_final;
                external_func1(&tmp);
                r10_final = tmp + r9_final;
            }
            
            r11_final = (r11_final * 31 + r9_final) % 1000;
        }
        
        result = r10_final + r11_final + r9_final;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
