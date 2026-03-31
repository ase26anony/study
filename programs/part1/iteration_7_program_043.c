#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Force a ZERO_EXTRACT by writing to specific bits of a register/memory */
    __asm__ volatile (
        "mov %[src], %[tmp]\n\t"
        "and $0xFF, %[tmp]\n\t"          /* Extract lower 8 bits */
        "mov %[tmp], %[out]\n\t"
        : [out] "=r,m" (*dest)           /* Output can be register or memory */
        : [src] "r,m" (src),
          [tmp] "r" (0)                  /* Early clobber to force complexity */
        : "memory"
    );
}

/* Function to test ZERO_EXTRACT with bit-field assignment */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t mask = 0x00000F00;  /* Bits 8-11 */
    uint32_t value = 0x5;
    
    /* This should generate ZERO_EXTRACT for the specific bit range */
    __asm__ volatile (
        "bts $8, %[dest]\n\t"           /* Set bit 8 */
        "btr $9, %[dest]\n\t"           /* Clear bit 9 */
        "btc $10, %[dest]\n\t"          /* Complement bit 10 */
        : [dest] "+m,r" (*dest)
        : 
        : "cc", "memory"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Force STRICT_LOW_PART by modifying only part of a register */
    __asm__ volatile (
        "addw %[src], %[dest]\n\t"      /* 'w' suffix for 16-bit operation */
        : [dest] "+&r,m" (*dest)        /* Early clobber & constraint */
        : [src] "r,m" (src)
        : "cc", "memory"
    );
}

/* Function to test STRICT_LOW_PART with byte operations */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src) {
    /* Byte operation that only affects low part */
    __asm__ volatile (
        "orb %[src], %[dest]\n\t"       /* Byte OR operation */
        : [dest] "+&q,m" (*dest)        /* 'q' constraint for byte-addressable reg */
        : [src] "q,m" (src)
        : "cc", "memory"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access a short within an int through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* This should generate SUBREG(MEM) pattern */
    __asm__ volatile (
        "movw $0xABCD, %[ptr]\n\t"
        : [ptr] "=m" (*short_ptr)
        :
        : "memory"
    );
}

/* Function to test SUBREG of MEM with different sizes */
static void test_subreg_mem_mixed(volatile uint64_t *long_ptr) {
    /* Access different sized subregions of the same memory */
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    volatile uint16_t *short_ptr = (volatile uint16_t *)long_ptr;
    volatile uint8_t *byte_ptr = (volatile uint8_t *)long_ptr;
    
    /* Multiple accesses to create complex SUBREG patterns */
    __asm__ volatile (
        "movl $0xDEADBEEF, %[int]\n\t"
        "movw $0x1234, %[short]\n\t"
        "movb $0xAA, %[byte]\n\t"
        : [int] "=m" (*int_ptr),
          [short] "=m" (*(short_ptr + 1)),  /* Offset to avoid overlap */
          [byte] "=m" (*(byte_ptr + 6))     /* Different offset */
        :
        : "memory"
    );
}

/* Function to test complex pattern combining multiple concepts */
static void test_combined_pattern(volatile uint32_t *data) {
    uint32_t temp;
    
    /* Complex inline assembly that might generate multiple target patterns */
    __asm__ volatile (
        "movl %[data], %[temp]\n\t"
        "andl $0xFFFF0000, %[temp]\n\t"    /* Zero extract high 16 bits */
        "shrl $16, %[temp]\n\t"
        "addw $1, %[temp]\n\t"             /* STRICT_LOW_PART on temp */
        "movw %w[temp], %[out]\n\t"        /* SUBREG store to memory */
        : [out] "=m" (*(volatile uint16_t *)data),
          [temp] "=&r" (temp)
        : [data] "m" (*data)
        : "cc", "memory"
    );
}

/* Driver function */
int main() {
    volatile uint32_t local_int = 0x87654321;
    volatile uint16_t local_short_array[4] = {0x1111, 0x2222, 0x3333, 0x4444};
    volatile uint64_t local_long = 0x1122334455667788ULL;
    
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    test_zero_extract(&global_int, 0x89ABCDEF);
    test_zero_extract_bitfield(&global_int);
    test_zero_extract(&local_int, 0xFEDCBA98);
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    test_strict_low_part(&global_short_array[0], 0x1234);
    test_strict_low_part(&global_short_array[1], 0x5678);
    test_strict_low_part_byte(&global_byte_array[0], 0x42);
    test_strict_low_part_byte(&global_byte_array[1], 0x99);
    
    /* Test SUBREG of MEM patterns */
    printf("Testing SUBREG of MEM patterns...\n");
    test_subreg_mem(&global_int);
    test_subreg_mem_mixed(&global_long);
    test_subreg_mem(&local_int);
    
    /* Test combined pattern */
    printf("Testing combined pattern...\n");
    test_combined_pattern(&local_int);
    
    /* Calculate checksum to prevent dead code elimination */
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
        checksum += local_short_array[i];
    }
    checksum += (uint32_t)local_long;
    checksum += (uint32_t)(local_long >> 32);
    
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
