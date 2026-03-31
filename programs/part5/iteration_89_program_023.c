/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_program test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Test structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_access {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Volatile memory buffer for MEM operations */
volatile uint32_t mem_buffer[256];

/* Global to prevent optimization */
volatile int g_input;

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* These assignments often compile to ZERO_EXTRACT or STRICT_LOW_PART */
        bf->a = (i & 0xF);           /* 4-bit field */
        bf->b = (i * 3) & 0xFF;      /* 8-bit field */
        bf->c = (i * 5) & 0xFFF;     /* 12-bit field */
        bf->d = (i * 7) & 0xFF;      /* 8-bit field */
        
        /* Force dependency between fields */
        bf->a = bf->b & 0xF;
        bf->c = (bf->a << 8) | bf->b;
    }
}

/* Function 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    uint32_t temp;
    
    /* Inline assembly that ties C variable to hard register */
    asm volatile (
        "mov %[temp], %[reg]\n\t"
        "ror %[temp], #8\n\t"
        "mov %[reg], %[temp]"
        : [temp] "=r" (temp), [reg] "+r" (reg_var)
        :
        : "cc"  /* Clobber flags to force reload analysis */
    );
    
    /* Use the register variable with bit-field like operation */
    struct bitfield_struct bf_local;
    bf_local.a = reg_var & 0xF;  /* This may generate STRICT_LOW_PART */
    
    /* Prevent dead code elimination */
    mem_buffer[0] = reg_var;
}

/* Function 3: Mixed-type accesses via union to generate SUBREG */
void test_mixed_type_access(union mixed_access *u, int offset) {
    volatile int idx = offset & 0xFF;
    
    /* Write whole word - generates MEM */
    u->word = 0xDEADBEEF;
    
    /* Write half-word - may generate SUBREG of MEM */
    u->half[1] = 0xCAFE;
    
    /* Write byte - another SUBREG possibility */
    u->byte[idx % 4] = 0x55;
    
    /* Read-modify-write with different type sizes */
    uint16_t temp = u->half[0];
    temp ^= 0x1234;
    u->half[0] = temp;  /* SUBREG store */
}

/* Function 4: Complex memory addressing with volatile */
void test_complex_mem_addressing(int iterations) {
    volatile uint32_t *ptr = mem_buffer;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex addressing mode */
        uint32_t idx = (i * g_input) & 0xFF;
        
        /* MEM operations with different access sizes */
        ptr[idx] = i * 0x1001;                     /* Full word MEM */
        *((volatile uint16_t *)(ptr + idx)) = i;   /* SUBREG of MEM */
        
        /* Pointer arithmetic with type punning */
        volatile uint8_t *byte_ptr = (volatile uint8_t *)(ptr + idx);
        byte_ptr[1] = (i >> 8) & 0xFF;            /* Another SUBREG */
    }
}

/* Function 5: Combined test with data-dependent control flow */
void test_combined(int seed) {
    struct bitfield_struct bf;
    union mixed_access u;
    volatile int condition = seed & 1;
    
    /* Initialize */
    bf.a = 0; bf.b = 0; bf.c = 0; bf.d = 0;
    u.word = 0;
    
    /* Data-dependent execution path */
    if (condition) {
        test_bitfield_ops(&bf, 10);
        test_asm_clobbers();
    } else {
        test_mixed_type_access(&u, seed);
        test_complex_mem_addressing(5);
    }
    
    /* Cross-test to ensure both paths get some coverage */
    test_bitfield_ops(&bf, 3);
    test_mixed_type_access(&u, seed + 1);
    
    /* Store results to prevent optimization */
    mem_buffer[255] = bf.a + bf.b + bf.c + bf.d;
    mem_buffer[254] = u.word;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    g_input = iterations;
    
    printf("Starting RTL pattern generation tests...\n");
    
    /* Test 1: Pure bit-field operations */
    struct bitfield_struct bf1;
    test_bitfield_ops(&bf1, iterations % 50);
    
    /* Test 2: Assembly with register variables */
    test_asm_clobbers();
    
    /* Test 3: Type-punning with unions */
    union mixed_access u1;
    test_mixed_type_access(&u1, iterations);
    
    /* Test 4: Complex memory accesses */
    test_complex_mem_addressing(iterations % 20);
    
    /* Test 5: Combined with data-dependent paths */
    test_combined(iterations);
    
    /* Verify some results */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Bitfield test: a=%u, b=%u, c=%u, d=%u\n", 
           bf1.a, bf1.b, bf1.c, bf1.d);
    printf("Union test: word=0x%08X\n", u1.word);
    
    return 0;
}
