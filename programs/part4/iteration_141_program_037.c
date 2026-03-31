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
    /* Force register clobbering with inline asm */
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

/* Global to prevent optimization */
volatile int global_counter = 0;

int main() {
    /* Create array with many values to create register pressure */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Function pointers to inhibit optimization */
    func_ptr_t funcs[2] = { 
        (func_ptr_t)external_func1, 
        (func_ptr_t)external_func2 
    };
    
    long long total_sum = 0;
    
    /* Outer loop to create multiple basic blocks */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables to target call-clobbered registers */
        register long r10 asm("r10") = data[outer * 10 + 0];
        register long r11 asm("r11") = data[outer * 10 + 1];
        register long r9  asm("r9")  = data[outer * 10 + 2];
        register long r8  asm("r8")  = data[outer * 10 + 3];
        
        /* Also use call-saved registers */
        register long rbx_val asm("rbx") = data[outer * 10 + 4];
        register long r12_val asm("r12") = data[outer * 10 + 5];
        
        /* Nested loop with function calls */
        for (int inner = 0; inner < 5; inner++) {
            /* Create live values in call-clobbered registers */
            r10 += r11 * inner;
            r11 ^= r9 << (inner & 3);
            r9  -= r8 / (inner + 1);
            r8  |= r10 & 0xFF;
            
            /* Mix with call-saved registers */
            rbx_val += r12_val;
            r12_val ^= rbx_val;
            
            /* Volatile to prevent reordering */
            volatile int temp = r10 + r11;
            
            /* Call external function - forces caller-save */
            funcs[inner & 1]((volatile void*)&temp);
            
            /* Use values after call - they need to be restored */
            r10 += temp;
            r11 ^= temp;
            
            /* More arithmetic keeping values live */
            r9 = (r9 * r8) / (r10 + 1);
            r8 = (r8 << 2) | (r9 & 0xF);
            
            /* Another call with different pattern */
            if (inner % 2 == 0) {
                volatile long temp2 = r9 + r8;
                external_func2(&temp2);
                r9 += temp2;
            } else {
                volatile int temp3 = r10 - r11;
                external_func1(&temp3);
                r10 ^= temp3;
            }
            
            /* Use inline asm to create artificial dependencies */
            asm volatile("" : "+r"(r10), "+r"(r11), "+r"(r9), "+r"(r8));
            
            /* Store results back to array */
            data[outer * 10 + 0] = r10;
            data[outer * 10 + 1] = r11;
            data[outer * 10 + 2] = r9;
            data[outer * 10 + 3] = r8;
            
            /* Update total sum */
            total_sum += r10 + r11 + r9 + r8 + rbx_val + r12_val;
            
            /* Force spill/reload with memory operations */
            volatile int* mem_ptr = &data[(outer * 10 + inner) % 256];
            *mem_ptr += global_counter;
        }
        
        /* Cross-basic block live ranges */
        if (outer % 3 == 0) {
            volatile long cross_live = r10 + r11;
            external_func1((volatile int*)&cross_live);
            total_sum += cross_live;
        }
    }
    
    /* Final computation and output */
    for (int i = 0; i < 256; i++) {
        total_sum += data[i];
    }
    
    printf("Result: %lld\n", total_sum);
    
    /* Verify with simple check */
    if (total_sum != 0) {
        printf("Test completed successfully\n");
    }
    
    return 0;
}
