/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char tail;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main() {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("r12") = 12345;  /* Fixed register to create pressure */
    auto int auto_var = 67890;
    volatile int vol_var = 13579;
    static long static_arr[16];
    
    /* Misaligned struct */
    struct misaligned_data packed = {.c = 'A', .i = -42, .l = 999999999, .tail = 'Z'};
    
    /* Array with complex indexing */
    double double_arr[64];
    for (int i = 0; i < 64; i++) double_arr[i] = i * 3.14159;
    
    /* Pointers with different base types */
    char *char_ptr = (char *)&packed;
    int *int_ptr = &auto_var;
    long *long_ptr = &static_arr[0];
    
    /* Variable to accumulate checksum */
    unsigned long checksum = 0;
    
    /* Loop to create varying constraints and trigger different reload types */
    for (int iteration = 0; iteration < 8; iteration++) {
        /* Vary constraints based on iteration */
        int constraint_type = iteration % 4;
        
        switch (constraint_type) {
            case 0: {
                /* Complex addressing modes with multiple memory operands */
                /* Likely triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
                asm volatile (
                    /* Input with complex address: [base + index*8 + displacement] */
                    "movq %[arr], %%rax\n\t"
                    "movq %[idx], %%rbx\n\t"
                    "leaq (%%rax,%%rbx,8), %%rcx\n\t"
                    /* Another complex address computation */
                    "movq %[base], %%rdx\n\t"
                    "addq %[offset], %%rdx\n\t"
                    /* Use both computed addresses */
                    "movq (%%rcx), %%r8\n\t"
                    "addq (%%rdx), %%r8\n\t"
                    "movq %%r8, %[out]\n\t"
                    : [out] "=m" (static_arr[iteration % 4])
                    : [arr] "m" (double_arr),
                      [idx] "r" (iteration * 8),
                      [base] "r" (long_ptr),
                      [offset] "i" (iteration * sizeof(long))
                    : "rax", "rbx", "rcx", "rdx", "r8", "memory"
                );
                break;
            }
            
            case 1: {
                /* Many operands to create register pressure */
                /* Likely triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
                long temp1, temp2, temp3, temp4, temp5;
                asm volatile (
                    /* Multiple early-clobber outputs with overlapping inputs */
                    "mov %[in1], %[out1]\n\t"
                    "add %[in2], %[out1]\n\t"
                    "mov %[in3], %[out2]\n\t"
                    "imul %[in4], %[out2]\n\t"
                    "mov %[in5], %[out3]\n\t"
                    "sub %[out1], %[out3]\n\t"
                    "mov %[in6], %[out4]\n\t"
                    "xor %[out2], %[out4]\n\t"
                    "mov %[in7], %[out5]\n\t"
                    "or %[out3], %[out5]\n\t"
                    : [out1] "=&r" (temp1),
                      [out2] "=&r" (temp2),
                      [out3] "=&r" (temp3),
                      [out4] "=&r" (temp4),
                      [out5] "=&r" (temp5)
                    : [in1] "r" (reg_var),
                      [in2] "r" (auto_var),
                      [in3] "m" (vol_var),
                      [in4] "i" (iteration + 1),
                      [in5] "r" (static_arr[0]),
                      [in6] "m" (packed.i),
                      [in7] "r" (global_counter)
                    : "cc"
                );
                /* Use results to prevent elimination */
                checksum += temp1 + temp2 + temp3 + temp4 + temp5;
                break;
            }
            
            case 2: {
                /* Nested address computation */
                /* Likely triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
                long *addr_of_mem;
                asm volatile (
                    /* Compute address of a memory operand with complex addressing */
                    "lea %[complex_addr], %[addr]\n\t"
                    /* Then use that address */
                    "movq (%[addr]), %%rax\n\t"
                    "addq $1, %%rax\n\t"
                    "movq %%rax, (%[addr])\n\t"
                    : [addr] "=r" (addr_of_mem)
                    : [complex_addr] "m" (double_arr[iteration * 2 + 1])
                    : "rax", "memory"
                );
                /* Force use of computed address */
                global_array[iteration] = *addr_of_mem;
                break;
            }
            
            case 3: {
                /* Mixed data types and alignment issues */
                /* Likely triggers: RELOAD_FOR_OTHER_ADDRESS, various others */
                char char_result;
                int int_result;
                long long_result;
                
                asm volatile (
                    /* Operations on misaligned data */
                    "movb %[char_in], %%al\n\t"
                    "addb $32, %%al\n\t"
                    "movb %%al, %[char_out]\n\t"
                    /* Integer operation with memory constraint */
                    "movl %[int_in], %%ebx\n\t"
                    "imull %[scale], %%ebx\n\t"
                    "movl %%ebx, %[int_out]\n\t"
                    /* Long operation with complex address */
                    "movq %[ptr], %%rcx\n\t"
                    "movq (%%rcx,%[idx],8), %%rdx\n\t"
                    "addq %[addend], %%rdx\n\t"
                    "movq %%rdx, %[long_out]\n\t"
                    : [char_out] "=m" (char_result),
                      [int_out] "=r" (int_result),
                      [long_out] "=r" (long_result)
                    : [char_in] "m" (packed.c),
                      [int_in] "m" (packed.i),
                      [ptr] "r" (double_arr),
                      [idx] "r" (iteration),
                      [scale] "i" (3),
                      [addend] "i" (1000)
                    : "rax", "rbx", "rcx", "rdx", "memory"
                );
                
                /* Update checksum with all results */
                checksum += char_result + int_result + long_result;
                packed.c = char_result;  /* Modify struct */
                break;
            }
        }
        
        /* Modify variables to change constraints in next iteration */
        reg_var += iteration;
        auto_var *= 2;
        vol_var = iteration;
        global_counter++;
        
        /* Force memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Compute final checksum from all modified data */
    checksum += reg_var + auto_var + vol_var + global_counter;
    for (int i = 0; i < 16; i++) checksum += static_arr[i];
    for (int i = 0; i < 32; i++) checksum += global_array[i];
    checksum += packed.c + packed.i + packed.l + packed.tail;
    
    /* Print result to ensure all code has observable effect */
    printf("Final checksum: %lu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
