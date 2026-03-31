/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Volatile variables to prevent optimization */
volatile int trigger = 1;

int main(void) {
    /* Large array to create register pressure */
    int data[256];
    long accum[8] = {0};
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Function pointers to inhibit optimizations */
    func_ptr_t funcs[2] = {(func_ptr_t)external_func1, (func_ptr_t)external_func2};
    
    /* Explicit register variables targeting call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    
    /* Main computation loop with nested loops */
    for (int outer = 0; outer < 100; outer++) {
        /* Reset register values each outer iteration */
        r10_val = outer;
        r11_val = outer * 2;
        r8_val = outer * 3;
        r9_val = outer * 4;
        
        /* Inner loop with function call and register-intensive operations */
        for (int i = 0; i < 128; i++) {
            /* Load values into call-clobbered registers */
            register long rax_val asm("rax") = data[i];
            register long rcx_val asm("rcx") = data[i + 128];
            register long rdx_val asm("rdx") = data[255 - i];
            
            /* Complex computation keeping values live in registers */
            rax_val = rax_val * r10_val + r11_val;
            rcx_val = rcx_val * r8_val - r9_val;
            rdx_val = rdx_val * rax_val / (rcx_val + 1);
            
            /* Mix with call-saved registers (rbx, rbp, r12-r15) */
            register long rbx_val asm("rbx") = accum[0];
            register long r12_val asm("r12") = accum[1];
            
            /* Artificial dependency with volatile */
            if (trigger) {
                rbx_val += rax_val;
                r12_val += rcx_val;
            }
            
            /* Save call-clobbered values to memory before call */
            long temp1 = rax_val;
            long temp2 = rcx_val;
            long temp3 = rdx_val;
            
            /* Call external function via function pointer */
            int idx = (i + outer) & 1;
            if (idx == 0) {
                external_func1((int*)&temp1);
            } else {
                external_func2(&temp2);
            }
            
            /* Restore and continue computation */
            rax_val = temp1 + rbx_val;
            rcx_val = temp2 + r12_val;
            rdx_val = temp3 * 2;
            
            /* Store results back, creating anti-dependencies */
            data[i] = (int)(rax_val & 0x7FFFFFFF);
            data[i + 128] = (int)(rcx_val & 0x7FFFFFFF);
            data[255 - i] = (int)(rdx_val & 0x7FFFFFFF);
            
            /* Update accumulators in call-saved registers */
            accum[0] = rbx_val;
            accum[1] = r12_val;
            
            /* Inline assembly to prevent reordering */
            asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val));
        }
        
        /* Update register variables for next iteration */
        r10_val += accum[0];
        r11_val += accum[1];
        r8_val ^= r10_val;
        r9_val ^= r11_val;
        
        /* Another function call with different register usage pattern */
        if (outer % 3 == 0) {
            long temp = r10_val + r11_val + r8_val + r9_val;
            external_func2(&temp);
            r10_val = temp;
        }
    }
    
    /* Final computation and output */
    long final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += data[i];
    }
    final_sum += accum[0] + accum[1];
    
    printf("Result: %ld\n", final_sum);
    return (final_sum > 0) ? 0 : 1;
}
