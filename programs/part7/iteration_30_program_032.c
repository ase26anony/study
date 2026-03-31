/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC */
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

int main() {
    /* Declare variables with different types and storage */
    int arr[100];
    long *ptr_arr[50];
    struct misaligned_data packed[10];
    volatile int vol_var = 0;
    register int reg_var asm ("r12") = 0; /* Try to fix a register */
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        ptr_arr[i] = (long*)&arr[i * 2];
    }
    
    /* Main loop with varying constraints */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 100;
        long *base_ptr = &arr[0];
        long scale = iter + 1;
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT with register pressure */
        asm volatile (
            /* Multiple output operands with early-clobber */
            "mov %[out1], %[in1] \n\t"
            "add %[out2], %[in2], %[in3] \n\t"
            "mul %[out3], %[in4], %[scale] \n\t"
            : [out1] "=&r" (arr[idx]),      /* Early-clobber output */
              [out2] "=&r" (arr[idx + 1]),   /* Early-clobber output */
              [out3] "=r" (arr[idx + 2])     /* Regular output */
            : [in1] "r" (arr[idx + 10]),     /* Input in register */
              [in2] "r" (arr[idx + 11]),     /* Input in register */
              [in3] "r" (arr[idx + 12]),     /* Input in register */
              [in4] "r" (arr[idx + 13]),     /* Input in register */
              [scale] "r" (scale)            /* Input in register */
            : "memory", "cc"
        );
        
        /* Complex addressing modes - triggers RELOAD_FOR_INPUT_ADDRESS */
        long complex_addr_result;
        asm volatile (
            "ldr %[result], [%[base], %[index], lsl #2] \n\t"
            : [result] "=r" (complex_addr_result)
            : [base] "r" (base_ptr),
              [index] "r" (idx)
            : "memory"
        );
        
        /* Nested address computation - triggers RELOAD_FOR_OPERAND_ADDRESS */
        long *addr_of_mem;
        asm volatile (
            "adr %[addr], [%[base], %[offset]] \n\t"
            : [addr] "=r" (addr_of_mem)
            : [base] "r" (base_ptr),
              [offset] "r" (idx * sizeof(int))
            : /* No clobbers */
        );
        
        /* Output with address reload - triggers RELOAD_FOR_OUTPUT_ADDRESS */
        long output_val = iter * 100;
        asm volatile (
            "str %[val], [%[addr], %[offset]] \n\t"
            : 
            : [val] "r" (output_val),
              [addr] "r" (base_ptr),
              [offset] "r" (idx * 2 * sizeof(int))
            : "memory"
        );
        
        /* Mixed data types with packed struct - triggers various reloads */
        struct misaligned_data *packed_ptr = &packed[iter % 10];
        int struct_field;
        asm volatile (
            "ldrb %[field], [%[ptr], #1] \n\t"
            "add %[field], %[field], #1 \n\t"
            "strb %[field], [%[ptr], #1] \n\t"
            : [field] "=&r" (struct_field)
            : [ptr] "r" (packed_ptr)
            : "memory"
        );
        
        /* Multiple memory operands with different constraints */
        long temp1, temp2, temp3;
        asm volatile (
            "ldr %[t1], [%[mem1]] \n\t"
            "ldr %[t2], [%[mem2], %[idx], lsl #3] \n\t"
            "add %[t3], %[t1], %[t2] \n\t"
            "str %[t3], [%[mem3]] \n\t"
            : [t1] "=&r" (temp1),
              [t2] "=&r" (temp2),
              [t3] "=r" (temp3)
            : [mem1] "r" (&arr[0]),
              [mem2] "r" (&arr[20]),
              [idx] "r" (iter),
              [mem3] "r" (&arr[30])
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        long *indirect_ptr = &arr[40];
        long indirect_val;
        asm volatile (
            "mov %[ptr], %[addr] \n\t"
            "ldr %[val], [%[ptr]] \n\t"
            "add %[val], %[val], #1 \n\t"
            "str %[val], [%[ptr]] \n\t"
            : [ptr] "=&r" (indirect_ptr),
              [val] "=&r" (indirect_val)
            : [addr] "r" (&arr[40 + iter])
            : "memory"
        );
        
        /* RELOAD_FOR_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        /* Use many operands to create register pressure */
        asm volatile (
            "add %[a], %[b], %[c] \n\t"
            "sub %[d], %[e], %[f] \n\t"
            "mul %[g], %[h], %[i] \n\t"
            "and %[j], %[k], %[l] \n\t"
            : [a] "=r" (arr[50]),
              [d] "=r" (arr[51]),
              [g] "=r" (arr[52]),
              [j] "=r" (arr[53])
            : [b] "r" (arr[60]),
              [c] "r" (arr[61]),
              [e] "r" (arr[62]),
              [f] "r" (arr[63]),
              [h] "r" (arr[64]),
              [i] "r" (arr[65]),
              [k] "r" (arr[66]),
              [l] "r" (arr[67])
            : "memory", "cc"
        );
        
        /* Update volatile to prevent dead code elimination */
        vol_var += iter;
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations have effect */
    long checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    for (int i = 0; i < 10; i++) {
        checksum += packed[i].c + packed[i].i;
    }
    
    checksum += vol_var + global_counter;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum > 0) ? 0 : 1;
}
