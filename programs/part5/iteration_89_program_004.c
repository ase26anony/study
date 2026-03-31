/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_vol_input = 0;

/* ========== Test 1: Bit-field assignments for ZERO_EXTRACT/STRICT_LOW_PART ========== */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

/* Register variable combined with bit-field manipulation */
register uint32_t reg_var asm ("r12");

void test_bitfields(struct bitfield_struct *bf, int n) {
    for (int i = 0; i < n; i++) {
        /* These assignments often generate ZERO_EXTRACT or STRICT_LOW_PART in RTL */
        bf->a = (i & 0x7);                    /* 3-bit field */
        bf->b = ((i >> 3) & 0x1F);            /* 5-bit field */
        bf->c = ((i >> 8) & 0xFF);            /* 8-bit field */
        bf->d = ((i >> 16) & 0xFFFF);         /* 16-bit field */
        
        /* Mix with register variable to stress reload */
        reg_var = bf->d;
        bf->c = reg_var & 0xFF;
    }
}

/* ========== Test 2: SUBREG and MEM patterns via type-punning ========== */
union mixed_types {
    uint32_t full;
    uint16_t half[2];
    uint8_t  byte[4];
};

void test_subreg_mem(volatile union mixed_types *mem, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Write full 32-bit (MEM) */
        mem->full = i * 0x01010101;
        
        /* Write 16-bit subword (likely SUBREG of MEM) */
        mem->half[1] = (i & 0xFFFF) ^ 0xAAAA;
        
        /* Write 8-bit subword (another SUBREG) */
        mem->byte[0] = (i & 0xFF) ^ 0x55;
        
        /* Complex addressing with volatile */
        volatile uint16_t *ptr = (volatile uint16_t *)&mem->byte[1];
        *ptr = (i & 0xFF) << 8;
    }
}

/* ========== Test 3: Inline assembly with clobbers for reload stress ========== */
void test_asm_clobber(void) {
    uint32_t var1 = 0x12345678;
    uint32_t var2 = 0x9ABCDEF0;
    
    /* Inline asm that ties C variables to hard registers */
    asm volatile (
        "mov %[v1], %%r10\n\t"
        "mov %[v2], %%r11\n\t"
        "add %%r10, %%r11\n\t"
        "mov %%r11, %[out]"
        : [out] "=r" (var1)
        : [v1] "r" (var1), [v2] "r" (var2)
        : "r10", "r11", "cc"
    );
    
    /* Use result to prevent dead code elimination */
    g_vol_input = var1;
}

/* ========== Test 4: Complex memory addressing modes ========== */
struct packed_data {
    uint8_t  header;
    uint32_t data[8];
    uint16_t footer;
} __attribute__((packed));

void test_complex_mem(struct packed_data *pd, int idx) {
    /* Data-dependent index prevents optimization */
    volatile int i = g_vol_input & 7;
    
    /* MEM with complex address: base + index*scale + displacement */
    pd->data[i] = idx * 0x1001;
    
    /* Another MEM with different addressing */
    ((volatile uint16_t *)&pd->data[2])[i] = idx & 0xFFFF;
    
    /* SUBREG from pointer cast */
    *(volatile uint8_t *)((char *)pd->data + i) = idx & 0xFF;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    /* Initialize test structures */
    struct bitfield_struct bf = {0};
    union mixed_types mem_area = {0};
    struct packed_data pd = {0};
    
    printf("Testing resource.cc lines 282-290 patterns...\n");
    printf("Iterations: %d\n", iterations);
    
    /* Run tests that should generate target RTL patterns */
    test_bitfields(&bf, iterations);
    test_subreg_mem(&mem_area, iterations);
    test_asm_clobber();
    test_complex_mem(&pd, iterations);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = bf.a + bf.b + bf.c + bf.d;
    checksum += mem_area.full;
    checksum += pd.data[0] + pd.header + pd.footer;
    checksum += g_vol_input;
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
