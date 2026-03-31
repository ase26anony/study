#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0, 1, 2, 3, 4, 5, 6, 7};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16];

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Force a ZERO_EXTRACT by writing to specific bits of a memory location */
    __asm__ volatile (
        "btsl %1, %0\n\t"          /* Bit test and set - modifies specific bit */
        : "+m" (*ptr)              /* Memory operand that will be ZERO_EXTRACTed */
        : "Ir" (bitpos)            /* Immediate or register for bit position */
        : "cc"
    );
    
    /* Another pattern: writing to a bitfield using constraints */
    uint32_t mask = (1 << bitsize) - 1;
    __asm__ volatile (
        "andl %1, %0\n\t"          /* Clear bits outside the field */
        "orl %2, %0\n\t"           /* Set specific bits */
        : "+m" (*ptr)
        : "r" (~(mask << bitpos)), "r" ((0x5A << bitpos) & (mask << bitpos))
        : "cc"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *arr, int index)
{
    uint32_t temp;
    
    /* Force STRICT_LOW_PART by modifying only part of a register */
    __asm__ volatile (
        "movzwl (%1), %0\n\t"      /* Zero-extend 16-bit load */
        "addl $0x1234, %0\n\t"     /* Add to full 32-bit register */
        "movw %%ax, (%1)\n\t"      /* But only write back low 16 bits */
        : "=&r" (temp)             /* Early-clobber register */
        : "r" (&arr[index])
        : "memory", "cc"
    );
    
    /* Another pattern using constraint modifiers */
    __asm__ volatile (
        "incw %0"                  /* Increment word - affects only low part */
        : "+m" (arr[index])
        :
        : "cc"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint64_t *big_var)
{
    /* Access different sub-regions of the memory */
    volatile uint32_t *as_int = (volatile uint32_t *)big_var;
    volatile uint16_t *as_short = (volatile uint16_t *)big_var;
    volatile uint8_t *as_byte = (volatile uint8_t *)big_var;
    
    /* Write to sub-register of memory using inline asm */
    __asm__ volatile (
        "movl $0xDEADBEEF, %0"
        : "=m" (*as_int)           /* 32-bit access to 64-bit memory */
        :
        : "memory"
    );
    
    __asm__ volatile (
        "movw $0x1234, %0"
        : "=m" (as_short[1])       /* 16-bit access with offset */
        :
        : "memory"
    );
    
    /* Complex pattern with multiple sub-register accesses */
    __asm__ volatile (
        "movb $0xAA, %0\n\t"
        "movb $0xBB, %1"
        : "=m" (as_byte[2]), "=m" (as_byte[6])
        :
        : "memory"
    );
}

/* Function that combines all patterns for complex RTL generation */
static void test_combined_patterns(void)
{
    volatile uint32_t combined = 0;
    volatile uint64_t big = 0x1122334455667788ULL;
    
    /* Create a SUBREG -> MEM chain */
    __asm__ volatile (
        "leaq %1, %%rax\n\t"
        "movl $0x88776655, (%%rax)\n\t"   /* Write to low 32 bits */
        "movw $0x4433, 4(%%rax)"          /* Write to next 16 bits */
        : 
        : "m" (big), "m" (big)            /* Memory operand appears twice */
        : "rax", "memory", "cc"
    );
    
    /* ZERO_EXTRACT with memory operand */
    __asm__ volatile (
        "lock btrl $17, %0"               /* Bit test and reset with lock prefix */
        : "+m" (combined)
        :
        : "cc"
    );
}

/* Main driver function */
int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 5, 4);
    test_zero_extract(&global_int, 16, 8);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(global_short_array, 2);
    test_strict_low_part(global_short_array, 5);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_long);
    checksum += (uint32_t)(global_long >> 32);
    checksum += (uint32_t)global_long;
    
    /* Test combined patterns */
    test_combined_patterns();
    
    /* Access byte array to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        global_byte_array[i] = i;
        checksum += global_byte_array[i];
    }
    
    /* Final checksum to ensure all code executes */
    printf("Checksum: 0x%08X\n", checksum);
    
    return (int)(checksum & 0xFF);
}
