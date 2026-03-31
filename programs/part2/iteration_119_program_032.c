/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int g_volatile = 42;
int g_array[100] = {0};
int g_global = 100;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
} g_bitfield = {0x1234, 0x5678};

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_registers(void) {
    int input = g_volatile;
    int output;
    
    /* Force secondary reload: memory -> specific register (eax) */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(input)
        : "%eax", "memory"
    );
    
    /* Alternative constraints forcing reload decisions */
    int a = output + 1;
    int b = g_volatile;
    
    asm volatile (
        "addl %1, %0"
        : "+r"(a)
        : "rm"(b)
        : "cc"
    );
    
    g_global = a;
}

/* Function using register variables bound to specific registers */
void test_register_variables(void) {
    /* Bind to specific registers that might conflict */
    register int r1 asm("ebx");
    register int r2 asm("edi");
    register int r3 asm("esi");
    
    r1 = g_volatile;
    r2 = r1 * 2;
    
    /* Force move between fixed registers via memory */
    volatile int temp = r2;
    
    /* Inline asm that requires different register */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "addl %%ecx, %0"
        : "+r"(r3)
        : "r"(temp)
        : "%ecx", "memory"
    );
    
    g_array[0] = r3;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    /* Operations that generate partial register accesses */
    int32_t full = g_volatile * 3;
    
    /* Explicit truncation to 16-bit */
    int16_t half = (int16_t)full;
    
    /* Bitfield access - likely generates SUBREG */
    struct bitfield_struct local_bf;
    local_bf.low16 = half;
    local_bf.high16 = full >> 16;
    
    /* Use in inline asm with register constraints */
    uint32_t combined;
    asm volatile (
        "movzwl %1, %%eax\n\t"      /* Zero-extend low16 */
        "shll $16, %%eax\n\t"       /* Shift to high position */
        "orl %2, %%eax\n\t"         /* Combine with high16 */
        "movl %%eax, %0"
        : "=r"(combined)
        : "r"(local_bf.high16), "r"(local_bf.low16)
        : "%eax"
    );
    
    g_global = combined;
}

/* Complex addressing modes with multiple alternatives */
void test_complex_addressing(void) {
    int index = g_volatile & 0xF;
    int scale = 2;
    
    /* Force complex address calculation */
    int* ptr = &g_array[index * scale + 3];
    
    /* Inline asm with multiple constraints */
    int result;
    asm volatile (
        "movl (%1, %2, %c3), %0"
        : "=r"(result)
        : "r"(g_array), "r"(index), "i"(sizeof(int) * scale + 3)
        : "memory"
    );
    
    /* Use result in another constrained operation */
    register int fixed_reg asm("edx") = result;
    
    asm volatile (
        "imull %1, %0"
        : "+r"(fixed_reg)
        : "rm"(g_volatile)
        : "cc"
    );
    
    g_array[1] = fixed_reg;
}

/* Function mixing volatile and register variables */
void test_volatile_mix(void) {
    volatile int v1 = g_volatile;
    volatile int v2 = g_global;
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Force reloads between volatile and register */
    r1 = v1;
    r2 = v2;
    
    /* Complex expression with intermediate results */
    int temp = r1 * r2 + g_array[0];
    
    /* Inline asm with output in specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "leal (%%eax, %%eax, 2), %0"
        : "=r"(r1)
        : "r"(temp)
        : "%eax"
    );
    
    v1 = r1;
    
    /* Another memory barrier */
    asm volatile ("" : : : "memory");
}

/* Test with immediate values and specific register classes */
void test_immediate_constraints(void) {
    int output;
    
    /* Immediate value with specific register constraint */
    asm volatile (
        "movl $0xDEADBEEF, %0"
        : "=a"(output)  /* Must be in eax */
    );
    
    /* Use output in memory operation */
    g_volatile = output;
    
    /* Another with different immediate */
    int output2;
    asm volatile (
        "movl %%eax, %0\n\t"
        "addl $42, %0"
        : "=r"(output2)
        : "a"(output)   /* Input must be in eax */
        : "cc"
    );
    
    g_global = output2;
}

/* Main function that calls all tests */
int main(void) {
    int sum = 0;
    
    /* Call each test multiple times with different values */
    for (int i = 0; i < 10; i++) {
        g_volatile = i * 7 + 1;
        
        test_restrictive_registers();
        sum += g_global;
        
        test_register_variables();
        sum += g_array[0];
        
        test_subreg_patterns();
        sum += g_global;
        
        test_complex_addressing();
        sum += g_array[1];
        
        test_volatile_mix();
        sum += g_volatile;
        
        test_immediate_constraints();
        sum += g_global;
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(sum) : "memory");
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
