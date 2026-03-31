#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Test structures with bit-fields for ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union type_pun {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Volatile memory buffer for MEM operations */
volatile uint32_t mem_buffer[256];

/* Global to prevent optimization */
volatile int g_input = 0;

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Multiple bit-field assignments */
        bf->a = (i & 0xF);                    /* 4-bit field */
        bf->b = ((i * 3) & 0xFF);             /* 8-bit field */
        bf->c = ((i * 5) & 0xFFF);            /* 12-bit field */
        bf->d = ((i * 7) & 0xFF);             /* 8-bit field */
        
        /* Mix with volatile to prevent optimization */
        bf->a ^= g_input & 0xF;
    }
}

/* Function 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    uint32_t temp;
    
    /* Inline assembly that clobbers registers */
    asm volatile (
        "mov %[reg], %[temp]\n\t"
        "ror $8, %[temp]\n\t"
        "mov %[temp], %[reg]"
        : [temp] "=r" (temp), [reg] "+r" (reg_var)
        :
        : "cc"  /* Clobber flags to force reload */
    );
    
    /* Use the result in a bit-field context */
    struct bitfield_struct bf_local;
    bf_local.a = reg_var & 0xF;
    bf_local.b = (reg_var >> 4) & 0xFF;
}

/* Function 3: Mixed-type accesses for SUBREG generation */
void test_mixed_type_access(union type_pun *tp, int offset) {
    /* Write as word, read as smaller types */
    tp->word = 0xDEADBEEF;
    
    /* SUBREG generation through type-punning */
    uint16_t half_val = tp->half[offset & 1];
    uint8_t byte_val = tp->byte[(offset * 3) & 3];
    
    /* Write back through different-sized access */
    tp->half[1] = half_val ^ byte_val;
    
    /* Cast pointer for SUBREG of MEM */
    *(volatile uint8_t *)(&tp->word) = byte_val;
}

/* Function 4: Volatile memory operations for MEM RTL */
void test_volatile_mem(int iterations) {
    volatile uint32_t *ptr = mem_buffer;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex addressing mode */
        uint32_t idx = (i * 17) & 0xFF;
        
        /* MEM operations with volatile */
        ptr[idx] = ptr[(idx + 1) & 0xFF] ^ 0x12345678;
        
        /* SUBREG of MEM through byte access */
        *(volatile uint8_t *)(&ptr[idx]) = i & 0xFF;
    }
}

/* Function 5: Combined operations in data-dependent loop */
void test_combined_ops(struct bitfield_struct *bf, union type_pun *tp, int count) {
    for (int i = 0; i < count; i++) {
        /* Data-dependent branching */
        if (g_input & (1 << (i & 3))) {
            /* Bit-field path */
            bf->a = i & 0xF;
            bf->b = (bf->b + 1) & 0xFF;
        } else {
            /* Memory path */
            mem_buffer[i & 0xFF] = tp->word;
            
            /* SUBREG memory access */
            uint8_t val = *(volatile uint8_t *)(&mem_buffer[i & 0xFF]);
            tp->byte[i & 3] = val;
        }
        
        /* Inline asm barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with runtime value to prevent constant propagation */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Initialize test data */
    struct bitfield_struct bf = {0};
    union type_pun tp;
    
    printf("Testing RTL pattern generation for resource tracking...\n");
    
    /* Run test functions */
    test_bitfield_ops(&bf, iterations);
    test_asm_clobbers();
    test_mixed_type_access(&tp, iterations);
    test_volatile_mem(iterations / 10);
    test_combined_ops(&bf, &tp, iterations);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum ^= bf.a + (bf.b << 4) + (bf.c << 12) + (bf.d << 24);
    checksum ^= tp.word;
    checksum ^= mem_buffer[0] ^ mem_buffer[255];
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
