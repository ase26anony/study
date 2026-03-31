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
__attribute__((noinline)) 
void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers */
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline))
void external_func2(volatile long* p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline))
void external_func3(volatile int* p, volatile long* q) {
    *p ^= *q;
    asm volatile("" : : : CLOBBER_REGS);
}

/* Mix of direct and indirect calls */
typedef void (*func_ptr_t)(volatile int*);

/* Global to prevent optimization */
volatile int global_counter = 0;

int main(void) {
    /* Large array to create register pressure */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Function pointers for indirect calls */
    func_ptr_t funcs[] = {external_func1, external_func1, external_func2};
    int func_index = 0;
    
    /* Explicit register variables to target specific call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register int ecx_val asm("ecx") = 0;
    register int edx_val asm("edx") = 0;
    
    /* Call-saved register usage mixed with call-clobbered */
    register long rbx_val asm("rbx") = 0;  /* Call-saved on x86-64 */
    register long r12_val asm("r12") = 0;  /* Call-saved on x86-64 */
    
    /* Nested loops with calls to create complex live ranges */
    for (int outer = 0; outer < 10; outer++) {
        rbx_val = outer * 100;
        r12_val = outer * 200;
        
        for (int inner = 0; inner < 100; inner++) {
            /* Load values into call-clobbered registers */
            r10_val = data[inner % 256];
            r11_val = data[(inner + 1) % 256];
            ecx_val = data[(inner + 2) % 256];
            edx_val = data[(inner + 3) % 256];
            
            /* Perform arithmetic keeping results live in call-clobbered regs */
            r10_val = r10_val * 3 + ecx_val;
            r11_val = r11_val / 2 + edx_val;
            ecx_val = ecx_val ^ r10_val;
            edx_val = edx_val | r11_val;
            
            /* Mix with call-saved registers */
            rbx_val += r10_val;
            r12_val += r11_val;
            
            /* Volatile to prevent reordering */
            volatile int temp1 = ecx_val;
            volatile long temp2 = edx_val;
            
            /* Call external function - forces caller-save for live values */
            if (inner % 3 == 0) {
                external_func1(&temp1);
            } else if (inner % 3 == 1) {
                /* Indirect call */
                funcs[func_index](&temp1);
                func_index = (func_index + 1) % 3;
            } else {
                external_func3(&temp1, &temp2);
            }
            
            /* Use values after call - they need to be restored */
            r10_val += temp1;
            r11_val += temp2;
            ecx_val ^= rbx_val;
            edx_val ^= r12_val;
            
            /* Store results back, creating dependencies */
            data[inner % 256] = r10_val + ecx_val;
            data[(inner + 1) % 256] = r11_val + edx_val;
            
            /* Inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(ecx_val), "+r"(edx_val));
        }
        
        /* Another call with different register usage pattern */
        volatile int sum = rbx_val + r12_val;
        external_func2(&sum);
        global_counter += sum;
        
        /* Complex expression spanning multiple calls */
        for (int i = 0; i < 5; i++) {
            register int eax_val asm("eax") = data[i];
            register int esi_val asm("esi") = data[i + 10];
            
            eax_val = eax_val * esi_val + i;
            external_func1(&eax_val);
            esi_val = esi_val ^ eax_val;
            external_func2(&esi_val);
            
            data[i] = eax_val;
            data[i + 10] = esi_val;
            
            /* Force spill/reload around calls */
            asm volatile("" : : "r"(eax_val), "r"(esi_val) : "memory");
        }
    }
    
    /* Compute final result */
    long final_result = 0;
    for (int i = 0; i < 256; i++) {
        final_result += data[i];
    }
    final_result += global_counter;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 1000);
}
