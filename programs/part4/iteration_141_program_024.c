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
void external_func1(volatile int* ptr) {
    *ptr += 1;
    /* Clobber call-clobbered registers */
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline))
void external_func2(volatile int* ptr) {
    *ptr *= 2;
    asm volatile("" : : : CLOBBER_REGS);
}

__attribute__((noinline))
void external_func3(volatile int* ptr) {
    *ptr -= 3;
    asm volatile("" : : : CLOBBER_REGS);
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

int main() {
    /* Large array to create register pressure */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    
    /* Explicit register variables for x86-64 */
#ifdef __x86_64__
    register long r10 asm("r10");
    register long r11 asm("r11");
    register long r12 asm("r12");  /* Call-saved */
    register long r13 asm("r13");  /* Call-saved */
    register long r14 asm("r14");  /* Call-saved */
    register long r15 asm("r15");  /* Call-saved */
#else
    /* For 32-bit, use available registers */
    register int r10 asm("eax");
    register int r11 asm("ecx");
    register int r12 asm("ebx");  /* Call-saved */
    register int r13 asm("esi");  /* Call-saved */
    register int r14 asm("edi");  /* Call-saved */
#endif
    
    int result = 0;
    
    /* Outer loop to create multiple basic blocks */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with function calls and register pressure */
        for (int i = 0; i < 100; i++) {
            /* Load values into specific registers */
            r10 = data[i % 256];
            r11 = data[(i + 1) % 256];
            
            /* Use call-saved registers for values that must survive */
            r12 = data[(i + 2) % 256];
            r13 = data[(i + 3) % 256];
            
            /* Complex calculation using all registers */
            r14 = r10 * r11 + r12 - r13;
            
            /* Volatile inline assembly to prevent reordering */
            asm volatile("" : "+r"(r10), "+r"(r11), "+r"(r12), "+r"(r13));
            
            /* Call external function - forces caller-save for r10, r11 */
            volatile int temp = r14;
            funcs[i % 3](&temp);
            
            /* More calculations with the saved values */
            r15 = r10 + r11 + r12 + r13 + temp;
            
            /* Store result back, creating dependency chain */
            data[i % 256] = r15;
            
            /* Use result in global volatile */
            global_counter += r15;
            
            /* Another volatile asm to create artificial dependencies */
            asm volatile("" : : "r"(r10), "r"(r11), "r"(r12), "r"(r13), "r"(r14));
        }
        
        /* Nested loop with different pattern */
        for (int j = 0; j < 50; j++) {
            /* Alternate between direct and indirect calls */
            func_ptr_t fp = (j % 2) ? external_func1 : external_func2;
            
            /* Create live values across calls */
            r10 = data[j * 2];
            r11 = data[j * 2 + 1];
            
            /* Force values to be in registers before call */
            asm volatile("" : "+r"(r10), "+r"(r11));
            
            volatile int val = r10 + r11;
            fp(&val);
            
            /* Use values after call */
            data[j * 2] = val + r10;
            data[j * 2 + 1] = val - r11;
            
            global_counter += val;
        }
    }
    
    /* Final computation using all registers */
    int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += data[i];
    }
    
    printf("Result: %d (global: %d)\n", final_sum, global_counter);
    return final_sum != 0 ? 0 : 1;
}
