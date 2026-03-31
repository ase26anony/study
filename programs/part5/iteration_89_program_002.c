/* 
 * Test program targeting uncovered lines in GCC's resource.cc
 * Specifically aims to trigger mark_set_resources paths for:
 * - ZERO_EXTRACT / STRICT_LOW_PART (bit-field assignments)
 * - SUBREG of MEM (subword memory operations)
 * - MEM_P (memory writes with complex addressing)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* ========== Bit-field structures for ZERO_EXTRACT/STRICT_LOW_PART ========== */

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_access {
    uint32_t full;
    uint16_t half[2];
    uint8_t byte[4];
};

/* ========== Test Functions ========== */

/* 
 * Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART
 * Multiple assignments with volatile control flow
 */
void test_bitfield_ops(void) {
    struct bitfield_struct bf = {0};
    volatile int control = g_volatile_input;
    
    /* Loop prevents optimization, creates multiple SETs in RTL */
    for (int i = 0; i < 10; i++) {
        /* Data-dependent assignments */
        if (control & (1 << i)) {
            bf.a = (i & 0x7);          /* 3-bit field */
            bf.b = (i & 0x1F);         /* 5-bit field */
        } else {
            bf.c = (i & 0xFF);         /* 8-bit field */
            bf.d = (i & 0xFFFF);       /* 16-bit field */
        }
        
        /* Mix with memory barrier to force RTL generation */
        asm volatile("" ::: "memory");
    }
    
    /* Use result to prevent dead code elimination */
    printf("Bitfield checksum: %u\n", 
           (unsigned int)(bf.a + bf.b + bf.c + bf.d));
}

/*
 * Test 2: Inline assembly with clobbers to stress reload pass
 * Ties C variables to specific hardware registers
 */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct bf = {0};
    
    /* Inline asm that clobbers hard register */
    asm volatile (
        "mov %[reg], %[out]\n\t"
        : [out] "=r" (bf.d)            /* Output to bit-field */
        : [reg] "r" (reg_var)          /* Input from register variable */
        : "r12"                        /* Explicit clobber */
    );
    
    /* Additional bit-field assignment after clobber */
    bf.a = (reg_var & 0x7);
    bf.b = (reg_var >> 3) & 0x1F;
    
    printf("ASM clobber result: a=%u, b=%u, d=%u\n", 
           bf.a, bf.b, bf.d);
}

/*
 * Test 3: Subword memory operations for SUBREG of MEM
 * Uses volatile pointers and type-punning
 */
void test_subreg_mem(void) {
    volatile uint32_t memory[4] = {0};
    union mixed_access *pun = (union mixed_access *)&memory[0];
    
    /* Write via different-sized views */
    for (int i = 0; i < 4; i++) {
        /* SUBREG generation: writing 8/16-bit to 32-bit location */
        pun->byte[i] = (uint8_t)(g_volatile_input + i);
        pun->half[i % 2] = (uint16_t)(g_volatile_input * i);
        
        /* Complex addressing mode for MEM_P */
        *(volatile uint32_t *)((char *)memory + i * sizeof(uint32_t)) = 
            (uint32_t)(i * 0x1001);
    }
    
    /* Read back to verify */
    uint32_t sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += memory[i];
    }
    printf("Memory sum: 0x%08x\n", sum);
}

/*
 * Test 4: Mixed-type accesses with volatile and loops
 * Forces generation of MEM RTL patterns
 */
void test_mixed_mem_access(void) {
    volatile uint8_t buffer[64];
    volatile uint32_t *word_ptr = (volatile uint32_t *)buffer;
    volatile uint16_t *half_ptr = (volatile uint16_t *)buffer;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        buffer[i] = (uint8_t)(i ^ 0x55);
    }
    
    /* Data-dependent memory updates */
    int limit = (g_volatile_input % 16) + 4;
    for (int i = 0; i < limit; i++) {
        /* Different access sizes create SUBREG RTL */
        half_ptr[i] = (uint16_t)(half_ptr[i] + i);
        word_ptr[i % 8] = word_ptr[i % 8] ^ (uint32_t)(i << 8);
        
        /* Volatile ensures MEM RTL generation */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum */
    uint32_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += buffer[i];
    }
    printf("Buffer checksum: 0x%08x\n", checksum);
}

/*
 * Test 5: Complex addressing modes for MEM_P
 * Array indexing with multiple dimensions and offsets
 */
void test_complex_addressing(void) {
    volatile int array[4][8][2];
    int idx1, idx2, idx3;
    
    /* Volatile indices prevent optimization */
    volatile int v = g_volatile_input;
    idx1 = (v >> 0) & 0x3;
    idx2 = (v >> 2) & 0x7;
    idx3 = (v >> 5) & 0x1;
    
    /* Complex addressing: array[idx1][idx2][idx3] */
    array[idx1][idx2][idx3] = v * 2;
    
    /* More complex: array[(v+1)&3][(v>>1)&7][v&1] */
    array[(v+1)&3][(v>>1)&7][v&1] = v * 3;
    
    /* Pointer arithmetic with scaling */
    volatile int *ptr = &array[0][0][0];
    ptr[(idx1 * 16) + (idx2 * 2) + idx3] += 1;
    
    printf("Array[%d][%d][%d] = %d\n", 
           idx1, idx2, idx3, array[idx1][idx2][idx3]);
}

/* ========== Main Function ========== */

int main(int argc, char **argv) {
    /* Use command-line argument for variability */
    if (argc > 1) {
        g_volatile_input = atoi(argv[1]);
    } else {
        g_volatile_input = 42;  /* Default seed */
    }
    
    printf("Testing with input: %d\n\n", g_volatile_input);
    
    /* Execute all test patterns */
    test_bitfield_ops();
    test_asm_clobber();
    test_subreg_mem();
    test_mixed_mem_access();
    test_complex_addressing();
    
    return 0;
}
