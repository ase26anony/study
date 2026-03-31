/* 
 * Test program to trigger uncovered lines in resource.cc (lines 282-290)
 * Specifically targets SET_DEST patterns: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, MEM
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force generation of ZERO_EXTRACT and STRICT_LOW_PART */
struct bitfields {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* For SUBREG patterns with memory */
union mixed_access {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Volatile to prevent optimization */
volatile int g_volatile_input = 0;

/* Test 1: Bit-field assignments for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfields *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* These assignments often compile to ZERO_EXTRACT or STRICT_LOW_PART */
        bf->a = (i & 0x7);                    /* 3-bit field */
        bf->b = ((i >> 3) & 0x1F);            /* 5-bit field */
        bf->c = ((i >> 8) & 0xFF);            /* 8-bit field */
        bf->d = ((i >> 16) & 0xFFFF);         /* 16-bit field */
        
        /* Mix with volatile to prevent constant propagation */
        if (g_volatile_input) {
            bf->a ^= 1;
            bf->b ^= 0x10;
        }
    }
}

/* Test 2: Inline assembly with register variables for reload stress */
void test_asm_register_clobber(void) {
    /* Register variables tied to specific registers */
    register uint32_t reg_var asm("r12") = 0x12345678;
    register uint32_t reg_var2 asm("r13") = 0x87654321;
    
    /* Inline assembly with clobbers forces reload pass activity */
    asm volatile (
        "mov %[src], %[dest]\n\t"
        : [dest] "=r" (reg_var2)
        : [src] "r" (reg_var)
        : /* No clobbers - but register tying creates conflicts */
    );
    
    /* Use the result in bit-field context */
    struct bitfields bf_local;
    bf_local.a = reg_var2 & 0x7;      /* Potential STRICT_LOW_PART */
    bf_local.b = (reg_var2 >> 3) & 0x1F;
    
    /* Prevent dead code elimination */
    g_volatile_input = bf_local.a + bf_local.b;
}

/* Test 3: Memory accesses with SUBREG and MEM patterns */
void test_memory_subreg(volatile union mixed_access *mem, int size) {
    for (int i = 0; i < size; i++) {
        /* Whole word access - generates MEM */
        mem[i].word = i * 0x01010101;
        
        /* Sub-word accesses - may generate SUBREG of MEM */
        mem[i].half[0] = (i * 0x101) & 0xFFFF;
        mem[i].byte[2] = (i * 0x11) & 0xFF;
        
        /* Type-punning pointer cast for SUBREG */
        uint32_t *word_ptr = (uint32_t *)&mem[i];
        uint16_t *half_ptr = (uint16_t *)word_ptr;  /* SUBREG pattern */
        
        /* Complex addressing mode */
        half_ptr[1] = half_ptr[0] ^ 0xAAAA;
        
        /* Volatile ensures MEM patterns survive */
        if (g_volatile_input) {
            mem[i].word ^= 0xFF00FF00;
        }
    }
}

/* Test 4: Mixed operations in data-dependent loop */
void test_mixed_operations(struct bitfields *bf_array, 
                          volatile union mixed_access *mem_array,
                          int count) {
    /* Data-dependent index prevents optimization */
    int start = g_volatile_input & 0xF;
    if (start >= count) start = 0;
    
    for (int i = start; i < count; i++) {
        /* Alternate between bit-field and memory ops */
        if (i & 1) {
            /* Bit-field manipulation */
            bf_array[i].a = (mem_array[i].byte[0] & 0x7);
            bf_array[i].b = (mem_array[i].byte[1] & 0x1F);
            
            /* This assignment pattern may use STRICT_LOW_PART */
            bf_array[i].c = bf_array[i].a + bf_array[i].b;
        } else {
            /* Memory sub-word access */
            uint32_t temp = bf_array[i].d;
            mem_array[i].half[0] = temp & 0xFFFF;
            mem_array[i].half[1] = (temp >> 16) & 0xFFFF;
            
            /* Pointer arithmetic with different types */
            uint8_t *byte_ptr = (uint8_t *)&mem_array[i];
            byte_ptr[3] = byte_ptr[0] ^ byte_ptr[1] ^ byte_ptr[2];
        }
        
        /* Volatile read creates data dependency */
        if (g_volatile_input > 1000) {
            break;  /* Early exit prevents loop unrolling */
        }
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command-line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test data */
    struct bitfields *bf_array = 
        (struct bitfields *)calloc(iterations, sizeof(struct bitfields));
    
    volatile union mixed_access *mem_array =
        (volatile union mixed_access *)malloc(iterations * sizeof(union mixed_access));
    
    if (!bf_array || !mem_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Starting tests with %d iterations...\n", iterations);
    
    /* Run tests that should generate target RTL patterns */
    test_bitfield_ops(bf_array, iterations);
    test_asm_register_clobber();
    test_memory_subreg(mem_array, iterations);
    test_mixed_operations(bf_array, mem_array, iterations);
    
    /* Compute checksum to ensure actual execution */
    uint32_t checksum = 0;
    for (int i = 0; i < iterations; i++) {
        checksum += bf_array[i].a + bf_array[i].b + bf_array[i].c + bf_array[i].d;
        checksum += mem_array[i].word & 0xFFFF;
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(bf_array);
    free((void *)mem_array);
    
    return 0;
}
