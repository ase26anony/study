/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */
/* This program is designed to trigger specific RTL patterns in GCC's resource.cc,
   particularly the uncovered lines in mark_set_resources() that handle
   ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM expressions. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_vol_input = 0;

/* ==================== BIT-FIELD STRUCTURES ==================== */
/* Bit-fields often generate ZERO_EXTRACT or STRICT_LOW_PART in RTL */

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
} __attribute__((packed));

struct nested_bitfield {
    struct {
        unsigned int low : 4;
        unsigned int high : 4;
    } byte;
    unsigned int word : 16;
};

/* ==================== MIXED-TYPE ACCESS ==================== */
/* Union for type-punning to generate SUBREG RTL */
union mixed_types {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
};

/* ==================== TEST FUNCTIONS ==================== */

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int idx) {
    /* Data-dependent indexing prevents optimization */
    volatile int i = idx;
    
    /* Multiple bit-field writes - may generate STRICT_LOW_PART */
    bf[i].a = (i & 0x7);                /* 3-bit field */
    bf[i].b = (i & 0x1F);               /* 5-bit field */
    bf[i].c = (i & 0xFF);               /* 8-bit field */
    bf[i].d = (i & 0xFFFF);             /* 16-bit field */
    
    /* Nested bit-field access */
    struct nested_bitfield nb;
    nb.byte.low = (i & 0xF);
    nb.byte.high = ((i >> 4) & 0xF);
    nb.word = (i & 0xFFFF);
    
    /* Write to prevent dead code elimination */
    bf[0].a = nb.byte.low;
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    int input = g_vol_input;
    int output;
    
    /* Inline asm that ties C variables to specific registers */
    asm volatile (
        "mov %1, %%r12\n\t"            /* Move input to r12 */
        "add $0x7F, %%r12\n\t"         /* Modify in register */
        "mov %%r12, %0\n\t"            /* Move result to output */
        : "=r" (output)
        : "r" (input)
        : "%r12", "cc"                 /* Clobber r12 and condition codes */
    );
    
    /* Use output to prevent optimization */
    volatile int sink = output;
}

/* Test 3: Volatile memory accesses with type-punning for SUBREG+MEM */
void test_volatile_mem(volatile union mixed_types *mem, int count) {
    for (int i = 0; i < count; i++) {
        /* Write whole word (MEM) */
        mem[i].word = i * 0x01010101;
        
        /* Write half-word (SUBREG of MEM) */
        mem[i].half[1] = (i & 0xFFFF);
        
        /* Write byte (SUBREG of MEM) */
        mem[i].byte[0] = (i & 0xFF);
        
        /* Complex addressing with offset */
        volatile uint16_t *half_ptr = &mem[i].half[0];
        half_ptr[1] = half_ptr[0] + 1;  /* Another SUBREG MEM */
    }
}

/* Test 4: Pointer casting for SUBREG generation */
void test_pointer_cast(volatile void *buffer, int size) {
    volatile char *cptr = (volatile char *)buffer;
    volatile short *sptr = (volatile short *)buffer;
    volatile int *iptr = (volatile int *)buffer;
    
    /* Mixed-size accesses to same memory */
    for (int i = 0; i < size; i += 4) {
        iptr[i/4] = g_vol_input + i;      /* MEM with int access */
        sptr[i/2] = (short)(iptr[i/4]);   /* SUBREG access */
        cptr[i] = (char)(sptr[i/2]);      /* SUBREG access */
    }
}

/* Test 5: Register variable with bit-field manipulation */
void test_register_bitfield(void) {
    /* Register variable tied to specific register */
    register uint32_t reg_var asm("ebx") = g_vol_input;
    
    /* Bit-field extraction/manipulation */
    struct bitfield_struct bf;
    bf.a = (reg_var & 0x7);          /* May use STRICT_LOW_PART */
    bf.b = ((reg_var >> 3) & 0x1F);
    
    /* Force register spilling/reloading */
    asm volatile ("" : "+r" (reg_var));
    
    /* More bit-field ops */
    bf.c = ((reg_var >> 8) & 0xFF);
    bf.d = ((reg_var >> 16) & 0xFFFF);
    
    /* Use result */
    volatile uint32_t result = bf.a + bf.b + bf.c + bf.d;
}

/* ==================== MAIN DRIVER ==================== */
int main(int argc, char *argv[]) {
    /* Use command-line argument for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* Allocate test structures */
    struct bitfield_struct *bf_array = 
        (struct bitfield_struct *)calloc(iterations, sizeof(struct bitfield_struct));
    
    union mixed_types *mem_buffer = 
        (union mixed_types *)malloc(iterations * sizeof(union mixed_types));
    
    /* Initialize volatile input */
    g_vol_input = iterations;
    
    printf("Starting resource tracking tests...\n");
    printf("Iterations: %d\n", iterations);
    
    /* Run all tests in sequence */
    for (int i = 0; i < iterations; i++) {
        test_bitfield_ops(bf_array, i % 4);
    }
    
    test_asm_clobber();
    
    test_volatile_mem(mem_buffer, iterations);
    
    test_pointer_cast(mem_buffer, iterations * sizeof(union mixed_types));
    
    test_register_bitfield();
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < iterations; i++) {
        checksum += bf_array[i].a + bf_array[i].b;
        checksum += mem_buffer[i].word & 0xFF;
    }
    
    printf("Checksum: %u\n", checksum);
    
    /* Cleanup */
    free(bf_array);
    free(mem_buffer);
    
    return 0;
}
