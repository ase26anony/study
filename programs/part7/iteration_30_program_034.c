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
    int arr[256];
    long *ptr_arr[16];
    char buffer[128];
    volatile int vol_var = 42;
    struct misaligned_data packed;
    register int reg_var asm ("r12") = 100; /* Suggest register but don't force */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 16; i++) ptr_arr[i] = (long*)&arr[i * 4];
    for (int i = 0; i < 128; i++) buffer[i] = i;
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7;
        long temp_sum = 0;
        
        /* VARYING CONSTRAINT 1: Complex addressing with multiple index registers */
        /* Likely triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            /* Complex addressing: [base + index*scale + displacement] */
            "mov %[val1], [%[base] + %[idx1]*4 + 16]\n\t"
            "add %[sum], %[val1]\n\t"
            /* Nested address computation */
            "lea %[addr], [%[base] + %[idx2]*8 + 32]\n\t"
            "mov %[val2], [%[addr]]\n\t"
            "add %[sum], %[val2]"
            : [sum] "+&r" (temp_sum), [addr] "=&r" (reg_var)
            : [base] "r" (arr), [idx1] "r" (idx % 64), 
              [idx2] "r" ((idx + 3) % 64), [val1] "r" (vol_var),
              [val2] "r" (vol_var)
            : "memory", "cc"
        );
        
        /* VARYING CONSTRAINT 2: Many operands exceeding available registers */
        /* Likely triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        {
            int out1, out2, out3, out4;
            int in1 = iter, in2 = iter*2, in3 = iter*3, in4 = iter*4;
            int in5 = iter*5, in6 = iter*6, in7 = iter*7, in8 = iter*8;
            
            asm volatile (
                "add %[o1], %[i1], %[i2]\n\t"
                "sub %[o2], %[i3], %[i4]\n\t"
                "mul %[o3], %[i5], %[i6]\n\t"
                "and %[o4], %[i7], %[i8]"
                : [o1] "=&r" (out1), [o2] "=&r" (out2),
                  [o3] "=&r" (out3), [o4] "=&r" (out4)
                : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
                  [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
                  [i7] "r" (in7), [i8] "r" (in8)
                : "memory", "cc"
            );
            
            temp_sum += out1 + out2 + out3 + out4;
        }
        
        /* VARYING CONSTRAINT 3: Address of address computation */
        /* Likely triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        {
            long* addr_ptr;
            long value;
            
            /* Take address of a complex memory operand */
            asm volatile (
                "lea %[aptr], [%[base] + %[idx]*4 + %[disp]]\n\t"
                "mov %[val], [%[aptr]]"
                : [aptr] "=&r" (addr_ptr), [val] "=r" (value)
                : [base] "r" (arr), [idx] "r" (idx % 32),
                  [disp] "r" (iter * 8)
                : "memory"
            );
            
            temp_sum += value;
            
            /* Use the address pointer in another operation */
            asm volatile (
                "add %[val], [%[aptr] + 8]"
                : [val] "+r" (value)
                : [aptr] "r" (addr_ptr)
                : "memory"
            );
            
            temp_sum += value;
        }
        
        /* VARYING CONSTRAINT 4: Mixed size operands with alignment issues */
        /* Likely triggers: RELOAD_FOR_OTHER_ADDRESS */
        {
            char* misaligned_ptr = (char*)&packed.i;
            int int_val;
            long long_val;
            
            /* Force misaligned access */
            asm volatile (
                "mov %[ival], [%[mptr]]\n\t"
                "mov %[lval], [%[mptr] + 1]"
                : [ival] "=r" (int_val), [lval] "=r" (long_val)
                : [mptr] "r" (misaligned_ptr)
                : "memory"
            );
            
            temp_sum += int_val + (long_val & 0xFFFFFFFF);
        }
        
        /* VARYING CONSTRAINT 5: Output address reloads */
        /* Likely triggers: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        {
            long* output_ptr = &arr[iter * 3];
            long input_val = iter * 1000;
            
            /* Complex address in output operand */
            asm volatile (
                "mov [%[out] + %[offset]], %[in]"
                : 
                : [out] "r" (output_ptr), [offset] "r" (iter * 4),
                  [in] "r" (input_val)
                : "memory"
            );
            
            /* Read it back with different addressing */
            asm volatile (
                "mov %[sum], [%[out] + %[offset]]"
                : [sum] "+r" (temp_sum)
                : [out] "r" (output_ptr), [offset] "r" (iter * 4)
                : "memory"
            );
        }
        
        global_sum += temp_sum;
        global_counter++;
    }
    
    /* Additional test: Nested loops with varying pressure */
    for (int outer = 0; outer < 5; outer++) {
        for (int inner = 0; inner < 20; inner++) {
            int idx = outer * 20 + inner;
            
            /* Mix register and memory constraints */
            asm volatile (
                "imul %[reg], %[idx], 37\n\t"
                "add [%[mem]], %[reg]"
                : [reg] "=&r" (reg_var)
                : [idx] "r" (idx), [mem] "r" (&arr[idx % 256])
                : "memory", "cc"
            );
        }
    }
    
    /* Compute final checksum to ensure all operations have effect */
    long final_checksum = global_sum;
    for (int i = 0; i < 256; i++) {
        final_checksum += arr[i];
    }
    final_checksum += packed.i + (packed.l & 0xFFFFFFFF);
    
    printf("Checksum: %ld\n", final_checksum);
    printf("Iterations: %d\n", global_counter);
    
    return (final_checksum != 0) ? 0 : 1;
}
