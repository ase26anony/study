/* Compile with: gcc -O2 -fdump-rtl-all -fdump-tree-all -o test_program test.c */
/* Also try: gcc -O3 -fno-strict-aliasing -frename-registers -o test_program test.c */
/* And: gcc -O1 -fschedule-insns -fno-merge-bitfields -o test_program test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
static volatile int g_volatile_input = 0;

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

/* Packed structure for SUBREG/MEM patterns */
struct __attribute__((packed)) packed_struct {
    char x;
    int y;
    short z;
};

/* Union for type-punning to generate SUBREG */
union type_pun {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* Global with bit-field to force memory operations */
static struct bitfield_struct g_bf;
/* Volatile memory buffer */
static volatile uint32_t g_mem_buffer[256];

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(int idx) {
    struct bitfield_struct local_bf;
    
    /* Data-dependent assignments */
    if (idx & 1) {
        local_bf.a = (idx & 0x7);          /* 3-bit field */
    }
    if (idx & 2) {
        local_bf.b = (idx & 0x1F);         /* 5-bit field */
    }
    if (idx & 4) {
        local_bf.c = (idx & 0xFF);         /* 8-bit field */
    }
    if (idx & 8) {
        local_bf.d = (idx & 0xFFFF);       /* 16-bit field */
    }
    
    /* Copy to global (might generate MEM with bit-field store) */
    g_bf = local_bf;
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_inline_asm(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    uint32_t temp;
    
    /* Inline assembly that uses and clobbers hard register */
    asm volatile (
        "mov %[reg], %[temp]\n\t"
        "ror $8, %[temp]\n\t"
        "mov %[temp], %[reg]"
        : [temp] "=r" (temp), [reg] "+r" (reg_var)
        :
        : "cc"
    );
    
    /* Use the register variable with bit-field */
    struct bitfield_struct bf2;
    bf2.a = (reg_var & 0x7);      /* This may generate STRICT_LOW_PART */
    bf2.b = ((reg_var >> 3) & 0x1F);
    
    g_bf.a = bf2.a;
    g_bf.b = bf2.b;
}

/* Test 3: Memory operations with volatile and type-punning for SUBREG/MEM */
void test_mem_ops(int idx) {
    union type_pun pun;
    volatile uint32_t *volatile_ptr = &g_mem_buffer[idx % 256];
    
    /* Initialize */
    pun.full = idx * 0x01010101;
    
    /* Write through different-sized views (may generate SUBREG) */
    pun.bytes[1] = idx & 0xFF;
    pun.half[0] = (pun.half[0] & 0xFF00) | (idx & 0xFF);
    
    /* Volatile memory write (MEM RTL) */
    *volatile_ptr = pun.full;
    
    /* Type-punned pointer cast for SUBREG of MEM */
    uint16_t *half_ptr = (uint16_t *)volatile_ptr;
    half_ptr[1] = (idx * 37) & 0xFFFF;  /* SUBREG of MEM */
}

/* Test 4: Complex addressing mode with packed struct */
void test_packed_struct(int idx) {
    static struct packed_struct packed_array[4];
    
    /* Data-dependent index */
    int i = idx % 4;
    
    /* Access packed struct members (likely generates SUBREG) */
    packed_array[i].x = (char)(idx & 0xFF);
    packed_array[i].y = idx * 2;
    packed_array[i].z = (short)(idx & 0xFFFF);
    
    /* Volatile read-modify-write */
    volatile struct packed_struct *vol_packed = 
        (volatile struct packed_struct *)&packed_array[i];
    vol_packed->z = (vol_packed->z + 1) & 0x7FFF;
}

/* Test 5: Loop with mixed operations to ensure RTL patterns survive */
void test_loop_mixed_ops(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Mix all patterns in a loop */
        test_bitfield_ops(i);
        test_mem_ops(i);
        
        /* Every 8 iterations, do inline asm */
        if ((i & 7) == 0) {
            test_inline_asm();
        }
        
        /* Every 16 iterations, do packed struct */
        if ((i & 15) == 0) {
            test_packed_struct(i);
        }
    }
}

/* Main function with runtime-dependent control flow */
int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command-line argument or default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize with volatile to prevent optimization */
    g_volatile_input = iterations;
    
    printf("Starting resource tracking test with %d iterations\n", iterations);
    
    /* Run the main test */
    test_loop_mixed_ops(iterations);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum ^= g_bf.a + g_bf.b + g_bf.c + g_bf.d;
    
    for (int i = 0; i < 256; i++) {
        checksum ^= g_mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
