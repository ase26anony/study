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
volatile long global_sum = 0;

int main() {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var1 asm("r12") = 1;
    register int reg_var2 asm("r13") = 2;
    int auto_var1 = 3, auto_var2 = 4, auto_var3 = 5;
    volatile int vol_var = 6;
    long long_var1 = 7, long_var2 = 8;
    char char_array[64];
    int int_array[32];
    long long_array[16];
    
    /* Packed/misaligned data */
    struct misaligned_data packed_data = { 'A', 100, 1000L, 'Z' };
    struct misaligned_data *packed_ptr = &packed_data;
    
    /* Address-taken variables */
    int *ptr1 = &auto_var1;
    int *ptr2 = &auto_var2;
    long *lptr = &long_var1;
    
    /* Initialize arrays */
    for (int i = 0; i < sizeof(char_array); i++) char_array[i] = i;
    for (int i = 0; i < 32; i++) int_array[i] = i * 2;
    for (int i = 0; i < 16; i++) long_array[i] = i * 3;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary constraints based on iteration to stress different reload paths */
        int constraint_type = iteration % 5;
        
        switch (constraint_type) {
            case 0: {
                /* Case 1: Complex addressing modes with multiple memory operands
                   Triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
                asm volatile (
                    /* Complex addressing: [base + index*scale + displacement] */
                    "mov %[val1], %[idx]\n\t"
                    "add %[val2], %[arr1 + %[idx]*4 + 16]\n\t"
                    "lea %[ptr], [%[arr2] + %[idx]*8 + 32]\n\t"
                    : [val1] "+r" (auto_var1),
                      [ptr] "=r" (ptr1)
                    : [val2] "r" (auto_var2),
                      [idx] "r" (iteration),
                      [arr1] "r" (int_array),
                      [arr2] "r" (long_array)
                    : "memory", "cc"
                );
                break;
            }
            
            case 1: {
                /* Case 2: Multiple operands with conflicting constraints
                   Triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
                asm volatile (
                    /* Early-clobber outputs with overlapping inputs */
                    "add %[out1], %[in1]\n\t"
                    "imul %[out2], %[in2]\n\t"
                    "sub %[out3], %[in3]\n\t"
                    : [out1] "=&r" (auto_var1),   /* Early clobber */
                      [out2] "=&r" (auto_var2),   /* Early clobber */
                      [out3] "=r" (auto_var3)
                    : [in1] "r" (reg_var1),
                      [in2] "0" (auto_var1),      /* Ties to output 0 */
                      [in3] "r" (reg_var2),
                      "m" (vol_var)               /* Memory operand */
                    : "cc"
                );
                break;
            }
            
            case 2: {
                /* Case 3: Nested address computation
                   Triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
                long *temp_ptr;
                asm volatile (
                    /* Take address of memory operand with complex addressing */
                    "lea %[addr], [%[base] + %[idx]*2 + %[disp]]\n\t"
                    "mov (%[addr]), %[val]\n\t"
                    : [addr] "=r" (temp_ptr),
                      [val] "=r" (long_var1)
                    : [base] "r" (char_array),
                      [idx] "r" (iteration * 8),
                      [disp] "i" (16),
                      "m" (*packed_ptr)           /* Memory clobber */
                    : "memory"
                );
                
                /* Use the computed address */
                asm volatile (
                    "add %[sum], %[val]\n\t"
                    : [sum] "+r" (global_sum)
                    : [val] "r" (*temp_ptr)
                    : "cc"
                );
                break;
            }
            
            case 3: {
                /* Case 4: Output address reloads
                   Triggers: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
                long output_addr;
                asm volatile (
                    /* Complex output addressing */
                    "mov %[out], %[idx]\n\t"
                    "mov [%[arr] + %[out]*8], %[val]\n\t"
                    : [out] "=r" (output_addr),
                      "=m" (long_array[iteration % 8])  /* Memory output */
                    : [idx] "r" (iteration),
                      [arr] "r" (long_array),
                      [val] "r" (global_counter)
                    : "memory", "cc"
                );
                break;
            }
            
            case 4: {
                /* Case 5: Mixed types and other address reloads
                   Triggers: RELOAD_FOR_OTHER_ADDRESS, RELOAD_OTHER */
                char *char_ptr;
                int *int_ptr;
                
                asm volatile (
                    /* Multiple address computations with different types */
                    "lea %[cp], [%[base] + %[off1]]\n\t"
                    "lea %[ip], [%[base] + %[off2]*4]\n\t"
                    "movb (%[cp]), %%al\n\t"
                    "addl %%eax, (%[ip])\n\t"
                    : [cp] "=r" (char_ptr),
                      [ip] "=r" (int_ptr),
                      "+m" (int_array[iteration % 4])
                    : [base] "r" (char_array),
                      [off1] "r" (iteration),
                      [off2] "r" (iteration * 2)
                    : "memory", "cc", "rax"
                );
                break;
            }
        }
        
        /* Modify volatile variable to prevent dead code elimination */
        asm volatile (
            "addl $1, %[counter]\n\t"
            : [counter] "+m" (global_counter)
            :
            : "cc"
        );
        
        /* Vary register pressure by rotating values */
        int temp = reg_var1;
        reg_var1 = reg_var2;
        reg_var2 = auto_var1;
        auto_var1 = temp;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    long checksum = 0;
    checksum += auto_var1 + auto_var2 + auto_var3;
    checksum += reg_var1 + reg_var2;
    checksum += vol_var;
    checksum += long_var1 + long_var2;
    checksum += *ptr1 + *ptr2;
    checksum += packed_data.i + packed_data.l;
    checksum += global_counter + global_sum;
    
    for (int i = 0; i < 32; i++) checksum += int_array[i];
    for (int i = 0; i < 16; i++) checksum += long_array[i];
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum % 256);
}
