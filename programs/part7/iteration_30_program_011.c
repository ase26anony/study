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
volatile long global_counter = 0;
volatile int global_index = 0;

int main(void) {
    /* Declare variables with different types and storage */
    register int reg_var1 asm("r12") = 1;
    register int reg_var2 asm("r13") = 2;
    int auto_var1 = 100, auto_var2 = 200, auto_var3 = 300;
    volatile int vol_var = 999;
    long long_var1 = 1000, long_var2 = 2000;
    char char_array[256];
    int int_array[128];
    long long_array[64];
    
    /* Packed/misaligned data */
    struct misaligned_data packed1, packed2;
    packed1.i = 0x12345678;
    packed2.i = 0x9ABCDEF0;
    
    /* Pointers with different properties */
    int *ptr1 = &auto_var1;
    volatile int *vol_ptr = &vol_var;
    char *char_ptr = char_array;
    long *long_ptr = long_array;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) char_array[i] = i;
    for (int i = 0; i < 128; i++) int_array[i] = i * 2;
    for (int i = 0; i < 64; i++) long_array[i] = i * 100;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Change constraints each iteration */
        int constraint_mod = iteration % 4;
        
        switch (constraint_mod) {
            case 0:
                /* Complex addressing modes - triggers RELOAD_FOR_INPUT_ADDRESS,
                   RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
                asm volatile (
                    /* Multiple memory operands with complex addressing */
                    "mov %[out1], [%[base1] + %[idx1]*4 + %[disp1]]\n\t"
                    "mov [%[base2] + %[idx2]*8 + %[disp2]], %[in1]\n\t"
                    "lea %[out2], [%[base3] + %[idx3]*2 + %[disp3]]\n\t"
                    : [out1] "=r" (auto_var1),
                      [out2] "=r" (auto_var2)
                    : [in1] "r" (auto_var3),
                      [base1] "r" (long_array),
                      [idx1] "r" (iteration * 2),
                      [disp1] "i" (16),
                      [base2] "r" (int_array),
                      [idx2] "r" (iteration * 3),
                      [disp2] "i" (32),
                      [base3] "r" (char_array),
                      [idx3] "r" (iteration * 4),
                      [disp3] "i" (64)
                    : "memory"
                );
                break;
                
            case 1:
                /* Many operands causing register pressure - triggers RELOAD_FOR_INPUT,
                   RELOAD_FOR_OUTPUT, RELOAD_OTHER */
                asm volatile (
                    /* Early-clobber outputs with overlapping inputs */
                    "add %[out1], %[in1], %[in2]\n\t"
                    "sub %[out2], %[in3], %[in4]\n\t"
                    "mul %[out3], %[in5], %[in6]\n\t"
                    "and %[out4], %[in7], %[in8]\n\t"
                    : [out1] "=&r" (auto_var1),
                      [out2] "=&r" (auto_var2),
                      [out3] "=&r" (auto_var3),
                      [out4] "=&r" (reg_var1)
                    : [in1] "r" (auto_var1),
                      [in2] "r" (iteration),
                      [in3] "r" (auto_var2),
                      [in4] "r" (global_counter),
                      [in5] "r" (auto_var3),
                      [in6] "r" (long_var1),
                      [in7] "r" (reg_var1),
                      [in8] "r" (reg_var2)
                    : "cc", "memory"
                );
                break;
                
            case 2:
                /* Nested address computation - triggers RELOAD_FOR_OPERAND_ADDRESS,
                   RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
                {
                    long *nested_ptr;
                    asm volatile (
                        /* Take address of memory operand with complex addressing */
                        "lea %[addr], [%[base] + %[idx]*%[scale] + %[disp]]\n\t"
                        "mov %[val], [%[addr]]\n\t"
                        : [addr] "=r" (nested_ptr),
                          [val] "=r" (long_var1)
                        : [base] "r" (long_array),
                          [idx] "r" (iteration * 5),
                          [scale] "i" (sizeof(long)),
                          [disp] "i" (offsetof(struct misaligned_data, l)),
                          "m" (*long_array)
                        : "memory"
                    );
                    
                    /* Use the computed address */
                    asm volatile (
                        "add %[out], [%[addr]], %[inc]\n\t"
                        : [out] "=r" (long_var2)
                        : [addr] "r" (nested_ptr),
                          [inc] "r" (iteration),
                          "m" (*nested_ptr)
                        : "cc", "memory"
                    );
                }
                break;
                
            case 3:
                /* Mixed data types and misaligned accesses */
                asm volatile (
                    /* Operands with different sizes and alignments */
                    "movsx %[out1], byte ptr [%[charptr] + %[off1]]\n\t"
                    "mov %[out2], dword ptr [%[misaligned]]\n\t"
                    "mov %[out3], qword ptr [%[longptr] + %[off2]*8]\n\t"
                    : [out1] "=r" (auto_var1),
                      [out2] "=r" (packed1.i),
                      [out3] "=r" (long_var1)
                    : [charptr] "r" (char_array),
                      [off1] "r" (iteration * 7),
                      [misaligned] "r" (&packed2.c + 1),  /* Misaligned int access */
                      [longptr] "r" (long_array),
                      [off2] "r" (iteration * 3),
                      "m" (char_array[0]),
                      "m" (packed2),
                      "m" (long_array[0])
                    : "memory"
                );
                break;
        }
        
        /* Force spill/reload between iterations */
        global_counter += auto_var1 + auto_var2 + auto_var3 + reg_var1 + long_var1;
        
        /* Address-taken operations */
        int *ptr_array[] = {&auto_var1, &auto_var2, &auto_var3};
        asm volatile (
            "mov %[out], [%[ptr] + %[idx]*4]\n\t"
            : [out] "=r" (vol_var)
            : [ptr] "r" (ptr_array),
              [idx] "r" (iteration % 3),
              "m" (ptr_array[0])
            : "memory"
        );
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = 0;
    checksum += auto_var1 + auto_var2 + auto_var3;
    checksum += reg_var1 + reg_var2;
    checksum += long_var1 + long_var2;
    checksum += packed1.i + packed2.i;
    checksum += global_counter;
    
    /* Use all variables to prevent dead code elimination */
    asm volatile (
        "add %[sum], %[sum], %[val1]\n\t"
        "add %[sum], %[sum], %[val2]\n\t"
        : [sum] "+r" (checksum)
        : [val1] "r" (vol_var),
          [val2] "r" (char_array[0])
        : "cc"
    );
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
