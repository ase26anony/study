/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile input to prevent constant propagation */
volatile int g_vol_input = 0;

/* Struct with bitfields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} __attribute__((packed));

/* Union for type-punning to generate SUBREG */
union mixed_access {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
};

/* Global volatile memory for MEM RTL generation */
volatile uint32_t g_mem_buffer[256];

/* Function 1: Bitfield assignments with volatile control */
void test_bitfield_ops(struct bitfield_struct *bf, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent assignments prevent optimization */
        int idx = (i + g_vol_input) & 0xFF;
        
        /* These assignments should generate ZERO_EXTRACT/STRICT_LOW_PART */
        bf[idx].a = (i & 0xF);
        bf[idx].b = (i * 3) & 0xFF;
        bf[idx].c = (i * 5) & 0xFFF;
        bf[idx].d = (i * 7) & 0xFF;
    }
}

/* Function 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    register uint32_t reg_var asm("r12") = 0x12345678;
    struct bitfield_struct local_bf;
    
    /* Inline asm that clobbers registers, forcing reload */
    asm volatile (
        "mov %[reg], %[val]\n\t"
        "and %[reg], #0xFFF\n\t"
        : [reg] "+r" (reg_var)
        : [val] "r" (0x89ABCDEF)
        : "cc"
    );
    
    /* Use the register variable with bitfield - may generate STRICT_LOW_PART */
    local_bf.c = reg_var & 0xFFF;
    
    /* More inline asm with memory clobber */
    asm volatile (
        "mov r0, %0\n\t"
        "str r0, [%1]\n\t"
        : 
        : "r" (reg_var), "r" (&g_mem_buffer[0])
        : "r0", "memory"
    );
}

/* Function 3: Mixed-type accesses via union and pointers for SUBREG */
void test_mixed_type_access(union mixed_access *u, int offset) {
    volatile uint8_t *byte_ptr;
    volatile uint16_t *half_ptr;
    
    /* Write full word - generates MEM RTL */
    u[offset].full = 0xDEADBEEF;
    
    /* Access via smaller types - may generate SUBREG of MEM */
    byte_ptr = (volatile uint8_t *)&u[offset].full;
    half_ptr = (volatile uint16_t *)&u[offset].full;
    
    /* Modify through different views */
    for (int i = 0; i < 4; i++) {
        byte_ptr[i] = (offset + i) & 0xFF;
    }
    
    /* SUBREG generation from half-word access */
    half_ptr[1] = half_ptr[0] ^ 0x5555;
}

/* Function 4: Complex memory addressing with volatile */
void test_complex_mem_addressing(int iterations) {
    volatile uint32_t * volatile ptr_array[4];
    
    /* Setup pointers with different offsets */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &g_mem_buffer[i * 16];
    }
    
    /* Loop with data-dependent memory writes */
    for (int i = 0; i < iterations; i++) {
        int idx = (i + g_vol_input) % 4;
        
        /* Complex addressing mode - ensures MEM_P(x) is true */
        *(ptr_array[idx] + (i & 0xF)) = i * i;
        
        /* Cast to char pointer for sub-register access */
        *((volatile uint8_t *)(ptr_array[idx] + (i & 0xF))) = i & 0xFF;
    }
}

/* Function 5: Nested bitfield in struct with volatile member */
struct container {
    volatile int counter;
    struct bitfield_struct bf;
    union mixed_access u;
};

void test_nested_struct_ops(struct container *c, int count) {
    for (int i = 0; i < count; i++) {
        /* Volatile read forces memory access */
        int base = c->counter;
        
        /* Bitfield assignment with volatile base */
        c->bf.b = (base + i) & 0xFF;
        c->bf.c = (base * i) & 0xFFF;
        
        /* Mixed access through union */
        c->u.bytes[0] = c->bf.a;
        c->u.half[1] = c->bf.b;
        
        /* Update volatile counter */
        c->counter += i & 0x3;
    }
}

int main(int argc, char *argv[]) {
    struct bitfield_struct *bf_array;
    union mixed_access *union_array;
    struct container *cont;
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Allocate test structures */
    bf_array = (struct bitfield_struct *)calloc(256, sizeof(struct bitfield_struct));
    union_array = (union mixed_access *)calloc(256, sizeof(union mixed_access));
    cont = (struct container *)calloc(4, sizeof(struct container));
    
    if (!bf_array || !union_array || !cont) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize volatile input */
    g_vol_input = iterations & 0xFF;
    
    printf("Starting resource tracking tests...\n");
    
    /* Execute test functions to generate target RTL patterns */
    test_bitfield_ops(bf_array, iterations);
    test_asm_clobber();
    test_mixed_type_access(union_array, iterations % 256);
    test_complex_mem_addressing(iterations % 64);
    test_nested_struct_ops(cont, iterations % 32);
    
    /* Compute checksum to ensure execution */
    uint32_t checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += bf_array[i].a + bf_array[i].b + bf_array[i].c + bf_array[i].d;
        checksum += union_array[i].full;
        if (i < 4) checksum += cont[i].counter;
    }
    
    for (int i = 0; i < 256; i++) {
        checksum += g_mem_buffer[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(bf_array);
    free(union_array);
    free(cont);
    
    return 0;
}
