#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
volatile int g_volatile_input = 0;

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* These assignments should generate ZERO_EXTRACT or STRICT_LOW_PART */
        bf->a = (i & 0xF);           /* 4-bit field */
        bf->b = (i * 3) & 0xFF;      /* 8-bit field */
        bf->c = (i * 5) & 0xFFF;     /* 12-bit field */
        bf->d = (i * 7) & 0xFF;      /* 8-bit field */
        
        /* Volatile read to prevent optimization */
        if (g_volatile_input) {
            bf->a = g_volatile_input & 0xF;
        }
    }
}

/* Function 2: Mixed bit-fields with volatile */
void test_mixed_bitfields(struct mixed_bitfields *mb, int count) {
    for (int i = 0; i < count; i++) {
        /* Mix of volatile and non-volatile bit-field accesses */
        mb->x = (i & 0x7);
        mb->y = ((i + 1) & 0x1F);
        mb->z = ((i * 2) & 0x3FF);
        mb->w = ((i * 3) & 0x3FFF);
        
        /* Data-dependent access pattern */
        if (i % 3 == 0) {
            mb->x = g_volatile_input & 0x7;
        }
    }
}

/* Function 3: Inline assembly with register variables and clobbers */
void test_asm_register_clobber(void) {
    /* Register variables tied to specific registers */
    register uint32_t reg_var1 asm("r12") = 0x12345678;
    register uint32_t reg_var2 asm("r13") = 0x87654321;
    uint32_t result;
    
    /* Inline assembly that clobbers registers */
    asm volatile (
        "mov %[reg1], %[out]\n\t"
        "xor %[reg2], %[out]\n\t"
        : [out] "=r" (result)
        : [reg1] "r" (reg_var1), [reg2] "r" (reg_var2)
        : "cc" /* Clobber condition codes */
    );
    
    /* Use result to prevent dead code elimination */
    if (g_volatile_input) {
        printf("ASM result: 0x%08x\n", result);
    }
}

/* Function 4: Type-punning and sub-word memory accesses */
void test_subreg_mem_access(union type_punner *data, int size) {
    volatile uint8_t *volatile_ptr = (volatile uint8_t *)&data->word;
    
    for (int i = 0; i < size; i++) {
        /* SUBREG generation: accessing parts of larger word */
        data->half[i % 2] = (uint16_t)(i * 11);
        data->byte[(i + 1) % 4] = (uint8_t)(i * 13);
        
        /* Volatile memory access - should generate MEM RTL */
        volatile_ptr[(i + 2) % 4] = (uint8_t)(i * 17);
        
        /* Complex addressing mode */
        uint32_t *ptr = &data->word + (i & 1);
        *ptr = *ptr ^ 0x00FF00FF;
    }
}

/* Function 5: Pointer casting for SUBREG of MEM */
void test_pointer_casting(void *buffer, int len) {
    volatile char *vol_buf = (volatile char *)buffer;
    uint32_t *word_ptr = (uint32_t *)buffer;
    
    for (int i = 0; i < len - 3; i++) {
        /* Write char to int location - may generate SUBREG */
        *((volatile uint8_t *)(word_ptr + (i % 2))) = (uint8_t)(i * 19);
        
        /* Volatile memory access */
        vol_buf[i] = vol_buf[i] ^ 0x55;
        
        /* Access short within long via pointer arithmetic */
        uint16_t *short_ptr = (uint16_t *)((char *)buffer + i);
        *short_ptr = (uint16_t)(*short_ptr + i);
    }
}

/* Function 6: Complex addressing modes */
void test_complex_addressing(int *array, int size) {
    volatile int *vol_array = (volatile int *)array;
    
    for (int i = 0; i < size; i++) {
        /* Array access with data-dependent index */
        int idx = (i + g_volatile_input) % size;
        
        /* Memory write with complex addressing - should generate MEM RTL */
        vol_array[idx] = vol_array[idx] * 2 + 1;
        
        /* Pointer with offset */
        int *ptr = array + ((i * 3) % size);
        *ptr = *ptr ^ 0xAAAAAAAA;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Initialize test data */
    struct bitfield_struct bf_data = {0};
    struct mixed_bitfields mixed_bf = {0};
    union type_punner pun_data = {0};
    int buffer[64];
    int array[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) buffer[i] = i;
    for (int i = 0; i < 32; i++) array[i] = i * 2;
    
    /* Set volatile input from command line or time */
    g_volatile_input = iterations;
    
    printf("Testing RTL pattern generation for resource tracking...\n");
    
    /* Execute test functions */
    test_bitfield_ops(&bf_data, iterations % 50 + 1);
    test_mixed_bitfields(&mixed_bf, iterations % 40 + 1);
    test_asm_register_clobber();
    test_subreg_mem_access(&pun_data, iterations % 30 + 1);
    test_pointer_casting(buffer, 64);
    test_complex_addressing(array, 32);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum ^= bf_data.a + bf_data.b + bf_data.c + bf_data.d;
    checksum ^= mixed_bf.x + mixed_bf.y + mixed_bf.z + mixed_bf.w;
    checksum ^= pun_data.word;
    
    for (int i = 0; i < 64; i++) checksum ^= buffer[i];
    for (int i = 0; i < 32; i++) checksum ^= array[i];
    
    printf("Final checksum: 0x%08x\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
