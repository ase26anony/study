#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Test structures with bit-fields */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

struct mixed_bitfields {
    volatile unsigned int x : 3;
    unsigned int y : 5;
    volatile unsigned int z : 10;
    unsigned int w : 14;
};

/* Union for type-punning to generate SUBREG */
union type_punner {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
    volatile uint32_t vword;
};

/* Global volatile to prevent optimization */
volatile int g_volatile = 0;

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* These assignments should generate ZERO_EXTRACT or STRICT_LOW_PART */
        bf->a = (i & 0xF);           /* 4-bit field */
        bf->b = (i * 3) & 0xFF;      /* 8-bit field */
        bf->c = (i * 5) & 0xFFF;     /* 12-bit field */
        bf->d = (i * 7) & 0xFF;      /* 8-bit field */
        
        /* Mix with volatile to prevent optimization */
        if (g_volatile) {
            bf->a ^= 0xF;
        }
    }
}

/* Function 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobbers(union type_punner *tp) {
    uint32_t temp;
    
    /* Inline assembly that ties C variable to specific register */
    register uint32_t reg_var asm("r12") = tp->word;
    
    asm volatile (
        "mov %[input], %[output]\n\t"
        "add $0x1234, %[output]\n\t"
        : [output] "=r" (temp)
        : [input] "r" (reg_var)
        : "r12", "cc"
    );
    
    /* Use the result in bit-field operations */
    struct mixed_bitfields mbf;
    mbf.x = temp & 0x7;          /* 3-bit field */
    mbf.z = (temp >> 3) & 0x3FF; /* 10-bit field */
    
    tp->word = temp;
}

/* Function 3: Memory operations with type-punning for SUBREG generation */
void test_mem_subreg(union type_punner *mem, int size) {
    volatile uint8_t *byte_ptr = (volatile uint8_t *)mem;
    volatile uint16_t *half_ptr = (volatile uint16_t *)mem;
    
    for (int i = 0; i < size; i++) {
        /* Mixed-size accesses to generate SUBREG of MEM */
        byte_ptr[i] = (i * 11) & 0xFF;
        
        if (i % 2 == 0) {
            /* Access as 16-bit to create SUBREG patterns */
            half_ptr[i/2] = (i * 13) & 0xFFFF;
        }
        
        /* Complex addressing with volatile */
        *(volatile uint32_t *)(byte_ptr + i) = 
            *(volatile uint32_t *)(byte_ptr + i) ^ 0xAA55AA55;
    }
}

/* Function 4: Complex memory addressing for MEM_P patterns */
void test_complex_mem_addressing(int *buffer, int size, int offset) {
    volatile int *volatile_ptr = buffer;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing mode */
        int idx = (i + offset) % size;
        
        /* This should generate MEM with complex address */
        volatile_ptr[idx] = volatile_ptr[idx] * 3 + i;
        
        /* Additional memory operation with pointer arithmetic */
        *(buffer + ((i * 7) % size)) ^= 0xDEADBEEF;
    }
}

/* Function 5: Mixed operations combining all patterns */
void test_mixed_operations(struct bitfield_struct *bf, 
                          union type_punner *tp, 
                          int *mem_buffer,
                          int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Bit-field operations */
        bf->b = (bf->a + i) & 0xFF;
        
        /* Memory operations with type-punning */
        tp->half[0] = tp->byte[1] | (tp->byte[2] << 8);
        
        /* Complex memory addressing */
        mem_buffer[(i * 3) % 16] = bf->c + tp->word;
        
        /* Inline assembly stress */
        register int asm_var asm("ebx") = i;
        int result;
        asm volatile (
            "lea (%[in], %[in], 2), %[out]\n\t"
            : [out] "=r" (result)
            : [in] "r" (asm_var)
            : "ebx"
        );
        
        /* Use result in volatile memory access */
        *(volatile int *)(mem_buffer + (i % 4)) = result;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Initialize test data */
    struct bitfield_struct bf = {0};
    union type_punner tp = {0};
    int mem_buffer[16] = {0};
    
    /* Initialize with some data */
    for (int i = 0; i < 16; i++) {
        mem_buffer[i] = i * i;
    }
    tp.word = 0x12345678;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run tests that should generate specific RTL patterns */
    test_bitfield_ops(&bf, iterations);
    printf("Bitfield test completed: a=%u, b=%u, c=%u, d=%u\n", 
           bf.a, bf.b, bf.c, bf.d);
    
    test_asm_clobbers(&tp);
    printf("Assembly clobber test completed: word=0x%08x\n", tp.word);
    
    test_mem_subreg(&tp, 8);
    printf("Memory SUBREG test completed: bytes=[%02x %02x %02x %02x]\n",
           tp.byte[0], tp.byte[1], tp.byte[2], tp.byte[3]);
    
    test_complex_mem_addressing(mem_buffer, 16, 3);
    printf("Complex memory addressing test completed\n");
    
    test_mixed_operations(&bf, &tp, mem_buffer, iterations);
    printf("Mixed operations test completed\n");
    
    /* Calculate checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum ^= mem_buffer[i];
    }
    checksum ^= tp.word;
    checksum ^= (bf.a | (bf.b << 4) | (bf.c << 12) | (bf.d << 24));
    
    printf("Final checksum: 0x%08x\n", checksum);
    
    return 0;
}
