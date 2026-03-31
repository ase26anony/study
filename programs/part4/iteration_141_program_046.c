/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_LIST "eax", "ecx", "edx"
#endif

/* Force noinline to prevent optimization */
__attribute__((noinline)) 
void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers via inline asm */
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline))
void external_func2(volatile int* p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline))
void external_func3(volatile int* p) {
    *p -= 3;
    asm volatile("" : : : CLOBBER_LIST);
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

int main() {
    /* Large array to create register pressure */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    int num_funcs = sizeof(funcs) / sizeof(funcs[0]);
    
    int result = 0;
    
    /* Nested loops with calls - creates complex live ranges */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables for call-clobbered registers */
        #ifdef __x86_64__
        register int64_t r10_val asm("r10") = data[outer * 5];
        register int64_t r11_val asm("r11") = data[outer * 5 + 1];
        register int64_t r9_val asm("r9") = data[outer * 5 + 2];
        #else
        register int32_t eax_val asm("eax") = data[outer * 5];
        register int32_t ecx_val asm("ecx") = data[outer * 5 + 1];
        register int32_t edx_val asm("edx") = data[outer * 5 + 2];
        #endif
        
        for (int inner = 0; inner < 100; inner++) {
            /* Create many live values in registers */
            int idx = (outer * 100 + inner) % 256;
            
            /* Mix of operations keeping values live in registers */
            #ifdef __x86_64__
            r10_val = r10_val * 3 + data[idx];
            r11_val = r11_val / 2 + data[(idx + 1) % 256];
            r9_val = r9_val ^ data[(idx + 2) % 256];
            
            /* Volatile to prevent reordering */
            volatile int temp = r10_val + r11_val;
            
            /* Function call with live values in call-clobbered registers */
            funcs[inner % num_funcs]((int*)&temp);
            
            /* Use values after call - forces save/restore */
            result += r10_val - r11_val + r9_val;
            
            /* More operations to extend live ranges */
            r10_val = result ^ r11_val;
            r11_val = result + r9_val;
            r9_val = r10_val * r11_val;
            #else
            eax_val = eax_val * 3 + data[idx];
            ecx_val = ecx_val / 2 + data[(idx + 1) % 256];
            edx_val = edx_val ^ data[(idx + 2) % 256];
            
            volatile int temp = eax_val + ecx_val;
            funcs[inner % num_funcs]((int*)&temp);
            
            result += eax_val - ecx_val + edx_val;
            eax_val = result ^ ecx_val;
            ecx_val = result + edx_val;
            edx_val = eax_val * ecx_val;
            #endif
            
            /* Inline asm to create artificial dependencies */
            asm volatile("" : "+r"(result));
        }
        
        /* Store results back to array */
        #ifdef __x86_64__
        data[outer * 5] = r10_val;
        data[outer * 5 + 1] = r11_val;
        data[outer * 5 + 2] = r9_val;
        #else
        data[outer * 5] = eax_val;
        data[outer * 5 + 1] = ecx_val;
        data[outer * 5 + 2] = edx_val;
        #endif
    }
    
    /* Final computation using all data */
    int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += data[i];
        /* Prevent loop optimization */
        asm volatile("" : "+r"(final_sum));
    }
    
    printf("Result: %d, Final sum: %d\n", result, final_sum);
    return 0;
}
