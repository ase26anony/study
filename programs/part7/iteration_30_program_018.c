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
    /* Diverse variables with different storage characteristics */
    register int reg_var1 asm("r12") = 1;
    register int reg_var2 asm("r13") = 2;
    int auto_var1 = 3, auto_var2 = 4, auto_var3 = 5;
    volatile int vol_var = 6;
    int *ptr1 = &auto_var1;
    int *ptr2 = &auto_var2;
    long double fp_var = 7.0;
    
    /* Array with complex indexing */
    int array[256];
    for (int i = 0; i < 256; i++) array[i] = i * 3;
    
    /* Packed/misaligned struct */
    struct misaligned_data packed = {0};
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE;
    
    /* Address-taken variables */
    int addr_taken1 = 100, addr_taken2 = 200;
    int *addr_ptr1 = &addr_taken1;
    int **addr_ptr2 = &addr_ptr1;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        int idx = iteration * 7 % 256;
        int scale = 1 + (iteration % 4);
        
        switch (iteration % 5) {
            case 0: {
                /* Complex addressing modes - RELOAD_FOR_INPUT_ADDRESS */
                asm volatile (
                    "mov %[val1], %[idx]\n\t"
                    "add %[val2], %[base], %[idx], lsl #%[scale]\n\t"
                    : [val1] "=r" (auto_var1), [val2] "=r" (auto_var2)
                    : [base] "r" (array), [idx] "r" (idx), [scale] "I" (scale)
                    : "memory", "cc"
                );
                break;
            }
            
            case 1: {
                /* Multiple operands with early clobber - RELOAD_FOR_INPUT/OUTPUT */
                asm volatile (
                    "ldr %[out1], [%[in1], %[in2], lsl #2]\n\t"
                    "str %[in3], [%[out2], %[in4], lsl #2]\n\t"
                    : [out1] "=&r" (auto_var1), [out2] "=&r" (auto_var2)
                    : [in1] "r" (array), [in2] "r" (idx),
                      [in3] "r" (auto_var3), [in4] "r" (idx + 1),
                      "m" (array[0]), "m" (array[255])
                    : "memory"
                );
                break;
            }
            
            case 2: {
                /* Nested address computation - RELOAD_FOR_OPERAND_ADDRESS */
                long *nested_ptr = &array[idx];
                asm volatile (
                    "mov %[tmp], %[ptr]\n\t"
                    "ldr %[val], [%[tmp], %[offset]]\n\t"
                    : [val] "=r" (auto_var1), [tmp] "=&r" (auto_var2)
                    : [ptr] "r" (nested_ptr), [offset] "r" (sizeof(int) * scale),
                      "m" (*nested_ptr)
                    : "memory"
                );
                break;
            }
            
            case 3: {
                /* Mixed data types and alignment - RELOAD_FOR_OTHER_ADDRESS */
                char *char_ptr = (char *)&packed.i;
                asm volatile (
                    "ldrb %[c1], [%[p1], #1]\n\t"
                    "ldr %[i1], [%[p1], #0]\n\t"
                    "add %[p2], %[p1], %[off]\n\t"
                    : [c1] "=r" (auto_var1), [i1] "=r" (auto_var2),
                      [p2] "=&r" (ptr1)
                    : [p1] "r" (char_ptr), [off] "r" (sizeof(int)),
                      "m" (packed), "m" (packed.i)
                    : "memory"
                );
                break;
            }
            
            case 4: {
                /* Maximum register pressure - RELOAD_OTHER */
                asm volatile (
                    "mov %[r1], %[a1]\n\t"
                    "add %[r2], %[a2], %[a3]\n\t"
                    "mul %[r3], %[a4], %[a5]\n\t"
                    "and %[r4], %[a6], %[a7]\n\t"
                    "orr %[r5], %[a8], %[a9]\n\t"
                    : [r1] "=&r" (reg_var1), [r2] "=&r" (reg_var2),
                      [r3] "=&r" (auto_var1), [r4] "=&r" (auto_var2),
                      [r5] "=&r" (auto_var3)
                    : [a1] "r" (iteration), [a2] "r" (idx),
                      [a3] "r" (scale), [a4] "r" (auto_var1),
                      [a5] "r" (auto_var2), [a6] "r" (addr_taken1),
                      [a7] "r" (addr_taken2), [a8] "r" (ptr1),
                      [a9] "r" (ptr2)
                    : "memory", "cc"
                );
                break;
            }
        }
        
        /* Force address-of-address reloads (RELOAD_FOR_INPADDR_ADDRESS) */
        int **double_ptr = &ptr1;
        asm volatile (
            "ldr %[val], [%[ptr]]\n\t"
            : [val] "=r" (auto_var1)
            : [ptr] "r" (double_ptr), "m" (*double_ptr)
            : "memory"
        );
        
        /* Output address reload (RELOAD_FOR_OUTPUT_ADDRESS) */
        int output_array[8];
        asm volatile (
            "str %[in], [%[out], %[idx], lsl #2]\n\t"
            : "=m" (output_array[0]), "=m" (output_array[7])
            : [in] "r" (iteration), [out] "r" (output_array),
              [idx] "r" (idx % 8)
            : "memory"
        );
        
        /* Update globals to prevent dead code elimination */
        global_array[iteration % 32] += auto_var1 + auto_var2 + auto_var3;
        global_counter++;
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = 0;
    checksum += reg_var1 + reg_var2;
    checksum += auto_var1 + auto_var2 + auto_var3;
    checksum += vol_var;
    checksum += (uintptr_t)ptr1 + (uintptr_t)ptr2;
    checksum += (int)fp_var;
    
    for (int i = 0; i < 256; i++) checksum += array[i];
    for (int i = 0; i < 32; i++) checksum += global_array[i];
    
    checksum += packed.i + (packed.l & 0xFFFFFFFF);
    checksum += addr_taken1 + addr_taken2;
    checksum += global_counter;
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum % 256);
}
