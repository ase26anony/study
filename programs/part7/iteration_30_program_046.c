/* reload_coverage.c */
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
volatile long global_base = 1000;
volatile int global_index = 2;
volatile char global_char = 'A';

int main() {
    /* Declare variables with different types and storage */
    register int reg_var asm("ebx") = 123;  /* Try to fix a register */
    volatile int vol_int = 456;
    long long_array[10] = {0};
    char char_buffer[64];
    double double_arr[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
    struct misaligned_data packed = {0};
    
    /* Address-taken variables */
    int* ptr_int = &vol_int;
    long* ptr_long = &long_array[3];
    char* ptr_char = &char_buffer[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        long_array[i] = i * 100 + 1;
    }
    for (int i = 0; i < 64; i++) {
        char_buffer[i] = '0' + (i % 10);
    }
    packed.i = 0x12345678;
    packed.l = 0x9876543210ABCDEFLL;
    
    /* Loop with varying constraints to trigger different reload types */
    unsigned long checksum = 0;
    
    for (int iteration = 0; iteration < 5; iteration++) {
        switch (iteration) {
            case 0:
                /* Complex addressing modes - RELOAD_FOR_INPUT_ADDRESS */
                asm volatile (
                    "mov %[src], %[dst]\n\t"
                    : [dst] "=m" (long_array[2])
                    : [src] "m" (*(long*)((char*)long_array + 8 * global_index + 16))
                    : "memory"
                );
                break;
                
            case 1:
                /* Multiple operands with early clobber - RELOAD_FOR_INPUT/OUTPUT */
                asm volatile (
                    "add %[in1], %[out1]\n\t"
                    "imul %[in2], %[out2]\n\t"
                    : [out1] "=&r" (reg_var), [out2] "=&r" (vol_int)
                    : [in1] "r" (global_base), [in2] "rm" (long_array[1]),
                      "0" (reg_var), "1" (vol_int)
                    : "cc"
                );
                break;
                
            case 2:
                /* Nested address computation - RELOAD_FOR_OPERAND_ADDRESS */
                {
                    long* complex_ptr = &long_array[global_index * 2 + 1];
                    asm volatile (
                        "lea (%[base], %[index], 8), %[temp]\n\t"
                        "mov (%[temp]), %[temp]\n\t"
                        "add %[temp], %[result]\n\t"
                        : [result] "+r" (reg_var), [temp] "=&r" (vol_int)
                        : [base] "r" (long_array), [index] "r" (global_index)
                        : "cc"
                    );
                }
                break;
                
            case 3:
                /* Output address reload - RELOAD_FOR_OUTPUT_ADDRESS */
                asm volatile (
                    "mov %[val], (%[dest], %[idx], 8)\n\t"
                    : 
                    : [val] "r" (global_base + iteration),
                      [dest] "r" (long_array),
                      [idx] "r" (global_index)
                    : "memory"
                );
                break;
                
            case 4:
                /* Mixed types and maximum register pressure - RELOAD_OTHER */
                asm volatile (
                    "mov %[ch], %%al\n\t"
                    "add %%al, %[sum8]\n\t"
                    "add %[int1], %[sum32]\n\t"
                    "add %[long1], %[sum64]\n\t"
                    : [sum8] "+r" (packed.c),
                      [sum32] "+r" (packed.i),
                      [sum64] "+r" (packed.l)
                    : [ch] "m" (char_buffer[global_index]),
                      [int1] "rm" (vol_int),
                      [long1] "rm" (long_array[0])
                    : "al", "cc", "memory"
                );
                break;
        }
        
        /* Force address computation for next iteration */
        global_index = (global_index + 1) % 5;
    }
    
    /* Additional test: Inpaddr address reload */
    {
        int dummy;
        /* RELOAD_FOR_INPADDR_ADDRESS - address of an input address */
        asm volatile (
            "mov %[addr], %[tmp]\n\t"
            "mov (%[tmp]), %[out]\n\t"
            : [out] "=r" (dummy), [tmp] "=&r" (vol_int)
            : [addr] "m" (ptr_long)
            : "memory"
        );
    }
    
    /* Additional test: Outaddr address reload */
    {
        long* output_ptr = &long_array[7];
        /* RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "mov %[val], %[tmp]\n\t"
            "mov %[tmp], (%[addr])\n\t"
            : 
            : [val] "r" (global_base), [addr] "m" (output_ptr),
              [tmp] "r" (vol_int)
            : "memory"
        );
    }
    
    /* Additional test: Opaddr addr reload - RELOAD_FOR_OPADDR_ADDR */
    {
        long temp;
        asm volatile (
            "mov %[complex], %[temp]\n\t"
            : [temp] "=r" (temp)
            : [complex] "m" (*(long*)((uintptr_t)long_array + (global_index << 3) + 8))
            : 
        );
    }
    
    /* Additional test: Other address reload - RELOAD_FOR_OTHER_ADDRESS */
    {
        struct misaligned_data* mis_ptr = &packed;
        asm volatile (
            "mov %[ptr], %%rax\n\t"
            "mov 1(%%rax), %%ebx\n\t"
            "add %%ebx, %[result]\n\t"
            : [result] "+r" (reg_var)
            : [ptr] "m" (mis_ptr)
            : "rax", "rbx", "cc", "memory"
        );
    }
    
    /* Compute checksum to ensure all asm statements executed */
    checksum = reg_var + vol_int + packed.c + packed.i + (packed.l & 0xFFFFFFFF);
    for (int i = 0; i < 10; i++) {
        checksum += long_array[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum % 256);
}
