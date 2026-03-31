/* reload_coverage.c - Test program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile variables to prevent optimization */
volatile int vol_int = 42;
volatile long vol_long = 100;
volatile char vol_char = 'A';

/* Global variables with address taken */
int global_array[32];
long global_long_array[16];
char global_char_buffer[64];

/* Function to create register pressure */
void create_reload_scenarios(int iterations) {
    /* Automatic variables with different types */
    int auto_int = 0;
    long auto_long = 0;
    char auto_char = 0;
    int *auto_ptr = &auto_int;
    long *auto_long_ptr = &auto_long;
    
    /* Misaligned struct */
    struct misaligned_data misaligned;
    misaligned.c = 'X';
    misaligned.i = 12345;
    misaligned.l = 67890;
    
    /* Array for indexing */
    int index_array[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    
    /* Loop to vary constraints and create different reload scenarios */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        int scale = 1 + (i % 4);
        
        /* Scenario 1: Complex addressing modes for RELOAD_FOR_INPUT_ADDRESS */
        /* Using [base + index*scale + displacement] addressing */
        asm volatile (
            "mov %[val1], %[dest1]\n\t"
            "add %[val2], %[dest1]\n\t"
            : [dest1] "=r" (auto_int)
            : [val1] "m" (global_array[idx * scale + 2]),
              [val2] "r" (index_array[idx])
            : "cc", "memory"
        );
        
        /* Scenario 2: Multiple operands with conflicting constraints */
        /* Forces RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        asm volatile (
            "add %[in1], %[out1]\n\t"
            "imul %[in2], %[out2]\n\t"
            "mov %[in3], %[out3]\n\t"
            : [out1] "=&r" (auto_int),    /* Early clobber */
              [out2] "=&r" (auto_long),   /* Early clobber */
              [out3] "=r" (auto_char)
            : [in1] "r" (vol_int),
              [in2] "m" (global_long_array[idx]),  /* Memory operand */
              [in3] "i" (65 + i),         /* Immediate */
              "0" (auto_int),             /* Tied operand */
              [out2] "1" (auto_long)      /* Tied operand */
            : "cc"
        );
        
        /* Scenario 3: Nested address computation */
        /* Potentially triggers RELOAD_FOR_OPERAND_ADDRESS */
        void *complex_addr;
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[addr]\n\t"
            "mov (%[addr]), %[tmp]\n\t"
            "add %[inc], %[tmp]\n\t"
            "mov %[tmp], (%[addr])\n\t"
            : [addr] "=&r" (complex_addr),
              [tmp] "=&r" (auto_int)
            : [base] "r" (global_array),
              [index] "r" (idx * sizeof(int)),
              [scale] "i" (1),
              [inc] "r" (i)
            : "memory"
        );
        
        /* Scenario 4: Output address reload (RELOAD_FOR_OUTPUT_ADDRESS) */
        long *output_ptr;
        asm volatile (
            "mov %[src], %%rax\n\t"
            "add $8, %%rax\n\t"
            "mov %%rax, %[ptr]\n\t"
            : [ptr] "=r" (output_ptr)
            : [src] "m" (global_long_array[idx])
            : "rax", "cc"
        );
        
        /* Scenario 5: Input address address (RELOAD_FOR_INPADDR_ADDRESS) */
        /* Taking address of a memory operand that needs reloading */
        int **addr_of_addr;
        asm volatile (
            "lea %[mem], %[aaddr]\n\t"
            "mov (%[aaddr]), %[val]\n\t"
            : [aaddr] "=r" (addr_of_addr),
              [val] "=r" (auto_int)
            : [mem] "m" (auto_ptr)
            : "cc"
        );
        
        /* Scenario 6: Mixed data types and alignment */
        /* Access misaligned struct members */
        asm volatile (
            "movzbl %[char], %%eax\n\t"
            "addl %[int], %%eax\n\t"
            "addq %[long], %%rax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=m" (misaligned.i)
            : [char] "m" (misaligned.c),
              [int] "m" (misaligned.i),
              [long] "r" (misaligned.l)
            : "rax", "cc", "memory"
        );
        
        /* Scenario 7: Many operands to create register pressure */
        /* Forces RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        int r1, r2, r3, r4, r5, r6;
        asm volatile (
            "mov %[a1], %[o1]\n\t"
            "mov %[a2], %[o2]\n\t"
            "mov %[a3], %[o3]\n\t"
            "mov %[a4], %[o4]\n\t"
            "mov %[a5], %[o5]\n\t"
            "mov %[a6], %[o6]\n\t"
            : [o1] "=&r" (r1),
              [o2] "=&r" (r2),
              [o3] "=&r" (r3),
              [o4] "=&r" (r4),
              [o5] "=&r" (r5),
              [o6] "=&r" (r6)
            : [a1] "m" (global_array[0]),
              [a2] "m" (global_array[1]),
              [a3] "m" (global_array[2]),
              [a4] "m" (global_array[3]),
              [a5] "m" (global_array[4]),
              [a6] "m" (global_array[5])
            : "memory"
        );
        
        /* Scenario 8: Operand address address (RELOAD_FOR_OPADDR_ADDR) */
        /* Complex chain of address computations */
        int ***triple_ptr;
        asm volatile (
            "lea %[base], %%rax\n\t"
            "lea (%%rax, %[idx], 4), %%rbx\n\t"
            "mov %%rbx, %[ptr]\n\t"
            : [ptr] "=r" (triple_ptr)
            : [base] "r" (&auto_ptr),
              [idx] "r" (idx)
            : "rax", "rbx", "cc"
        );
        
        /* Update variables to prevent dead code elimination */
        auto_int += i;
        auto_long += auto_int;
        auto_char ^= i;
        vol_int += auto_int;
    }
    
    /* Final checksum calculation with observable side effect */
    unsigned long checksum = 0;
    checksum += auto_int;
    checksum += auto_long;
    checksum += auto_char;
    checksum += (unsigned long)auto_ptr;
    
    /* Force all operations to complete */
    asm volatile ("" : : "r"(checksum) : "memory");
    
    printf("Checksum: %lu\n", checksum);
}

/* Main function with different optimization barriers */
int main() {
    /* Initialize global arrays */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        global_long_array[i] = i * 5;
    }
    for (int i = 0; i < 64; i++) {
        global_char_buffer[i] = 'a' + (i % 26);
    }
    
    /* Create different reload scenarios */
    create_reload_scenarios(16);
    
    /* Additional test with different constraints */
    {
        /* Test with immediate constraints that may need reloading */
        int x = 0, y = 0, z = 0;
        
        /* Force reloads with large immediates */
        asm volatile (
            "mov $0x12345678, %%eax\n\t"
            "mov $0x9ABCDEF0, %%ebx\n\t"
            "add %%eax, %%ebx\n\t"
            "mov %%ebx, %[out]\n\t"
            : [out] "=r" (x)
            : 
            : "eax", "ebx", "cc"
        );
        
        /* Memory operand with complex address */
        asm volatile (
            "mov %[addr1], %%rax\n\t"
            "add %[addr2], %%rax\n\t"
            "mov (%%rax), %[out]\n\t"
            : [out] "=r" (y)
            : [addr1] "r" (global_array),
              [addr2] "r" (16)
            : "rax", "memory"
        );
        
        printf("Results: x=%d, y=%d\n", x, y);
    }
    
    return 0;
}
