/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main() {
    /* Declare variables with different types and storage */
    register int reg_var asm("ebx") = 12345;  /* Hint register */
    volatile int vol_var = 67890;
    int auto_var = 13579;
    long long_var = 24680;
    char char_var = 'A';
    double double_var = 3.14159;
    
    /* Address-taken variables */
    int* ptr_var = &auto_var;
    long* long_ptr = &long_var;
    
    /* Packed/misaligned data */
    struct misaligned_data packed = {.c = 'X', .i = 999, .l = 777777};
    struct misaligned_data* packed_ptr = &packed;
    
    /* Array with complex indexing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Loop to vary constraints and trigger different reload types */
    unsigned long checksum = 0;
    
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary register pressure by using different constraints each iteration */
        switch (iteration % 4) {
            case 0: {
                /* Complex addressing modes - triggers RELOAD_FOR_INPUT_ADDRESS, 
                   RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
                asm volatile (
                    /* Output with complex addressing */
                    "mov %[out1], %[base1] + %[idx1]*4 + %[disp1] \n\t"
                    /* Input with nested address computation */
                    "add %[in1], %[base2] + %[idx2]*8 + %[disp2] \n\t"
                    /* Address of address computation */
                    "lea (%[base3], %[idx3], 4), %[addr1] \n\t"
                    : [out1] "=m" (array[iteration * 16 + 0]),
                      [addr1] "=r" (ptr_var)
                    : [in1] "r" (reg_var),
                      [base1] "r" (array), [idx1] "r" (iteration * 8), [disp1] "i" (16),
                      [base2] "r" (global_array), [idx2] "r" (iteration * 4), [disp2] "i" (32),
                      [base3] "r" (array), [idx3] "r" (iteration * 2)
                    : "memory", "cc"
                );
                break;
            }
            
            case 1: {
                /* Multiple operand constraints with early-clobber - 
                   triggers RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
                long temp1, temp2, temp3;
                asm volatile (
                    /* Early-clobber outputs with overlapping inputs */
                    "mov %[in2], %[out2] \n\t"
                    "add %[in3], %[out3] \n\t"
                    "imul %[in4], %[out4] \n\t"
                    : [out2] "=&r" (temp1),
                      [out3] "=&r" (temp2),
                      [out4] "=&r" (temp3)
                    : [in2] "r" (long_var),
                      [in3] "rm" (packed.i),  /* Mix register/memory constraint */
                      [in4] "rm" (iteration * 100)
                    : "cc"
                );
                checksum += temp1 + temp2 + temp3;
                break;
            }
            
            case 2: {
                /* Nested address computation - triggers RELOAD_FOR_OPERAND_ADDRESS,
                   RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
                int* addr_of_addr;
                long double_result;
                
                asm volatile (
                    /* Take address of memory operand with complex addressing */
                    "lea (%[mem_base], %[mem_idx], 4), %[addr_out] \n\t"
                    /* Use that address in another computation */
                    "mov (%[addr_out]), %[data_out] \n\t"
                    /* Address of the address itself */
                    "mov %[addr_out], %[addr_of_addr_out] \n\t"
                    : [addr_out] "=r" (ptr_var),
                      [data_out] "=r" (double_result),
                      [addr_of_addr_out] "=r" (addr_of_addr)
                    : [mem_base] "r" (array),
                      [mem_idx] "r" (iteration * 3 + 1)
                    : "memory"
                );
                
                /* Force use of computed addresses */
                asm volatile (
                    "addl $1, (%[ptr]) \n\t"
                    :
                    : [ptr] "r" (addr_of_addr)
                    : "memory"
                );
                break;
            }
            
            case 3: {
                /* Mixed data types and maximum register pressure - 
                   triggers all reload types through spill/reload */
                char c1, c2;
                short s1, s2;
                int i1, i2, i3, i4, i5, i6, i7, i8;
                long l1, l2, l3, l4;
                float f1, f2;
                double d1, d2;
                
                /* Force many simultaneous live values */
                asm volatile (
                    "mov %[in_c1], %[out_c1] \n\t"
                    "mov %[in_s1], %[out_s1] \n\t"
                    "mov %[in_i1], %[out_i1] \n\t"
                    "mov %[in_i2], %[out_i2] \n\t"
                    "mov %[in_i3], %[out_i3] \n\t"
                    "mov %[in_i4], %[out_i4] \n\t"
                    "mov %[in_i5], %[out_i5] \n\t"
                    "mov %[in_i6], %[out_i6] \n\t"
                    "mov %[in_l1], %[out_l1] \n\t"
                    "mov %[in_l2], %[out_l2] \n\t"
                    "mov %[in_f1], %[out_f1] \n\t"
                    "mov %[in_d1], %[out_d1] \n\t"
                    : [out_c1] "=r" (c1),
                      [out_s1] "=r" (s1),
                      [out_i1] "=r" (i1), [out_i2] "=r" (i2),
                      [out_i3] "=r" (i3), [out_i4] "=r" (i4),
                      [out_i5] "=r" (i5), [out_i6] "=r" (i6),
                      [out_l1] "=r" (l1), [out_l2] "=r" (l2),
                      [out_f1] "=r" (f1), [out_d1] "=r" (d1)
                    : [in_c1] "r" (char_var),
                      [in_s1] "r" ((short)iteration),
                      [in_i1] "r" (auto_var), [in_i2] "r" (vol_var),
                      [in_i3] "r" (reg_var), [in_i4] "r" (global_counter),
                      [in_i5] "r" (iteration * 2), [in_i6] "r" (iteration * 3),
                      [in_l1] "r" (long_var), [in_l2] "r" (packed.l),
                      [in_f1] "r" ((float)double_var),
                      [in_d1] "r" (double_var)
                    : "memory"
                );
                
                checksum += c1 + s1 + i1 + i2 + i3 + i4 + i5 + i6 + l1 + l2;
                break;
            }
        }
        
        /* Modify variables to create dependencies between iterations */
        auto_var += iteration;
        long_var ^= iteration * 0x1234;
        char_var += 1;
        double_var *= 1.1;
        global_counter++;
        
        /* Force memory clobber to prevent optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Compute final checksum to ensure all operations have side effects */
    for (int i = 0; i < 256; i++) {
        checksum += array[i];
    }
    checksum += auto_var + long_var + char_var + (int)double_var;
    checksum += packed.i + packed.l;
    checksum += *ptr_var + global_counter;
    
    printf("Checksum: %lu\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum > 1000000) {
        printf("Reload test completed successfully.\n");
    }
    
    return 0;
}
