/* reload_trigger.c - Complex program to exercise GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Volatile structures to prevent optimization */
typedef struct {
    volatile int64_t a;
    volatile int64_t b;
    volatile int64_t c[4];
} VolStruct;

typedef struct {
    VolStruct inner[8];
    volatile double matrix[16][16];
} BigVolStruct;

/* Global volatile data to force memory accesses */
volatile BigVolStruct global_data[4];
volatile int64_t global_array[256];

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Explicit register variables - create register pressure */
    register int64_t r10_var asm("r10") = 0x1234;
    register int64_t r11_var asm("r11") = 0x5678;
    register int64_t r12_var asm("r12") = 0x9ABC;
    register int64_t r13_var asm("r13") = 0xDEF0;
    register int64_t r14_var asm("r14") = 0x2468;
    
    /* Local arrays with different alignments */
    int64_t aligned_array[128] __attribute__((aligned(64)));
    VolStruct local_structs[16];
    
    /* Volatile pointers to force memory clobbers */
    volatile int64_t* volatile ptr1 = &global_array[0];
    volatile VolStruct* volatile ptr2 = &global_data[0].inner[0];
    
    /* Initialize some data */
    for (int i = 0; i < 128; i++) {
        aligned_array[i] = i * 3;
    }
    
    /* Main loop with complex addressing modes */
    for (int i = 0; i < iterations; i++) {
        /* Complex computation involving multiple registers */
        int64_t idx1 = (r10_var + i) & 0x7F;
        int64_t idx2 = (r11_var * i) & 0xF;
        int64_t idx3 = (r12_var + r13_var * i) & 0x7;
        
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS with memory constraint */
        /* Multiple operands with conflicting constraints */
        asm volatile (
            "/* Complex addressing pattern 1 */\n\t"
            "add %[out1], %[in1], %[mem1]\n\t"
            "sub %[out2], %[in2], %[mem2]"
            : [out1] "=r" (r10_var), [out2] "=r" (r11_var)
            : [in1] "r" (r12_var), 
              [mem1] "m" (*(VolStruct*)(ptr2 + idx3)),  /* Requires address reload */
              [in2] "r" (r13_var),
              [mem2] "m" (global_data[idx2].matrix[idx1][idx1])  /* Complex address */
            : "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Nested structure access with explicit register for address */
        register VolStruct* r15_ptr asm("r15") = &local_structs[idx2];
        
        asm volatile (
            "/* Pattern 2 - nested addressing */\n\t"
            "mov %[val], [%[ptr] + %[off]]\n\t"
            "add %[val], %[val], %[inc]"
            : [val] "=r" (r12_var)
            : [ptr] "r" (r15_ptr),  /* Fixed register constraint */
              [off] "r" (idx3 * sizeof(VolStruct)),  /* Computed offset */
              [inc] "irm" (i)  /* Immediate, register, or memory */
            : "memory"
        );
        
        /* Pattern 3: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output operand with memory constraint requiring address computation */
        int64_t temp_output;
        volatile int64_t* output_ptr = &aligned_array[idx1];
        
        asm volatile (
            "/* Pattern 3 - output address reload */\n\t"
            "lea %[tmp], [%[base] + %[index]*8]\n\t"
            "mov [%[out]], %[tmp]"
            : [tmp] "=&r" (temp_output)
            : [base] "r" (aligned_array),  /* Base address in register */
              [index] "r" (idx2),          /* Index in register */
              [out] "m" (*output_ptr)      /* Memory output - needs address reload */
            : "memory"
        );
        
        /* Pattern 4: RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Multiple memory operands with different addressing modes */
        asm volatile (
            "/* Pattern 4 - multiple memory operands */\n\t"
            "imul %[res], %[mem1], %[mem2]"
            : [res] "=r" (r13_var)
            : [mem1] "m" (global_array[(r10_var + idx1) & 0xFF]),  /* Complex address */
              [mem2] "m" (*(int64_t*)((char*)ptr1 + idx2 * 8))     /* Pointer arithmetic */
            : "memory", "cc"
        );
        
        /* Pattern 5: Mix immediate, register, and memory constraints */
        /* This should trigger various reload types */
        asm volatile (
            "/* Pattern 5 - mixed constraints */\n\t"
            "add %[out], %[in1], %[in2]\n\t"
            "sub %[out], %[out], %[imm]"
            : [out] "=&r" (r14_var)
            : [in1] "r" (r10_var),
              [in2] "irm" (r11_var),  /* Can be immediate, register, or memory */
              [imm] "i" (42)          /* Immediate only */
            : "cc"
        );
        
        /* Pattern 6: RELOAD_OTHER case */
        /* Complex asm with multiple clobbered registers */
        asm volatile (
            "/* Pattern 6 - multiple clobbers */\n\t"
            "mov r8, %[val1]\n\t"
            "mov r9, %[val2]\n\t"
            "add r8, r9\n\t"
            "mov %[out], r8"
            : [out] "=r" (r10_var)
            : [val1] "r" (r12_var),
              [val2] "r" (r13_var)
            : "r8", "r9", "cc"  /* Explicit clobbers force spills/reloads */
        );
        
        /* Update indices for next iteration */
        r11_var = (r11_var * 1103515245 + 12345) & 0x7FFFFFFF;
        r12_var = (r12_var + r14_var) & 0xFF;
        
        /* Access volatile memory with complex addressing */
        int64_t complex_idx = (r10_var ^ r11_var ^ r12_var) & 0x3;
        volatile BigVolStruct* volatile_ptr = &global_data[complex_idx];
        
        /* Force address computation for volatile access */
        asm volatile (
            "/* Volatile access with address computation */\n\t"
            "mov %[val], [%[ptr] + %[off]]"
            : [val] "=r" (temp_output)
            : [ptr] "r" (volatile_ptr),
              [off] "r" (idx3 * sizeof(VolStruct) * 2)
            : "memory"
        );
    }
    
    /* Final pattern: Multiple output operands with memory inputs */
    int64_t final_results[4];
    
    asm volatile (
        "/* Final complex pattern */\n\t"
        "mov %[out1], [%[mem1]]\n\t"
        "mov %[out2], [%[mem2]]\n\t"
        "add %[out3], %[out1], %[out2]\n\t"
        "mov %[out4], [%[mem3]]"
        : [out1] "=&r" (final_results[0]),
          [out2] "=&r" (final_results[1]),
          [out3] "=&r" (final_results[2]),
          [out4] "=&r" (final_results[3])
        : [mem1] "m" (global_array[r10_var & 0xFF]),
          [mem2] "m" (global_array[r11_var & 0xFF]),
          [mem3] "m" (global_array[r12_var & 0xFF])
        : "memory", "cc"
    );
}

