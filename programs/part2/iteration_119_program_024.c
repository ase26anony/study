/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int g_volatile = 42;
int g_normal = 100;
register int g_reg asm("ebx") = 200;  /* Force specific register binding */

/* Bitfield structure to generate SUBREG accesses */
struct bitfields {
    int a : 8;
    int b : 16;
    int c : 8;
} g_bits = {1, 2, 3};

/* Test 1: Complex addressing modes with fixed register constraints */
void test_fixed_reg_constraints(void) {
    int input = g_volatile;
    int output;
    
    /* Force secondary reload: memory -> fixed register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)          /* Output in any register */
        : "m"(input)            /* Input from memory */
        : "%eax", "memory"      /* Clobber eax and memory */
    );
    
    /* Alternative constraints that may require secondary reloads */
    int temp = output + 1;
    asm volatile (
        "addl %1, %0"
        : "+a"(temp)            /* Input/output in eax */
        : "rm"(g_normal)        /* Register or memory */
        : "cc"
    );
    
    g_normal = temp;
}

/* Test 2: Multiple alternative constraints */
void test_multiple_alternatives(void) {
    int x = g_volatile;
    int y = g_normal;
    
    /* Multiple constraints that may force difficult choices */
    asm volatile (
        "imull %1, %0"
        : "+r,a"(x)             /* Two alternatives for output */
        : "rm,i"(y)             /* Register/memory or immediate */
        : "cc"
    );
    
    /* Complex constraint with matching */
    int result;
    asm volatile (
        "lea (%1, %2, 2), %0"
        : "=r"(result)
        : "r"(x), "r"(y)
    );
    
    g_normal = result;
}

/* Test 3: Register variables with conflicts */
void test_register_conflicts(void) {
    /* Declare register variables bound to specific registers */
    register int r1 asm("esi") = g_volatile;
    register int r2 asm("edi") = g_normal;
    
    /* Force conflict: use register variable in asm requiring different reg */
    int temp;
    asm volatile (
        "xchgl %%ebx, %1\n\t"   /* Use ebx (g_reg) */
        "addl %%esi, %0\n\t"    /* Use esi (r1) */
        "xchgl %%ebx, %1"       /* Restore */
        : "=r"(temp), "+r"(r2)
        : "1"(r2)
        : "%ebx", "%esi", "cc"
    );
    
    /* Access bitfields to generate SUBREG */
    g_bits.a = temp & 0xFF;
    g_bits.b = (temp >> 8) & 0xFFFF;
    
    /* STRICT_LOW_PART pattern via truncation */
    int16_t low_part = (int16_t)temp;
    asm volatile (
        "movw %1, %0"
        : "=r"(r1)
        : "r"(low_part)
    );
}

/* Test 4: Complex memory addressing */
void test_complex_addressing(void) {
    volatile int arr[10] = {0};
    int index = g_volatile & 7;
    
    /* Complex addressing mode that might need secondary reload */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%1, %2, 4)"
        : 
        : "r"(arr), "r"(index)
        : "%eax", "memory", "cc"
    );
    
    /* Double indirection */
    int *ptr = &g_normal;
    int value;
    asm volatile (
        "movl (%1), %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(value)
        : "r"(ptr)
        : "%eax", "memory"
    );
    
    g_volatile = value;
}

/* Test 5: Mixed size operations for partial register accesses */
void test_mixed_sizes(void) {
    uint64_t big = (uint64_t)g_normal * 1000;
    uint32_t part1, part2;
    
    /* Access 64-bit value as two 32-bit parts */
    part1 = (uint32_t)big;
    part2 = (uint32_t)(big >> 32);
    
    /* Operations that might generate SUBREG */
    asm volatile (
        "addl %1, %0\n\t"
        "adcl $0, %2"
        : "+r"(part1), "+r"(part2)
        : "r"(g_volatile)
        : "cc"
    );
    
    /* Byte operations */
    unsigned char bytes[4];
    asm volatile (
        "movb %%al, %0\n\t"
        "movb %%ah, %1"
        : "=m"(bytes[0]), "=m"(bytes[1])
        : 
        : "%eax"
    );
}

/* Test 6: Inline asm with many operands and clobbers */
void test_many_operands(void) {
    int a = g_volatile;
    int b = g_normal;
    int c = g_reg;
    int d, e, f;
    
    asm volatile (
        "movl %3, %0\n\t"
        "imull %4, %0\n\t"
        "addl %5, %0\n\t"
        "movl %0, %1\n\t"
        "shrl $8, %1\n\t"
        "movl %0, %2\n\t"
        "andl $0xFF, %2"
        : "=&r"(d), "=r"(e), "=r"(f)
        : "rm"(a), "rm"(b), "rm"(c)
        : "cc", "memory"
    );
    
    g_normal = d + e + f;
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    /* Run tests multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test_fixed_reg_constraints();
        result += g_normal;
        
        test_multiple_alternatives();
        result += g_volatile;
        
        test_register_conflicts();
        result += g_bits.a + g_bits.b;
        
        test_complex_addressing();
        result += g_normal;
        
        test_mixed_sizes();
        result += g_volatile;
        
        test_many_operands();
        result += g_normal;
        
        /* Force some control flow variation */
        if (result & 1) {
            g_volatile++;
        } else {
            g_normal--;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    return result & 0xFF;
}
