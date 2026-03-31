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
    /* Inline assembly to clobber call-clobbered registers */
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

/* Global array to create register pressure */
static int data[256];

/* Initialize data array */
void init_data(void) {
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
}

/* Main test function with complex register usage */
int test_caller_save_scenario(void) {
    volatile int result = 0;
    
    /* Use explicit register variables to target specific call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    
    /* Function pointers to inhibit optimizations */
    func_ptr_t fp_array[2] = {external_func1, external_func2};
    int fp_index = 0;
    
    /* Nested loops with function calls - creates complex live ranges */
    for (int outer = 0; outer < 4; outer++) {
        /* Load values into call-clobbered registers */
        r10_val = data[outer * 16] + 1;
        r11_val = data[outer * 16 + 1] * 2;
        r8_val = data[outer * 16 + 2] - 3;
        r9_val = data[outer * 16 + 3] / 4;
        
        for (int inner = 0; inner < 8; inner++) {
            /* Create artificial dependencies to keep values live */
            volatile int temp = 0;
            
            /* Mix of arithmetic operations in call-clobbered registers */
            r10_val = r10_val + r11_val;
            r11_val = r11_val ^ r8_val;
            r8_val = r8_val | r9_val;
            r9_val = r9_val & r10_val;
            
            /* Force values to be used in memory operations */
            temp = r10_val + r11_val;
            data[outer * 16 + inner + 4] = temp;
            
            /* Call external function with many live values */
            fp_array[fp_index](&result);
            
            /* More operations after call - values must be restored */
            r10_val = r10_val - data[outer * 16 + inner];
            r11_val = r11_val + data[outer * 16 + inner + 1];
            
            /* Use inline assembly to prevent reordering/elimination */
            asm volatile("" : "+r"(r8_val), "+r"(r9_val));
            
            /* Switch function pointer to create indirect call */
            fp_index = 1 - fp_index;
            
            /* Another call with different function */
            fp_array[fp_index](&result);
            
            /* Final computation using all registers */
            r8_val = r8_val * r9_val;
            r9_val = r9_val + r10_val;
            r10_val = r10_val ^ r11_val;
            r11_val = r11_val | r8_val;
            
            /* Store results back */
            data[outer * 16 + inner + 8] = r10_val + r11_val + r8_val + r9_val;
        }
        
        /* Cross-iteration dependencies */
        r10_val = r10_val + outer;
        r11_val = r11_val - outer;
    }
    
    /* Final aggregation */
    int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += data[i];
    }
    
    return final_sum + result;
}

/* Another test with different pattern */
int test_alternate_pattern(void) {
    volatile int accumulator = 0;
    
    /* Use different register combinations */
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    register long rsi_val asm("rsi") = 0;
    register long rdi_val asm("rdi") = 0;
    
    for (int i = 0; i < 128; i++) {
        /* Load from different array positions */
        rcx_val = data[i] + i;
        rdx_val = data[i + 64] * 2;
        rsi_val = data[i + 128] - 3;
        rdi_val = data[i + 192] / 4;
        
        /* Chain of computations */
        for (int j = 0; j < 3; j++) {
            rcx_val = (rcx_val * rdx_val) + j;
            rdx_val = (rdx_val ^ rsi_val) - j;
            rsi_val = (rsi_val | rdi_val) * (j + 1);
            rdi_val = (rdi_val & rcx_val) + (j * 2);
            
            /* Call between computations */
            external_func1(&accumulator);
            
            /* Continue with modified values */
            rcx_val = rcx_val + accumulator;
            rdx_val = rdx_val - accumulator;
            
            external_func2(&accumulator);
            
            rsi_val = rsi_val ^ accumulator;
            rdi_val = rdi_val | accumulator;
        }
        
        /* Store results with mixing */
        data[i] = rcx_val;
        data[i + 64] = rdx_val;
        data[i + 128] = rsi_val;
        data[i + 192] = rdi_val;
    }
    
    return accumulator;
}

int main(void) {
    init_data();
    
    printf("Initial test...\n");
    int result1 = test_caller_save_scenario();
    printf("Result 1: %d\n", result1);
    
    /* Re-initialize for second test */
    init_data();
    
    printf("Alternate pattern test...\n");
    int result2 = test_alternate_pattern();
    printf("Result 2: %d\n", result2);
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < 256; i++) {
        final_check ^= data[i];
    }
    printf("Final checksum: %d\n", final_check);
    
    return (result1 != 0 && result2 != 0) ? 0 : 1;
}
