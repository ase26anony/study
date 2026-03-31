/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int g_volatile_int = 42;
int g_normal_int = 100;
static int g_static_int = 200;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low : 8;
    unsigned int high : 24;
} g_bitfield = {0xAA, 0xBBCCDD};

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_registers(void) {
    int input = g_volatile_int;
    int output;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(input)
        : "%eax"
    );
    
    /* Multiple alternative constraints with fixed register output */
    int a = output + 1;
    int b = g_normal_int;
    
    asm volatile (
        "addl %1, %0"
        : "+a"(a)      /* Fixed to eax */
        : "rm"(b)      /* Register or memory */
        : "cc"
    );
    
    g_normal_int = a;
}

/* Test 2: Register variables with conflicting constraints */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int r1 asm("ebx") = g_volatile_int;
    register int r2 asm("ecx") = g_static_int;
    
    /* Force conflict: use r1 in inline asm that requires different register */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "r"(r1), "r"(r2)
        : "%eax"
    );
    
    /* Complex constraint with immediate */
    asm volatile (
        "imull %1, %0"
        : "+r"(result)
        : "rmi"(123)  /* Register, memory, or immediate */
        : "cc"
    );
    
    g_static_int = result;
}

/* Test 3: SUBREG generation through bitfield operations */
void test_bitfield_subreg(void) {
    volatile struct bitfield_struct local_bf;
    local_bf = g_bitfield;
    
    /* Access bitfields - may generate SUBREG RTL */
    unsigned char low_part = local_bf.low;
    unsigned int high_part = local_bf.high;
    
    /* Force truncation to smaller types */
    int16_t truncated = (int16_t)high_part;
    uint8_t small = (uint8_t)(low_part + 1);
    
    /* Use in inline asm with restrictive constraints */
    asm volatile (
        "movb %1, %%al\n\t"
        "addw %2, %%ax"
        : "+a"(truncated)
        : "r"(small), "r"(truncated)
        : "cc"
    );
    
    /* STRICT_LOW_PART pattern */
    int32_t full = high_part;
    asm volatile (
        "andl $0x0000FFFF, %0"
        : "+r"(full)
        :
        : "cc"
    );
}

/* Test 4: Complex addressing modes and multiple reloads */
void test_complex_addressing(void) {
    int array[10];
    volatile int* volatile_ptr = &g_volatile_int;
    
    /* Create complex addressing situation */
    for (int i = 0; i < 10; i++) {
        array[i] = i * g_volatile_int;
    }
    
    /* Force memory operand with index */
    int sum = 0;
    asm volatile (
        "movl (%%ebx, %%ecx, 4), %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(sum)
        : "b"(array), "c"(5)   /* ebx=array, ecx=5 */
        : "%eax", "memory"
    );
    
    /* Multiple memory constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=m"(*volatile_ptr)
        : "m"(sum)
        : "%eax", "memory"
    );
}

/* Test 5: Mixed constraints and optimization barriers */
void test_mixed_constraints(void) {
    volatile int v1 = g_volatile_int;
    volatile int v2 = g_normal_int;
    int r1, r2;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Complex constraint with multiple alternatives */
    asm volatile (
        "movl %2, %0\n\t"
        "addl %3, %0\n\t"
        "movl %0, %1"
        : "=&r"(r1), "=m"(v1)
        : "g"(v2), "ri"(100)  /* General or immediate */
        : "cc"
    );
    
    /* Force secondary reload with specific register class */
    double dbl = 3.14159;
    long long ll;
    
    /* This may require secondary reloads on some architectures */
    asm volatile (
        "movq %1, %0"
        : "=r"(ll)
        : "m"(dbl)
    );
}

/* Test 6: Nested inline asm and register pressure */
void test_register_pressure(void) {
    /* Create many live variables to increase register pressure */
    int a = g_volatile_int;
    int b = a + 1;
    int c = b * 2;
    int d = c - 3;
    int e = d / 4;
    int f = e % 5;
    int g = f << 2;
    int h = g >> 1;
    int i = h | 0xFF;
    int j = i & 0x0F;
    
    /* Use all in one complex asm statement */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "subl %3, %%eax\n\t"
        "imull %4, %%eax\n\t"
        "addl %5, %%eax\n\t"
        "orl %6, %%eax\n\t"
        "andl %7, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm"(j)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g)
        : "%eax", "cc", "memory"
    );
    
    g_volatile_int = j;
}

int main(void) {
    int result = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    for (int i = 0; i < 3; i++) {
        test_restrictive_registers();
        test_register_conflicts();
        test_bitfield_subreg();
        test_complex_addressing();
        test_mixed_constraints();
        test_register_pressure();
        
        result += g_volatile_int + g_normal_int + g_static_int;
        
        /* Modify globals to create different code paths */
        g_volatile_int += i;
        g_normal_int -= i;
        g_static_int *= (i + 1);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    /* Final complex asm to ensure reloads are needed */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "+m"(result)
        :
        : "%eax", "cc"
    );
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
