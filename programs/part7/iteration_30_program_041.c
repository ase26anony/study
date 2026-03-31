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

int main() {
    /* Declare variables with different types and storage */
    int arr[100];
    long *ptr_arr[50];
    struct misaligned_data packed;
    volatile int vol_var = 1;
    register int reg_var asm("r12") = 2; /* Suggest register but don't force */
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
        if (i < 50) ptr_arr[i] = (long*)&arr[i];
    }
    packed.c = 'A';
    packed.i = 0x12345678;
    packed.l = 0x9876543210ABCDEFLL;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx1 = (iter * 7) % 100;
        int idx2 = (iter * 13) % 100;
        int idx3 = (iter * 19) % 100;
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        /* Multiple output operands with early-clobber */
        asm volatile (
            "add %[out1], %[in1], %[in2]\n\t"
            "sub %[out2], %[in3], %[in4]\n\t"
            "mul %[out3], %[in5], %[in6]"
            : [out1] "=&r" (arr[idx1]),  /* Early-clobber output */
              [out2] "=&r" (arr[idx2]),  /* Another early-clobber */
              [out3] "=r" (arr[idx3])    /* Regular output */
            : [in1] "r" (arr[(idx1 + 1) % 100]),
              [in2] "r" (iter),
              [in3] "r" (arr[(idx2 + 2) % 100]),
              [in4] "r" (vol_var),
              [in5] "r" (arr[(idx3 + 3) % 100]),
              [in6] "r" (reg_var)
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_INPUT_ADDRESS with complex addressing */
        long complex_addr_result;
        asm volatile (
            "ldr %[res], [%[base], %[index], lsl #2]\n\t"
            "add %[res], %[res], %[offset]"
            : [res] "=r" (complex_addr_result)
            : [base] "r" (&arr[0]),
              [index] "r" (idx1 * 2),
              [offset] "r" (iter * 100)
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS - address of input address */
        long* addr_of_addr;
        asm volatile (
            "mov %[out], %[in]\n\t"
            "add %[out], %[out], #8"
            : [out] "=r" (addr_of_addr)
            : [in] "r" (&ptr_arr[idx1 % 50])
            : "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        long output_addr_store;
        long* output_addr_ptr = &output_addr_store;
        asm volatile (
            "str %[val], [%[addr]]\n\t"
            "add %[addr], %[addr], #4"
            : [addr] "+&r" (output_addr_ptr)  /* Early-clobber output address */
            : [val] "r" (complex_addr_result)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OUTADDR_ADDRESS - address of output address */
        long** outaddr_ptr = &output_addr_ptr;
        asm volatile (
            "ldr %[tmp], [%[ptr]]\n\t"
            "add %[tmp], %[tmp], #1\n\t"
            "str %[tmp], [%[ptr]]"
            : [tmp] "=&r" (vol_var)  /* Temporary clobbered */
            : [ptr] "r" (outaddr_ptr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS with nested addressing */
        struct misaligned_data* packed_ptr = &packed;
        long nested_addr_result;
        asm volatile (
            "ldrb %[res], [%[ptr], #1]\n\t"   /* Access misaligned field */
            "sxtb %[res], %[res]"
            : [res] "=r" (nested_addr_result)
            : [ptr] "r" (packed_ptr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPADDR_ADDR - address of operand address */
        char** opaddr;
        asm volatile (
            "add %[out], %[in], %[offset]"
            : [out] "=r" (opaddr)
            : [in] "r" (&packed_ptr),
              [offset] "r" (iter * 4)
            : "cc"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with memory clobber */
        asm volatile (
            "dmb sy\n\t"
            "isb"
            : 
            : "r" (arr), "r" (ptr_arr), "r" (&packed)
            : "memory", "cc"
        );
        
        /* Force RELOAD_OTHER with many constraints */
        int temp1, temp2, temp3;
        asm volatile (
            "mov %[t1], %[a1]\n\t"
            "mov %[t2], %[a2]\n\t"
            "mov %[t3], %[a3]"
            : [t1] "=r" (temp1),
              [t2] "=r" (temp2),
              [t3] "=r" (temp3)
            : [a1] "r" (arr[idx1]),
              [a2] "r" (arr[idx2]),
              [a3] "r" (arr[idx3]),
              "m" (*ptr_arr[idx1 % 50]),  /* Memory constraint */
              "m" (packed)                /* Another memory constraint */
            : "cc"
        );
        
        /* Update globals to prevent dead code elimination */
        global_counter++;
        global_sum += complex_addr_result + nested_addr_result + temp1 + temp2 + temp3;
    }
    
    /* Compute checksum to ensure all operations have effect */
    long checksum = global_sum;
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    checksum += packed.i + packed.l;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum > 0) ? 0 : 1;
}
