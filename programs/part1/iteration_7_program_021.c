#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Use bit-field assignment to generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        /* Write to bits 8-15 of dest */
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output constraint for register */
        : "r" (src)     /* Input constraint */
        : "memory"
    );
}

/* Alternative ZERO_EXTRACT with memory destination */
static void test_zero_extract_mem(volatile uint32_t *dest, uint32_t src) {
    uint32_t temp;
    __asm__ volatile (
        /* Complex pattern that might generate ZERO_EXTRACT on memory */
        "and %0, %1, #0xFF00\n\t"  /* Extract middle byte */
        : "=r" (temp)
        : "r" (src)
    );
    
    /* Force memory write with bit-field semantics */
    __asm__ volatile (
        "strb %1, [%0, #1]\n\t"  /* Store byte at offset 1 (bits 8-15) */
        : 
        : "r" (dest), "r" ((uint8_t)(temp >> 8))
        : "memory"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    uint32_t wide_reg;
    
    __asm__ volatile (
        /* Operation that only affects low part of register */
        "add %0, %1, %2\n\t"
        /* The '=r' constraint with early clobber might generate STRICT_LOW_PART */
        : "=&r" (wide_reg)      /* Early clobber to force new register */
        : "r" ((uint32_t)*dest), "r" ((uint32_t)src)
    );
    
    /* Write back only low 16 bits */
    *dest = (uint16_t)wide_reg;
}

/* Alternative STRICT_LOW_PART with inline assembly modifier */
static void test_strict_low_part_direct(volatile uint32_t *dest, uint32_t src) {
    __asm__ volatile (
        /* Using '=r' constraint for 32-bit, but operation only affects low 16 bits */
        "addw %0, %1, %2\n\t"  /* 'w' suffix for 32-bit operation */
        : "=r" (*dest)
        : "r" (*dest), "r" (src)
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access memory through different-sized views */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        /* Write to short within int (SUBREG of MEM) */
        "strh %1, [%0]\n\t"
        : 
        : "r" (short_ptr), "r" ((uint16_t)0xABCD)
        : "memory"
    );
}

/* More complex SUBREG of MEM with array access */
static void test_subreg_mem_complex(volatile uint64_t *array) {
    /* Access different sub-parts of the 64-bit memory location */
    volatile uint32_t *as_int = (volatile uint32_t *)array;
    volatile uint16_t *as_short = (volatile uint16_t *)array;
    volatile uint8_t *as_byte = (volatile uint8_t *)array;
    
    __asm__ volatile (
        /* Multiple memory writes of different sizes to same base */
        "str %1, [%0]\n\t"      /* 32-bit write */
        "strh %2, [%0, #4]\n\t" /* 16-bit write at offset 4 */
        "strb %3, [%0, #6]\n\t" /* 8-bit write at offset 6 */
        : 
        : "r" (as_int), 
          "r" ((uint32_t)0xDEADBEEF),
          "r" ((uint16_t)0xCAFE),
          "r" ((uint8_t)0x42)
        : "memory"
    );
}

/* Combined test with register pressure to force SUBREG patterns */
static void test_with_register_pressure(void) {
    volatile uint32_t vars[10];
    volatile uint16_t shorts[10];
    
    /* Create many live variables to increase register pressure */
    for (int i = 0; i < 10; i++) {
        vars[i] = i * 100;
        shorts[i] = i * 10;
    }
    
    /* Multiple inline asm blocks that might generate SUBREGs */
    for (int i = 0; i < 9; i++) {
        __asm__ volatile (
            "add %0, %1, %2\n\t"
            : "=r" (vars[i+1])
            : "r" (vars[i]), "r" (shorts[i])
        );
    }
    
    /* Access memory through cast pointers (SUBREG of MEM) */
    volatile uint8_t *byte_view = (volatile uint8_t *)&vars[5];
    __asm__ volatile (
        "strb %1, [%0, #2]\n\t"
        : 
        : "r" (byte_view), "r" ((uint8_t)0xFF)
        : "memory"
    );
}

/* Main driver function */
int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x87654321);
    test_zero_extract_mem(&global_int, 0x9ABCDEF0);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part_direct(&global_int, 0x1000);
    checksum += global_short_array[0];
    checksum += global_int;
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem_complex(&global_long);
    checksum += global_int;
    checksum += (uint32_t)(global_long & 0xFFFFFFFF);
    checksum += (uint32_t)(global_long >> 32);
    
    /* Test with register pressure */
    test_with_register_pressure();
    
    /* Access all global arrays to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: %u\n", checksum);
    return (int)(checksum & 0xFF);
}
