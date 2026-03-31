/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi"
#endif

/* Force noinline to prevent optimization */
__attribute__((noinline)) void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers */
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func2(volatile int* p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func3(volatile int* p) {
    *p -= 3;
    asm volatile("" : : : CLOBBER_LIST);
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Global to prevent optimization */
volatile int global_counter = 0;

int main() {
    /* Large array to work with */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    int num_funcs = sizeof(funcs)/sizeof(funcs[0]);
    
    /* Explicit register variables for x86-64 */
    #ifdef __x86_64__
    register long r10 asm("r10");
    register long r11 asm("r11");
    register long r8 asm("r8");
    register long r9 asm("r9");
    #endif
    
    register int rsi_val asm("esi");
    register int edi_val asm("edi");
    
    int sum = 0;
    
    /* Outer loop to create multiple basic blocks */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with register-intensive calculations */
        for (int i = 0; i < 100; i++) {
            /* Load values into specific registers */
            int idx = (i + outer) % 256;
            
            #ifdef __x86_64__
            r10 = data[idx] * 3;
            r11 = data[(idx + 1) % 256] * 5;
            r8 = data[(idx + 2) % 256] * 7;
            r9 = data[(idx + 3) % 256] * 11;
            #endif
            
            rsi_val = data[(idx + 4) % 256] * 13;
            edi_val = data[(idx + 5) % 256] * 17;
            
            /* Complex calculation keeping values live */
            int temp1 = data[idx] + rsi_val;
            #ifdef __x86_64__
            int temp2 = r10 + r11;
            int temp3 = r8 * r9;
            temp1 += temp2 - temp3;
            #endif
            
            /* Volatile to prevent reordering */
            volatile int vol = temp1;
            
            /* Call external function - forces caller-save */
            funcs[i % num_funcs]((volatile int*)&vol);
            
            /* Use the values after call - forces restore */
            #ifdef __x86_64__
            data[idx] = (r10 + r11 + r8 + r9) % 1000;
            #endif
            data[(idx + 1) % 256] = (rsi_val * edi_val) % 1000;
            
            /* Another volatile operation */
            asm volatile("" : "+r"(rsi_val), "+r"(edi_val));
            
            /* Nested calculation with another call */
            if (i % 3 == 0) {
                volatile int vol2 = data[idx] * 2;
                external_func1(&vol2);
                sum += vol2;
            }
            
            /* Mix of call-clobbered and call-saved usage */
            int saved1 = data[idx];
            int saved2 = data[(idx + 10) % 256];
            
            /* Another indirect call */
            func_ptr_t fp = (i % 2) ? external_func2 : external_func3;
            volatile int vol3 = saved1 + saved2;
            fp(&vol3);
            
            /* Use both saved and call-clobbered values */
            #ifdef __x86_64__
            data[idx] = (saved1 + r10) % 1000;
            #else
            data[idx] = (saved1 + rsi_val) % 1000;
            #endif
            
            global_counter++;
        }
        
        /* Additional pressure with different pattern */
        for (int j = 0; j < 50; j++) {
            int idx = (outer * 50 + j) % 256;
            
            /* Force values into registers */
            register int a asm("eax") = data[idx];
            register int b asm("ecx") = data[(idx + 1) % 256];
            register int c asm("edx") = data[(idx + 2) % 256];
            
            /* Calculation before call */
            int res = a * b + c;
            
            /* Call with live register values */
            external_func2((volatile int*)&res);
            
            /* Use results */
            data[idx] = res % 1000;
            
            /* Inline assembly to create dependencies */
            asm volatile("" : "+r"(a), "+r"(b), "+r"(c));
        }
    }
    
    /* Final computation to use results */
    int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += data[i];
    }
    
    printf("Result: %d (global_counter: %d)\n", final_sum, global_counter);
    return final_sum != 0 ? 0 : 1;
}
