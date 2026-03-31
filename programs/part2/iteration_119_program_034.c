/* reload_stress_test.c
 * Designed to trigger secondary reload initialization in GCC's reload.cc
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[4] = {1, 2, 3, 4};

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    int a : 8;
    int b : 16;
    int c : 8;
} bitfield_global;

/* Test 1: Force secondary reloads via fixed register constraints */
void test_fixed_registers() {
    /* Bind variables to specific registers */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    
    /* Force reloads by moving between fixed registers and memory */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (reg_b)
        : "m" (global_var)
        : "eax", "cc"
    );
    
    /* Complex constraint with alternatives */
    int temp = 100;
    asm volatile (
        "imull %1, %0"
        : "+a" (reg_a)
        : "rm" (temp)
        : "cc"
    );
    
    /* Output constraint with specific register class */
    int result;
    asm volatile (
        "movl %%ebx, %0"
        : "=r" (result)
        : 
        : "ebx"
    );
}

/* Test 2: SUBREG operations via bitfields */
void test_subreg_operations() {
    volatile struct bitfield_struct local_bitfield;
    
    /* Operations that generate SUBREG accesses */
    local_bitfield.a = global_var & 0xFF;
    local_bitfield.b = (global_var >> 8) & 0xFFFF;
    local_bitfield.c = (global_var >> 24) & 0xFF;
    
    /* Force partial register accesses */
    int16_t partial;
    asm volatile (
        "movw %1, %0"
        : "=r" (partial)
        : "m" (local_bitfield.b)
    );
    
    /* Truncation that may require reloads */
    int32_t full = 0x12345678;
    int16_t truncated = (int16_t)full;
    
    asm volatile (
        "addw %1, %0"
        : "+r" (truncated)
        : "rm" (partial)
        : "cc"
    );
}

/* Test 3: Complex addressing modes with multiple alternatives */
void test_complex_addressing() {
    int index = 2;
    int value;
    
    /* Memory operand with complex addressing */
    asm volatile (
        "movl (%1,%2,4), %0"
        : "=r" (value)
        : "r" (global_array), "r" (index)
        : "memory"
    );
    
    /* Multiple alternative constraints */
    int temp = 50;
    asm volatile (
        "subl %1, %0"
        : "+r" (value)
        : "rmi" (temp)  /* register, memory, or immediate */
        : "cc"
    );
    
    /* Output to memory with register constraint */
    asm volatile (
        "movl %1, (%0)"
        : 
        : "r" (&global_array[0]), "r" (value)
        : "memory"
    );
}

/* Test 4: STRICT_LOW_PART patterns */
void test_strict_low_part() {
    volatile uint32_t data = 0x87654321;
    uint16_t low_part;
    
    /* This may generate STRICT_LOW_PART */
    asm volatile (
        "movzwl %1, %0"
        : "=r" (low_part)
        : "m" (data)
    );
    
    /* Operation preserving only low part */
    uint32_t modified = data;
    asm volatile (
        "andl $0xFFFF, %0"
        : "+r" (modified)
        : 
        : "cc"
    );
}

/* Test 5: Register pressure to force more reloads */
void test_register_pressure() {
    /* Many live variables to increase register pressure */
    register int r1 asm("eax");
    register int r2 asm("ebx");
    register int r3 asm("ecx");
    register int r4 asm("edx");
    register int r5 asm("esi");
    register int r6 asm("edi");
    
    int v1 = global_var;
    int v2 = global_array[0];
    int v3 = global_array[1];
    int v4 = global_array[2];
    int v5 = global_array[3];
    volatile int v6 = 1000;
    
    /* Complex expression forcing many moves */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (r2)
        : "m" (v1), "r" (v2)
        : "eax", "cc"
    );
    
    asm volatile (
        "imull %1, %0"
        : "+r" (r3)
        : "rm" (v3)
        : "cc"
    );
    
    asm volatile (
        "orl %1, %0"
        : "+r" (r4)
        : "rm" (v4)
        : "cc"
    );
    
    /* Force spill/reload by using all registers */
    asm volatile (
        "movl %1, %%esi\n\t"
        "movl %2, %%edi\n\t"
        "addl %%esi, %%edi\n\t"
        "movl %%edi, %0"
        : "=r" (r5)
        : "m" (v5), "r" (v6)
        : "esi", "edi", "cc"
    );
}

/* Test 6: Immediate values with register constraints */
void test_immediate_reloads() {
    int result;
    
    /* Immediate to fixed register */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : 
        : "eax"
    );
    
    /* Large immediate that might require reloads on some archs */
    asm volatile (
        "addl $0x7FFFFFFF, %0"
        : "+r" (result)
        : 
        : "cc"
    );
    
    /* Immediate with memory destination */
    asm volatile (
        "movl $999, %0"
        : "=m" (global_var)
        : 
        : "memory"
    );
}

/* Main function that runs all tests */
int main() {
    int checksum = 0;
    
    /* Run each test multiple times to increase coverage chance */
    for (int i = 0; i < 3; i++) {
        test_fixed_registers();
        checksum += global_var;
        
        test_subreg_operations();
        checksum += bitfield_global.a;
        
        test_complex_addressing();
        checksum += global_array[0];
        
        test_strict_low_part();
        checksum += 1;
        
        test_register_pressure();
        checksum += 2;
        
        test_immediate_reloads();
        checksum += global_var;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (checksum));
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
