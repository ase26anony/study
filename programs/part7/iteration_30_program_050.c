/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misalignment */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr[100];
    long *ptr_arr[50];
    struct misaligned_data md[10];
    volatile int vol_var = 0;
    register int reg_var asm ("r12") = 0; /* Try to reserve a register */
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        ptr_arr[i] = (long*)&arr[i * 2];
    }
    for (int i = 0; i < 10; i++) {
        md[i].c = i;
        md[i].i = i * 100;
        md[i].l = i * 1000L;
    }
    
    long checksum = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx1 = (iter * 7) % 100;
        int idx2 = (iter * 13) % 100;
        int idx3 = (iter * 19) % 10;
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        /* Multiple output operands with early-clobber */
        asm volatile (
            "add %[out1], %[in1], %[in2]\n\t"
            "sub %[out2], %[in3], %[in4]\n\t"
            "mul %[out3], %[in5], %[in6]"
            : [out1] "=&r" (arr[idx1]),  /* Early-clobber output */
              [out2] "=&r" (arr[idx2]),  /* Early-clobber output */
              [out3] "=r" (vol_var)      /* Regular output */
            : [in1] "r" (arr[(idx1 + 1) % 100]),
              [in2] "r" (iter),
              [in3] "r" (arr[(idx2 + 2) % 100]),
              [in4] "r" (iter * 2),
              [in5] "r" (arr[(idx1 + 3) % 100]),
              [in6] "r" (iter * 3)
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_INPUT_ADDRESS with complex addressing */
        /* Memory operand with index, scale, and displacement */
        long complex_addr_result;
        asm volatile (
            "mov %[res], [%[base] + %[index]*4 + %[disp]]"
            : [res] "=r" (complex_addr_result)
            : [base] "r" (arr),
              [index] "r" (idx1),
              [disp] "r" (idx2 * sizeof(int))
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        /* Taking address of memory operand that needs reload */
        long *addr_ptr;
        asm volatile (
            "lea %[ptr], [%[base] + %[idx]*%[scale] + %[off]]\n\t"
            "mov %[val], [%[ptr]]"
            : [ptr] "=&r" (addr_ptr),    /* Early-clobber address register */
              [val] "=r" (md[idx3].l)    /* Output using that address */
            : [base] "r" (md),
              [idx] "r" (idx3),
              [scale] "i" (sizeof(struct misaligned_data)),
              [off] "i" (offsetof(struct misaligned_data, l))
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Nested address computation */
        long **nested_addr;
        asm volatile (
            "mov %[addr], %[base]\n\t"
            "add %[addr], %[offset]"
            : [addr] "=r" (nested_addr)
            : [base] "r" (ptr_arr),
              [offset] "r" (idx3 * sizeof(long*))
            : "cc"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
        /* Multiple memory constraints with clobbered registers */
        asm volatile (
            "mov r10, [%[mem1]]\n\t"
            "mov r11, [%[mem2]]\n\t"
            "add r10, r11\n\t"
            "mov [%[mem3]], r10\n\t"
            "mov r12, [%[mem4]]\n\t"
            "imul r12, %[imm]\n\t"
            "mov [%[mem5]], r12"
            : 
            : [mem1] "m" (arr[idx1]),
              [mem2] "m" (arr[idx2]),
              [mem3] "m" (arr[(idx1 + 5) % 100]),
              [mem4] "m" (md[idx3].i),
              [mem5] "m" (md[(idx3 + 1) % 10].i),
              [imm] "i" (iter + 1)
            : "r10", "r11", "r12", "cc", "memory"
        );
        
        /* Mix register classes to increase pressure */
        char char_var;
        short short_var;
        asm volatile (
            "mov %[c], %[cval]\n\t"
            "mov %[s], %[sval]"
            : [c] "=r" (char_var),
              [s] "=r" (short_var)
            : [cval] "i" (iter & 0xFF),
              [sval] "i" (iter * 100)
        );
        
        /* Update checksum to ensure side effects */
        checksum += arr[idx1] + arr[idx2] + complex_addr_result + md[idx3].l;
        global_counter++;
    }
    
    /* Final computation to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    for (int i = 0; i < 10; i++) {
        checksum += md[i].i + md[i].l;
    }
    
    global_sum = checksum;
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum > 0) ? 0 : 1;
}