/* Helper function with different calling convention pressure */
void __attribute__((noinline)) 
helper_func(volatile int64_t* ptr, int64_t idx1, int64_t idx2, int64_t idx3) {
    register int64_t rbx_var asm("rbx") = idx1;
    register int64_t rbp_var asm("rbp") = idx2;
    
    /* Complex addressing in helper */
    asm volatile (
        "/* Helper function complex addressing */\n\t"
        "mov %[out], [%[base] + %[idx1]*8 + %[idx2]*4]"
        : [out] "=r" (rbx_var)
        : [base] "r" (ptr),
          [idx1] "r" (idx1),
          [idx2] "r" (idx2)
        : "memory"
    );
    
    /* Use the result */
    asm volatile (
        "add %[out], %[in], %[val]"
        : [out] "=r" (rbp_var)
        : [in] "r" (rbp_var),
          [val] "m" (global_array[idx3 & 0xFF])
        : "cc"
    );
}

/* Main function that sets up and calls the trigger */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Call the main trigger function */
    trigger_reloads(iterations);
    
    /* Call helper with complex arguments */
    for (int i = 0; i < 10; i++) {
        helper_func((int64_t*)&global_array[0], 
                   (i * 7) & 0xFF, 
                   (i * 13) & 0xFF,
                   (i * 19) & 0xFF);
    }
    
    return 0;
}
