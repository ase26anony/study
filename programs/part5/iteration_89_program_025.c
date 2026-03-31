/* 
 * Test program targeting uncovered lines in resource.cc (lines 282-290)
 * Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_rtl test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_volatile_input = 0;

/* ========== BIT-FIELD STRUCTURES ========== */
/* These generate ZERO_EXTRACT/STRICT_LOW_PART in RTL */

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

/* Mixed bitfields to force complex extraction */
struct mixed_bitfields {
    volatile unsigned int x : 7;  /* volatile prevents optimization */
    unsigned int y : 9;
    unsigned int z : 4;
    unsigned int w : 12;
};

/* ========== UNION FOR TYPE PUNNING ========== */
/* Generates SUBREG patterns */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } halves;
    uint8_t bytes[4];
};

/* ========== TEST FUNCTIONS ========== */

/* Test 1: Bitfield assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent assignments prevent optimization */
        bf->a = (i + g_volatile_input) & 0x7;
        bf->b = (i * 3) & 0x1F;
        bf->c = (i + g_volatile_input * 2) & 0xFF;
        bf->d = (i * 5) & 0xFFFF;
    }
}

/* Test 2: Mixed bitfield with volatile member */
void test_mixed_bitfields(struct mixed_bitfields *mb, int idx) {
    /* Complex addressing with volatile */
    mb[idx].x = (g_volatile_input + idx) & 0x7F;
    mb[idx].y = (idx * 7) & 0x1FF;
    mb[idx].z = (g_volatile_input - idx) & 0xF;
    mb[idx].w = (idx * 11) & 0xFFF;
}

/* Test 3: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = g_volatile_input;
    register uint32_t result asm("r11") = 0;
    
    /* Inline asm that forces register allocation and clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (reg_var)
        : "%eax", "cc"
    );
    
    /* Use result in bitfield to combine patterns */
    struct bitfield_struct bf_local;
    bf_local.a = result & 0x7;
    bf_local.b = (result >> 3) & 0x1F;
}

/* Test 4: SUBREG generation via type punning and memory access */
void test_subreg_mem(volatile union type_pun *mem, int size) {
    for (int i = 0; i < size; i++) {
        /* Access different views of the same memory */
        mem[i].halves.low = g_volatile_input + i;
        mem[i].halves.high = g_volatile_input - i;
        
        /* Byte access generates SUBREG of MEM */
        mem[i].bytes[0] = (i * 2) & 0xFF;
        mem[i].bytes[3] = (i * 3) & 0xFF;
    }
}

/* Test 5: Complex MEM addressing modes */
void test_complex_mem_addr(volatile uint32_t *base, int offset) {
    /* Complex address calculation */
    volatile uint32_t *ptr = base + (offset * g_volatile_input);
    
    /* Multiple memory writes with different patterns */
    for (int i = 0; i < 4; i++) {
        ptr[i] = g_volatile_input * i;
        *(ptr + i + 8) = g_volatile_input + (i << 2);
    }
    
    /* Pointer arithmetic with type conversion */
    volatile uint16_t *short_ptr = (volatile uint16_t *)ptr;
    short_ptr[5] = g_volatile_input & 0xFFFF;  /* SUBREG of MEM */
}

/* Test 6: Combined patterns in loop with data-dependent control */
void test_combined_patterns(int iterations) {
    struct bitfield_struct bf_array[8];
    volatile union type_pun mem_buffer[16];
    
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent index prevents optimization */
        int idx = (i + g_volatile_input) & 0x7;
        
        /* Mix different patterns */
        test_bitfield_ops(&bf_array[idx], 1);
        
        if (i & 1) {
            test_subreg_mem(&mem_buffer[idx], 2);
        } else {
            /* Memory access with complex addressing */
            test_complex_mem_addr((volatile uint32_t *)mem_buffer, idx);
        }
        
        /* Inline asm every 4th iteration */
        if ((i % 4) == 0) {
            test_asm_clobber();
        }
    }
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    int iterations = 100;
    
    /* Use command line argument for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Testing RTL pattern generation for resource.cc lines 282-290\n");
    printf("Iterations: %d\n", iterations);
    
    /* Initialize test structures */
    struct bitfield_struct bf_test;
    struct mixed_bitfields mb_test[4];
    union type_pun mem_test[16];
    volatile uint32_t mem_base[32];
    
    /* Initialize volatile memory */
    for (int i = 0; i < 32; i++) {
        mem_base[i] = i * 0x100;
    }
    
    /* Run individual tests */
    test_bitfield_ops(&bf_test, iterations);
    test_mixed_bitfields(mb_test, iterations & 0x3);
    test_asm_clobber();
    test_subreg_mem((volatile union type_pun *)mem_test, 8);
    test_complex_mem_addr(mem_base, 2);
    
    /* Run combined test */
    test_combined_patterns(iterations);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    checksum ^= bf_test.a + bf_test.b + bf_test.c + bf_test.d;
    checksum ^= mb_test[0].x + mb_test[0].y;
    checksum ^= mem_test[0].full;
    checksum ^= mem_base[0];
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
