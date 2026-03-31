#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Write to specific bit-field (bits 8-15) using inline assembly */
    __asm__ volatile (
        "mov %1, %0\n\t"
        : "=r" (*dest)
        : "r" (src & 0xFF00)  /* Only bits 8-15 are significant */
        : "memory"
    );
}

/* Alternative ZERO_EXTRACT with memory destination */
static void test_zero_extract_mem(volatile uint32_t *dest, uint32_t src) {
    /* Using constraint modifiers to hint at bit-field access */
    __asm__ volatile (
        "or %0, %1\n\t"
        : "+m" (*dest)
        : "r" (src & 0x0000FF00)  /* Modify only bits 8-15 */
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    uint32_t temp;
    
    /* Operation that only modifies low part of register */
    __asm__ volatile (
        "addw %0, %1\n\t"  /* 'w' suffix for 16-bit operation */
        : "=r" (temp)
        : "r" (src), "0" ((uint32_t)*dest)
        : "cc"
    );
    
    *dest = (uint16_t)temp;
}

/* STRICT_LOW_PART with early-clobber to force complex RTL */
static void test_strict_low_part_complex(volatile uint32_t *dest, uint32_t src1, uint32_t src2) {
    /* Early-clobber (&) complicates register allocation */
    __asm__ volatile (
        "imul %0, %1, %2\n\t"
        : "=&r" (*dest)    /* Early-clobber output */
        : "r" (src1 & 0xFFFF), "r" (src2 & 0xFFFF)  /* Only low 16 bits used */
        : "cc"
    );
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Write to 16-bit sub-register of 32-bit memory location */
    __asm__ volatile (
        "movw %1, %0\n\t"
        : "=m" (*short_ptr)      /* 16-bit memory access */
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
}

/* SUBREG of MEM with array access */
static void test_subreg_mem_array(volatile uint64_t *array) {
    /* Access 32-bit portion of 64-bit array element */
    volatile uint32_t *sub_ptr = (volatile uint32_t *)&array[1];
    
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "+m" (*sub_ptr)        /* 32-bit access within 64-bit memory */
        : "ri" (0x1000)
        : "cc", "memory"
    );
}

/* Complex case: SUBREG -> MEM -> address computation */
static void test_subreg_mem_complex(volatile uint8_t *base) {
    /* Access 16-bit value at offset 2 within byte array */
    volatile uint16_t *ptr = (volatile uint16_t *)(base + 2);
    
    __asm__ volatile (
        "xorw %0, %0\n\t"
        : "=m" (*ptr)
        :
        : "memory", "cc"
    );
}

/* Driver function that exercises all patterns */
int main(void) {
    volatile uint32_t local_int = 0xDEADBEEF;
    volatile uint64_t local_long_array[4] = {0};
    volatile uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0x00FF0000);
    test_zero_extract_mem(&local_int, 0x0000FF00);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part_complex(&local_int, 0x1111, 0x2222);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_int);
    test_subreg_mem_array(local_long_array);
    test_subreg_mem_complex(global_byte_array);
    
    /* Additional mixed test cases */
    for (int i = 0; i < 4; i++) {
        test_zero_extract(&local_int, i * 0x1000);
        test_subreg_mem((volatile uint32_t *)&global_short_array[i]);
    }
    
    /* Compute checksum to prevent dead code elimination */
    checksum += global_int;
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    checksum += (uint32_t)global_long;
    checksum += (uint32_t)(global_long >> 32);
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    checksum += local_int;
    for (int i = 0; i < 4; i++) {
        checksum += (uint32_t)local_long_array[i];
        checksum += (uint32_t)(local_long_array[i] >> 32);
    }
    
    printf("Checksum: %u\n", checksum);
    return (int)checksum;
}
