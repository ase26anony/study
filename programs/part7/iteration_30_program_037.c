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
    int large_array[256];
    for (int i = 0; i < 256; i++) {
        large_array[i] = i * 3;
    }
    
    /* Packed/misaligned data */
    struct misaligned_data packed;
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE;
    packed.tail = 'Z';
    
    /* Address-taken variables */
    int addr_taken1 = 100, addr_taken2 = 200;
    int *addr_ptr1 = &addr_taken1;
    int *addr_ptr2 = &addr_taken2;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary constraints based on iteration */
        const char *constraint1 = (iteration % 3 == 0) ? "=r" : "=&r";
        const char *constraint2 = (iteration % 3 == 1) ? "r" : "m";
        int use_memory = iteration & 1;
        
        /* Complex inline assembly to trigger multiple reload types */
        asm volatile (
            /* Output operands with early clobber - RELOAD_FOR_OUTPUT */
            "%0 = add %1, %2\n\t"
            /* Complex addressing mode for input - RELOAD_FOR_INPUT_ADDRESS */
            "ldw %3, [%4 + %5 * 4 + %6]\n\t"
            /* Nested address computation - RELOAD_FOR_OPERAND_ADDRESS */
            "stw %7, [%8]\n\t"
            /* Output with address reload - RELOAD_FOR_OUTPUT_ADDRESS */
            "stw.d %9, [%10 + %11]\n\t"
            /* Multiple constraints to force spills - RELOAD_OTHER */
            : "=r"(auto_var1), "=m"(large_array[iteration * 8])
            : "r"(reg_var1), 
              "m"(*(int*)((char*)&packed + 1)),  /* Misaligned access */
              "r"(large_array), 
              "r"(iteration), 
              "i"(16),
              "r"(vol_var),
              "r"(&large_array[iteration * 4 + 1]),
              "r"(global_counter),
              "r"(iteration * sizeof(long)),
              "0"(auto_var1)
            : "memory", "r14", "r15", "cc"
        );
        
        /* Second asm with different operand patterns */
        if (iteration & 2) {
            /* Trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
            asm volatile (
                "mov %0, [%1 + %2]\n\t"
                "mov [%3 + %4], %5\n\t"
                "lea %6, [%7 + %8 * 2]\n\t"
                : "=r"(auto_var2), "=m"(addr_taken1)
                : "r"(ptr1), 
                  "r"(&addr_taken2), 
                  "r"(iteration * 4),
                  "r"(reg_var2),
                  "r"(ptr2),
                  "r"(large_array),
                  "r"(iteration + 128)
                : "memory"
            );
        }
        
        /* Third asm focusing on address-of-address scenarios */
        int **double_ptr = &ptr1;
        asm volatile (
            /* RELOAD_FOR_OPADDR_ADDR - address of an address operand */
            "mov %0, [%1]\n\t"
            "add %0, %2\n\t"
            "mov [%3], %0\n\t"
            : "=&r"(auto_var3)
            : "r"(double_ptr),
              "r"(iteration),
              "m"(*ptr2)
            : "memory"
        );
        
        /* Update variables to create dependencies between iterations */
        reg_var1 += auto_var1;
        reg_var2 ^= auto_var2;
        vol_var = iteration;
        global_counter++;
        
        /* Force memory operand with complex address computation */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=m"(large_array[iteration * 2 + (iteration % 3)])
            : "r"(reg_var1),
              "r"(iteration)
            : "memory"
        );
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = 0;
    checksum += auto_var1;
    checksum += auto_var2;
    checksum += auto_var3;
    checksum += reg_var1;
    checksum += reg_var2;
    checksum += vol_var;
    
    for (int i = 0; i < 256; i++) {
        checksum += large_array[i];
    }
    
    checksum += packed.i;
    checksum += packed.l;
    checksum += addr_taken1;
    checksum += addr_taken2;
    checksum += global_counter;
    
    printf("Checksum: %lu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
