/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force runtime values to prevent constant propagation */
static volatile int g_volatile_value = 0;

/* ========== Bit-field structures for ZERO_EXTRACT/STRICT_LOW_PART ========== */
struct bitfield_packed {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

struct bitfield_mixed {
    volatile unsigned int low : 4;
    unsigned int high : 12;
    unsigned int pad : 16;
};

/* ========== Union for type-punning SUBREG generation ========== */
union subreg_access {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* ========== Test functions ========== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_packed *bp, struct bitfield_mixed *bm, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Complex bit-field assignment with volatile source */
        bp->a = (g_volatile_value + i) & 0x7;
        bp->b = (bp->a << 2) | 0x1;
        bp->c = bp->b * 3;
        bp->d = bp->c + (i << 8);
        
        /* Mixed volatile/non-volatile bit-field access */
        bm->low = (i & 0xF);
        bm->high = bm->low * 256;
    }
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(union subreg_access *mem) {
    register uint32_t reg_var asm("r12") = mem->word;
    register uint16_t reg_half asm("r13") = 0;
    
    /* Move between registers with clobber */
    asm volatile (
        "movw %w1, %w0\n\t"
        "rorl $8, %0\n\t"
        : "+r" (reg_var), "=r" (reg_half)
        : 
        : "cc"
    );
    
    /* Force STRICT_LOW_PART-like behavior */
    mem->half[1] = reg_half;
    
    /* Additional clobber to force resource tracking */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r" (reg_var)
        : "r" (g_volatile_value)
        : "cc"
    );
    
    mem->word = reg_var;
}

/* Test 3: Memory accesses with SUBREG and MEM patterns */
void test_mem_subreg(volatile uint32_t *mem_base, int offset) {
    volatile uint16_t *half_ptr = (volatile uint16_t *)(mem_base + offset);
    volatile uint8_t *byte_ptr = (volatile uint8_t *)(mem_base + offset * 2);
    
    /* SUBREG of MEM pattern */
    *half_ptr = (uint16_t)g_volatile_value;
    
    /* Different sized accesses to same location */
    union {
        volatile uint32_t *word;
        volatile uint16_t *half;
    } pun;
    pun.word = mem_base;
    
    /* This should generate SUBREG RTL */
    pun.half[1] = pun.half[0] + 1;
    
    /* Complex addressing mode */
    byte_ptr[g_volatile_value & 3] = (uint8_t)(offset);
}

/* Test 4: Mixed-type pointer casting for SUBREG generation */
void test_mixed_type_access(void *buffer, size_t size) {
    uint32_t *word_ptr = (uint32_t *)buffer;
    uint16_t *half_ptr = (uint16_t *)buffer;
    uint8_t *byte_ptr = (uint8_t *)buffer;
    
    for (size_t i = 0; i < size / 4; i++) {
        /* Write as 32-bit, read as smaller parts */
        word_ptr[i] = (uint32_t)(g_volatile_value + i);
        
        /* Access subparts - should generate SUBREG */
        half_ptr[i * 2 + 1] = (uint16_t)(word_ptr[i] >> 16);
        byte_ptr[i * 4 + 2] = (uint8_t)(half_ptr[i * 2] >> 8);
    }
}

/* Test 5: Loop with data-dependent memory updates */
void test_loop_mem_pattern(volatile uint32_t *arr, int len) {
    uint32_t pattern = g_volatile_value;
    
    for (int i = 0; i < len; i++) {
        /* Data-dependent index prevents optimization */
        int idx = (pattern + i) % len;
        
        /* Complex addressing with shift */
        arr[idx] = (arr[(idx + 1) % len] << 3) 
                  | (arr[(idx + len - 1) % len] >> 5);
        
        /* Volatile memory barrier */
        asm volatile("" : : "m" (arr[idx]));
    }
}

/* ========== Main test driver ========== */
int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    /* Initialize test structures */
    struct bitfield_packed bp = {0};
    struct bitfield_mixed bm = {0};
    
    /* Allocate aligned memory for memory tests */
    volatile uint32_t *mem_buffer = 
        (volatile uint32_t *)aligned_alloc(16, 1024 * sizeof(uint32_t));
    union subreg_access *union_buf = 
        (union subreg_access *)malloc(64 * sizeof(union subreg_access));
    
    if (!mem_buffer || !union_buf) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero pattern */
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i * 0x01010101;
    }
    for (int i = 0; i < 64; i++) {
        union_buf[i].word = i * 0x11111111;
    }
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all test patterns */
    test_bitfield_ops(&bp, &bm, iterations);
    printf("Bitfield test completed: bp.d=%u, bm.high=%u\n", bp.d, bm.high);
    
    test_asm_clobber(&union_buf[0]);
    printf("Assembly clobber test completed: word=0x%08x\n", union_buf[0].word);
    
    test_mem_subreg(mem_buffer, iterations % 64);
    printf("Memory SUBREG test completed: mem[0]=0x%08x\n", mem_buffer[0]);
    
    test_mixed_type_access(union_buf, 64 * sizeof(union subreg_access));
    printf("Mixed-type access test completed\n");
    
    test_loop_mem_pattern(mem_buffer, 128);
    
    /* Compute checksum to ensure all operations executed */
    uint32_t checksum = 0;
    for (int i = 0; i < 128; i++) {
        checksum ^= mem_buffer[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    checksum ^= bp.d;
    checksum ^= bm.high;
    checksum ^= union_buf[0].word;
    
    printf("Final checksum: 0x%08x\n", checksum);
    printf("All tests completed successfully.\n");
    
    /* Cleanup */
    free((void *)mem_buffer);
    free(union_buf);
    
    return 0;
}
