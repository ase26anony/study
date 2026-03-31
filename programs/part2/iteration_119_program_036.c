/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 1234;
int global_normal = 5678;
static int static_global = 9012;

/* Bitfield structure to generate SUBREG RTL patterns */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} bitfield_global = {0xAAAA, 0xBBBB, 0xCC};

/* Test 1: Force secondary reloads via fixed register constraints */
void test_fixed_registers(void) {
    /* Bind variables to specific registers */
    register int reg_eax asm("eax");
    register int reg_ebx asm("ebx");
    register int reg_ecx asm("ecx");
    
    int temp1, temp2, temp3;
    
    /* Complex inline assembly with conflicting constraints */
    asm volatile (
        /* Move from memory to fixed register - may need secondary reload */
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "=r"(temp1)
        : "m"(global_volatile)
        : "%eax", "memory", "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %2, %1\n\t"
        "addl %1, %0\n\t"
        : "+r"(temp2), "=&a"(reg_eax)
        : "rm"(global_normal), "0"(temp2)
        : "cc"
    );
    
    /* Output in fixed register, input with multiple alternatives */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "leal (%%ecx, %2), %%eax\n\t"
        : "=a"(temp3)
        : "g"(static_global), "r"(reg_ebx)
        : "%ecx", "cc"
    );
    
    /* Use the results to prevent dead code elimination */
    global_volatile = temp1 + temp2 + temp3;
}

/* Test 2: SUBREG patterns via bitfield operations */
void test_subreg_patterns(void) {
    struct bitfield_struct local_bitfield;
    local_bitfield.low16 = 0x1234;
    local_bitfield.high16 = 0x5678;
    local_bitfield.volatile_field = 0x9A;
    
    /* Operations that generate SUBREG accesses */
    uint16_t low_part = local_bitfield.low16;
    uint16_t high_part = local_bitfield.high16;
    
    /* Force reloads with mixed-size operations */
    asm volatile (
        "movzwl %1, %%eax\n\t"
        "addw %2, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=m"(local_bitfield.low16)
        : "m"(high_part), "r"(low_part)
        : "%eax", "cc"
    );
    
    /* Explicit truncation */
    int32_t large_val = 0x12345678;
    int16_t truncated = (int16_t)large_val;
    
    /* Use truncated value in constrained context */
    asm volatile (
        "addw %1, %0\n\t"
        : "+m"(local_bitfield.high16)
        : "ri"(truncated)
        : "cc"
    );
}

/* Test 3: Complex addressing modes and memory operands */
void test_complex_addressing(void) {
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int* volatile_ptr = array;
    int index = global_volatile & 7;
    
    /* Memory operand with complex addressing */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+m"(global_normal)
        : "r"(array), "r"(index)
        : "%eax", "memory", "cc"
    );
    
    /* Multiple memory constraints */
    asm volatile (
        "movl %1, %%ebx\n\t"
        "addl %%ebx, %0\n\t"
        "movl %0, %%ecx\n\t"
        : "+m"(array[5])
        : "m"(volatile_ptr[index])
        : "%ebx", "%ecx", "memory", "cc"
    );
    
    /* Immediate to memory with register pressure */
    register int reg_edx asm("edx");
    reg_edx = 999;
    
    asm volatile (
        "movl %1, %%eax\n\t"
        "subl %%eax, %0\n\t"
        : "+m"(static_global)
        : "ri"(reg_edx)
        : "%eax", "cc"
    );
}

/* Test 4: STRICT_LOW_PART patterns */
void test_strict_low_part(void) {
    unsigned int combined = 0;
    unsigned short low_part, high_part;
    
    /* Operations that might generate STRICT_LOW_PART */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=m"(low_part)
        : "ri"(0xABCD)
        : "%ax"
    );
    
    asm volatile (
        "movw %1, %%bx\n\t"
        "movw %%bx, %0\n\t"
        : "=m"(high_part)
        : "ri"(0x1234)
        : "%bx"
    );
    
    /* Combine parts - may need partial register updates */
    asm volatile (
        "movzwl %1, %%eax\n\t"
        "shll $16, %%eax\n\t"
        "movzwl %2, %%ebx\n\t"
        "orl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(combined)
        : "m"(high_part), "m"(low_part)
        : "%eax", "%ebx", "cc"
    );
    
    /* Use the result */
    global_volatile = combined;
}

/* Test 5: Register pressure and spilling */
void test_register_pressure(void) {
    /* Create many live variables to force register spilling */
    int v1 = global_volatile;
    int v2 = global_normal;
    int v3 = static_global;
    int v4 = bitfield_global.low16;
    int v5 = bitfield_global.high16;
    int v6, v7, v8, v9, v10;
    
    /* Complex expression with many intermediate values */
    v6 = v1 + v2;
    v7 = v3 - v4;
    v8 = v5 * v6;
    v9 = v7 / (v1 ? v1 : 1);
    v10 = v8 ^ v9;
    
    /* Inline assembly that uses many registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "movl %3, %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(global_normal)
        : "g"(v6), "g"(v7), "g"(v8)
        : "%eax", "%ebx", "%ecx", "memory", "cc"
    );
    
    /* More operations to create complex live ranges */
    for (int i = 0; i < 5; i++) {
        asm volatile (
            "addl $1, %0\n\t"
            : "+m"(v10)
            :
            : "cc"
        );
    }
    
    /* Final use to prevent elimination */
    global_volatile = v10;
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 3; i++) {
        test_fixed_registers();
        result += global_volatile;
        
        test_subreg_patterns();
        result += bitfield_global.low16;
        
        test_complex_addressing();
        result += global_normal;
        
        test_strict_low_part();
        result += global_volatile;
        
        test_register_pressure();
        result += static_global;
        
        /* Modify globals to change code paths */
        global_volatile ^= result;
        global_normal += i;
        static_global -= result;
    }
    
    printf("Final result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
