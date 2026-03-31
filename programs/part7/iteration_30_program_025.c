/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC's reload pass */
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
    /* Declare diverse variables with different storage characteristics */
    int auto_int = 42;
    volatile int vol_int = 100;
    long auto_long = 9999;
    char buffer[256];
    double fp_var = 3.14159;
    struct misaligned_data packed = {0};
    
    /* Pointers with different properties */
    int *ptr_int = &auto_int;
    volatile int *vol_ptr = &vol_int;
    char *ptr_char = buffer;
    long *ptr_long = &auto_long;
    
    /* Variables for address computations */
    int index_array[10] = {0,1,2,3,4,5,6,7,8,9};
    long displacement = 128;
    
    /* Loop to vary constraints and trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        /* Vary displacement and index to create different addressing modes */
        long base_disp = iter * 16;
        int scale_factor = (iter % 4) + 1;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Complex addressing mode that likely needs a temporary register */
        asm volatile (
            "mov %[val1], %[dest1]\n\t"
            "add %[val2], %[dest1]\n\t"
            : [dest1] "=r" (auto_int)
            : [val1] "m" (*(int*)((char*)index_array + base_disp + iter)),
              [val2] "r" (iter)
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output with complex addressing mode */
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[out]\n\t"
            "mov %[in], (%[out])\n\t"
            : [out] "=&r" (ptr_char), "=m" (*(int*)(buffer + iter * 4))
            : [base] "r" (buffer),
              [index] "r" ((long)iter * 4),
              [scale] "i" (1),
              [in] "r" (iter)
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Taking address of a memory operand with complex addressing */
        int *complex_addr;
        asm volatile (
            "mov %[addr], %[result]\n\t"
            : [result] "=r" (complex_addr)
            : [addr] "m" (ptr_int[iter * 2])
            : "cc"
        );
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT with register pressure */
        /* Many operands competing for registers */
        int r1, r2, r3, r4, r5, r6;
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "imul %[in2], %[out1]\n\t"
            "add %[in3], %[out2]\n\t"
            "sub %[in4], %[out3]\n\t"
            "xor %[in5], %[out4]\n\t"
            "or %[in6], %[out5]\n\t"
            : [out1] "=&r" (r1),
              [out2] "=&r" (r2),
              [out3] "=&r" (r3),
              [out4] "=&r" (r4),
              [out5] "=&r" (r5),
              [out6] "=r" (r6)
            : [in1] "r" (auto_int),
              [in2] "r" (vol_int),
              [in3] "m" (packed.i),
              [in4] "r" (iter),
              [in5] "m" (index_array[iter]),
              [in6] "r" (global_counter),
              "m" (*(int*)(buffer + iter)),  /* Extra memory input */
              "m" (*(long*)(&fp_var))        /* Another memory input */
            : "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Force RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        /* Volatile asm with memory clobber and many constraints */
        asm volatile (
            "movq (%[base], %[idx], 8), %%rax\n\t"
            "addq %%rax, %[sum]\n\t"
            "movq %[sum], (%[dest], %[idx2], 2)\n\t"
            : [sum] "+r" (global_sum)
            : [base] "r" (index_array),
              [idx] "r" ((long)iter),
              [dest] "r" (buffer),
              [idx2] "r" ((long)iter * 2),
              "m" (*(struct misaligned_data*)((char*)&packed + iter)),
              "m" (*(volatile int*)(vol_ptr + iter))
            : "rax", "cc", "memory"
        );
        
        /* Mix data types to increase pressure */
        if (iter % 3 == 0) {
            /* Use char operands */
            char char_result;
            asm volatile (
                "movb %[in1], %[out]\n\t"
                "addb %[in2], %[out]\n\t"
                : [out] "=r" (char_result)
                : [in1] "m" (buffer[iter]),
                  [in2] "i" (1)
                : "cc"
            );
            buffer[iter + 128] = char_result;
        }
        
        /* Update variables to create dependencies between iterations */
        auto_int += r1;
        vol_int = iter;
        packed.i = auto_int;
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations have observable effects */
    long checksum = 0;
    checksum += auto_int;
    checksum += vol_int;
    checksum += auto_long;
    checksum += (long)fp_var;
    checksum += global_sum;
    
    for (int i = 0; i < 256; i++) {
        checksum += buffer[i];
    }
    
    for (int i = 0; i < 10; i++) {
        checksum += index_array[i];
    }
    
    checksum += packed.i + packed.l;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global sum: %ld\n", global_sum);
    
    return (checksum != 0) ? 0 : 1;
}
