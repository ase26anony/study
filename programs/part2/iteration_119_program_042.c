/* reload_stress_test.c
 * Designed to trigger GCC's reload pass initialization of secondary reload fields
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 12345;
int global_normal = 67890;

/* Bitfield structure to generate SUBREG RTL patterns */
struct bitfield_struct {
    int full : 32;
    int low16 : 16;
    int high16 : 16;
    int partial : 8;
} bf;

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_volatile;
    int output;
    
    /* Force secondary reload: memory -> specific register (eax) */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(input)
        : "%eax"
    );
    
    /* Multiple alternative constraints with fixed register output */
    int in2 = global_normal;
    int out2 = output;
    
    asm volatile (
        "addl %1, %0"
        : "+a"(out2)          /* Fixed to eax */
        : "rm"(in2)           /* Register or memory */
        : "cc"
    );
    
    global_normal = out2;
}

/* Function using register variables bound to specific registers */
void test_register_binding(void) {
    /* Bind to specific registers that might conflict */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = global_volatile;
    r2 = global_normal;
    
    /* Force a move between fixed registers */
    int result;
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : 
        : "%eax", "%ebx", "%ecx"
    );
    
    /* Use in another asm with different constraints */
    asm volatile (
        "imull %1, %0"
        : "+r"(result)
        : "r"(r1)
        : "cc"
    );
    
    global_volatile = result;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    /* Bitfield accesses generate SUBREG */
    bf.full = global_normal;
    bf.low16 = (int16_t)global_volatile;  /* Truncation */
    bf.high16 = (int16_t)(global_volatile >> 16);
    
    /* Use bitfields in arithmetic */
    int32_t temp = bf.full;
    int16_t low_part = bf.low16;
    int16_t high_part = bf.high16;
    
    /* Operations that might use partial registers */
    temp = temp + (int32_t)low_part;
    temp = temp - (int32_t)high_part;
    
    /* Inline asm with partial register constraints */
    asm volatile (
        "addw %w1, %w0"  /* Using 16-bit register names */
        : "+r"(temp)
        : "r"(low_part)
        : "cc"
    );
    
    global_normal = temp;
}

/* Complex function mixing various patterns */
void test_complex_reloads(void) {
    volatile int mem_var = 9999;
    register int fixed_reg asm("esi") = 7777;
    
    /* Multiple asm statements with overlapping constraints */
    for (int i = 0; i < 3; i++) {
        int temp;
        
        /* Memory input, fixed register output */
        asm volatile (
            "movl %1, %%eax\n\t"
            "leal (%%eax, %2), %0"
            : "=r"(temp)
            : "m"(mem_var), "r"(fixed_reg)
            : "%eax"
        );
        
        /* Immediate value with register constraint */
        asm volatile (
            "orl $0xFF, %0"
            : "+a"(temp)  /* Force eax */
            :
            : "cc"
        );
        
        /* Use result in another asm */
        asm volatile (
            ""
            : "+r"(temp)
            : "r"(fixed_reg)
        );
        
        mem_var = temp + i;
    }
    
    /* Double word operations for potential secondary reloads */
    int64_t big_val = (int64_t)global_normal * 1000;
    int32_t part1, part2;
    
    part1 = (int32_t)(big_val & 0xFFFFFFFF);
    part2 = (int32_t)(big_val >> 32);
    
    /* Mixed size operations */
    asm volatile (
        "addl %1, %0\n\t"
        "adcl $0, %0"
        : "+r"(part1)
        : "r"(part2)
        : "cc"
    );
    
    global_volatile = part1;
}

/* Function with pointer arithmetic and memory constraints */
void test_memory_constraints(void) {
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = i * global_volatile;
    }
    
    int *ptr = &array[5];
    int index = global_normal & 3;
    
    /* Complex addressing mode in asm */
    int result;
    asm volatile (
        "movl (%1, %2, 4), %0"
        : "=r"(result)
        : "r"(ptr), "r"(index)
        : "memory"
    );
    
    /* Force reload with 'g' constraint */
    asm volatile (
        "subl %1, %0"
        : "+g"(result)
        : "g"(global_volatile)
        : "cc"
    );
    
    global_normal = result;
}

/* Main function that calls all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize bitfield */
    bf.full = 0x12345678;
    bf.low16 = 0xABCD;
    bf.high16 = 0xEF01;
    bf.partial = 0x42;
    
    /* Run all test functions multiple times */
    for (int i = 0; i < 2; i++) {
        test_restrictive_constraints();
        checksum += global_normal;
        
        test_register_binding();
        checksum += global_volatile;
        
        test_subreg_patterns();
        checksum += bf.full;
        
        test_complex_reloads();
        checksum += global_volatile;
        
        test_memory_constraints();
        checksum += global_normal;
    }
    
    /* Final computation to prevent dead code elimination */
    checksum = (checksum & 0xFFFF) + (checksum >> 16);
    
    /* Output to prevent optimization */
    printf("Reload test checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
