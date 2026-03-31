/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main(void) {
    /* Declare variables with different types and storage */
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed;
    volatile int vol_var = 0;
    register int reg_var asm("r12") = 0; /* Try to tie to specific reg */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        ptr_arr[i] = (long*)&arr[i * 16];
    }
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    
    int result = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 256;
        int idx2 = iter * 13 % 256;
        int idx3 = iter * 19 % 256;
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        /* Many operands to cause register pressure */
        asm volatile (
            "mov %[out1], %[in1]\n\t"
            "add %[out1], %[in2]\n\t"
            "imul %[out1], %[in3]\n\t"
            : [out1] "=&r" (arr[idx]),   /* Early clobber output */
              [out2] "=r" (arr[idx2])
            : [in1] "r" (arr[idx3]),     /* Input in register */
              [in2] "m" (arr[idx + 1]),  /* Input in memory */
              [in3] "r" (iter),          /* Another register input */
              "0" (arr[idx]),            /* Matching constraint */
              [in4] "i" (123)            /* Immediate */
            : "cc", "memory"
        );
        
        /* Complex addressing modes for RELOAD_FOR_INPUT_ADDRESS */
        /* and RELOAD_FOR_INPADDR_ADDRESS */
        long complex_addr_result;
        asm volatile (
            "lea (%[base], %[index], 4), %%rax\n\t"
            "mov (%%rax, %[disp]), %%rbx\n\t"
            "add %%rbx, %[sum]\n\t"
            : [sum] "+r" (global_sum),
              [result] "=r" (complex_addr_result)
            : [base] "r" (arr),          /* Base register */
              [index] "r" (idx),         /* Index register */
              [disp] "i" (16),           /* Displacement */
              "m" (*(struct misaligned_data*)&packed) /* Memory operand */
            : "rax", "rbx", "cc", "memory"
        );
        
        /* Trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        long* output_ptr;
        asm volatile (
            "mov %[addr], %[outptr]\n\t"
            "movl $0x12345678, (%[outptr], %[offset], 4)\n\t"
            : [outptr] "=&r" (output_ptr)  /* Early clobber output address */
            : [addr] "r" (&arr[0]),        /* Input address */
              [offset] "r" (idx2),         /* Index offset */
              "m" (arr[0])                 /* Memory clobber */
            : "cc", "memory"
        );
        
        /* Nested address computation for RELOAD_FOR_OPERAND_ADDRESS */
        /* and RELOAD_FOR_OPADDR_ADDR */
        long** nested_ptr = &ptr_arr[iter % 8];
        asm volatile (
            "mov (%[nested]), %%rax\n\t"
            "mov (%%rax), %%rbx\n\t"
            "add %%rbx, %[total]\n\t"
            : [total] "+r" (result)
            : [nested] "r" (nested_ptr),   /* Pointer to pointer */
              "m" (**nested_ptr)           /* Memory through double indirection */
            : "rax", "rbx", "cc", "memory"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with many memory operands */
        asm volatile (
            "mov %[val1], %%eax\n\t"
            "add %[val2], %%eax\n\t"
            "mov %%eax, %[out]\n\t"
            : [out] "=m" (vol_var)        /* Memory output */
            : [val1] "m" (packed.i),      /* Misaligned memory input */
              [val2] "m" (global_counter) /* Volatile memory input */
            : "eax", "cc", "memory"
        );
        
        /* RELOAD_OTHER with inline asm that uses specific registers */
        asm volatile (
            "mov %[input], %%r10\n\t"
            "add $1, %%r10\n\t"
            "mov %%r10, %[output]\n\t"
            : [output] "=r" (reg_var)
            : [input] "0" (reg_var)       /* Matching constraint */
            : "r10", "cc"
        );
        
        /* Mix everything with volatile to prevent reordering */
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations have effect */
    long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    checksum += global_sum + result + vol_var + reg_var + packed.i;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum != 0) ? 0 : 1;
}
