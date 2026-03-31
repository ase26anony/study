/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Volatile variables to prevent optimization */
volatile int trigger = 0;
volatile long accumulator = 0;

int main(void) {
    /* Large array to work with */
    int data[256];
    long results[128];
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Function pointers to inhibit optimization */
    func_ptr_t funcs[2];
    funcs[0] = (func_ptr_t)external_func1;
    funcs[1] = (func_ptr_t)external_func2;
    
    /* Main computation loop with nested loops */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables for call-clobbered registers */
        register long r10 asm("r10") = outer * 100;
        register long r11 asm("r11") = outer * 200;
        register int rcx_val asm("rcx") = 0;
        
        for (int inner = 0; inner < 128; inner++) {
            /* Load values into registers - creating live ranges */
            register long rax_val asm("rax") = data[inner * 2];
            register long rdx_val asm("rdx") = data[inner * 2 + 1];
            register long r8_val asm("r8") = inner;
            register long r9_val asm("r9") = inner * 2;
            
            /* Complex computation keeping values live in registers */
            rax_val = rax_val * rdx_val + r10;
            rdx_val = rdx_val - r11 + r8_val;
            r8_val = r8_val ^ r9_val;
            r9_val = r9_val | rax_val;
            
            /* Mix with call-saved registers */
            register long rbx_val asm("rbx") = rax_val;
            register long r12_val asm("r12") = rdx_val;
            register long r13_val asm("r13") = r8_val;
            register long r14_val asm("r14") = r9_val;
            
            /* Artificial dependency with volatile */
            trigger = inner;
            
            /* Call external function - forcing caller-save */
            if (inner % 2 == 0) {
                external_func1(&data[inner]);
            } else {
                external_func2((long*)&data[inner]);
            }
            
            /* Use function pointer for indirect call */
            if (inner % 3 == 0) {
                funcs[inner % 2]((void*)&data[inner + 1]);
            }
            
            /* More computation after call - values must be restored */
            rbx_val = rbx_val + r12_val;
            r13_val = r13_val - r14_val;
            
            /* Use inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(rax_val), "+r"(rdx_val), "+r"(r8_val), "+r"(r9_val));
            
            /* Store results - mixing register values */
            results[inner] = rax_val + rdx_val + r8_val + r9_val + 
                            rbx_val + r12_val + r13_val + r14_val + 
                            r10 + r11;
            
            /* Update register variables for next iteration */
            r10 += inner;
            r11 -= inner;
            rcx_val++;
        }
        
        /* Another loop with different pattern */
        for (int i = 0; i < 64; i++) {
            register double xmm0 asm("xmm0") = results[i];
            register double xmm1 asm("xmm1") = results[i + 64];
            
            /* Force floating point register usage */
            xmm0 = xmm0 * 1.5;
            xmm1 = xmm1 / 2.0;
            
            /* Call with floating point values live */
            external_func1(&data[i]);
            
            /* Use values after call */
            results[i] = (long)(xmm0 + xmm1);
            
            /* Inline assembly to prevent reordering */
            asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3", 
                        "xmm4", "xmm5", "xmm6", "xmm7");
        }
    }
    
    /* Compute final result */
    long final_sum = 0;
    for (int i = 0; i < 128; i++) {
        final_sum += results[i];
    }
    
    /* Also sum data array */
    int data_sum = 0;
    for (int i = 0; i < 256; i++) {
        data_sum += data[i];
    }
    
    printf("Final sum: %ld, Data sum: %d\n", final_sum, data_sum);
    
    return (final_sum > 0 && data_sum > 0) ? 0 : 1;
}
