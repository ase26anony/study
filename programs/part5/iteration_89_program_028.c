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
};

/* Union for type-punning to generate SUBREG */
union type_punner {
    uint32_t full;
    uint16_t half[2];
    uint8_t byte[4];
};

/* Volatile memory buffer */
static volatile uint32_t mem_buffer[256];

/* Function 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    volatile int seed = iterations; /* Prevent constant propagation */
    
    for (int i = 0; i < iterations; i++) {
        /* These assignments often generate ZERO_EXTRACT or STRICT_LOW_PART */
        bf->a = (seed + i) & 0xF;
        bf->b = (seed * i) & 0xFF;
        bf->c = (seed - i) & 0xFFF;
        bf->d = (seed ^ i) & 0xFF;
        
        /* Mix with volatile to prevent optimization */
        seed = bf->b;
    }
}

/* Function 2: Mixed-type accesses via union to generate SUBREG */
void test_subreg_access(union type_punner *pun, int offset) {
    volatile uint32_t control = offset;
    
    /* Access different sized parts of the same memory */
    pun->half[0] = (uint16_t)(control & 0xFFFF);
    pun->byte[2] = (uint8_t)((control >> 8) & 0xFF);
    pun->full = pun->full ^ 0x00FF00FF;
    
    /* SUBREG may appear when accessing sub-parts */
    uint16_t temp = pun->half[1];
    pun->byte[3] = (uint8_t)(temp >> 4);
}

/* Function 3: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(struct mixed_bitfields *mf) {
    register uint32_t reg_var asm("r12") = mf->x;
    uint32_t temp;
    
    /* Inline assembly that clobbers registers */
    asm volatile (
        "mov %[input], %%eax\n\t"
        "ror $4, %%eax\n\t"
        "mov %%eax, %[output]\n\t"
        : [output] "=r" (temp)
        : [input] "r" (reg_var)
        : "eax", "cc"
    );
    
    /* Bit-field assignment after asm - may generate STRICT_LOW_PART */
    mf->y = temp & 0x1F;
    mf->z = (temp >> 5) & 0x3FF;
}

/* Function 4: Memory operations with complex addressing for MEM_P */
void test_mem_ops(int index, int value) {
    volatile int idx = index;
    volatile int val = value;
    
    /* Complex addressing mode */
    uint32_t *ptr = (uint32_t *)&mem_buffer[idx % 256];
    
    /* Direct memory write - should generate MEM RTL */
    *ptr = val;
    
    /* Pointer arithmetic with different type */
    uint16_t *short_ptr = (uint16_t *)ptr;
    short_ptr[1] = (uint16_t)(val >> 16);  /* May generate SUBREG of MEM */
    
    /* Volatile memory read-modify-write */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)ptr;
    byte_ptr[3] = byte_ptr[0] ^ byte_ptr[2];
}

/* Function 5: Combined test with data-dependent control flow */
void test_combined(struct bitfield_struct *bf1, 
                   union type_punner *pun,
                   struct mixed_bitfields *mf,
                   int base_val) {
    volatile int counter = base_val;
    
    for (int i = 0; i < 100; i++) {
        /* Data-dependent branching */
        if (counter & 1) {
            test_bitfield_ops(bf1, 2);
        } else {
            test_subreg_access(pun, counter);
        }
        
        /* Mix with inline assembly every 3rd iteration */
        if (i % 3 == 0) {
            test_asm_clobber(mf);
        }
        
        /* Memory operations with varying addresses */
        test_mem_ops(counter + i, mf->z);
        
        /* Update counter in non-trivial way */
        counter = (counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

int main(int argc, char *argv[]) {
    /* Initialize test data */
    struct bitfield_struct bf = {0};
    union type_punner pun = {0xDEADBEEF};
    struct mixed_bitfields mf = {0};
    
    /* Use command-line argument for runtime variability */
    int base_value = 42;
    if (argc > 1) {
        base_value = atoi(argv[1]) & 0xFF;
    }
    
    printf("Starting resource pattern tests...\n");
    printf("Initial values: bf.a=%u, pun.full=0x%08X, mf.x=%u\n", 
           bf.a, pun.full, mf.x);
    
    /* Run individual tests */
    test_bitfield_ops(&bf, base_value + 10);
    test_subreg_access(&pun, base_value);
    test_asm_clobber(&mf);
    test_mem_ops(base_value, 0x12345678);
    
    /* Run combined test */
    test_combined(&bf, &pun, &mf, base_value);
    
    /* Compute and print checksum to ensure execution */
    uint32_t checksum = bf.a + bf.b + bf.c + bf.d;
    checksum += pun.full;
    checksum += mf.x + mf.y + mf.z;
    
    for (int i = 0; i < 16; i++) {
        checksum += mem_buffer[i];
    }
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
