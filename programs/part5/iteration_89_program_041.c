/* Compile with: gcc -O2 -fdump-rtl-all -fno-strict-aliasing -o test_resource test_resource.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force runtime values */
static volatile int g_volatile_input = 0;

/* Bit-field structures to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

/* Packed struct for SUBREG generation */
struct __attribute__((packed)) mixed_types {
    char c;
    short s;
    int i;
};

/* Union for type-punning */
union punner {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Test 1: Bit-field assignments to trigger ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(struct bitfield_struct *bf, int idx) {
    /* Data-dependent index prevents constant propagation */
    volatile int i = idx;
    
    /* Multiple bit-field writes - may generate STRICT_LOW_PART */
    bf[i % 2].a = (g_volatile_input & 0xF);
    bf[i % 2].b = (g_volatile_input >> 4) & 0xFF;
    bf[i % 2].c = (g_volatile_input >> 12) & 0xFFF;
    bf[i % 2].d = (g_volatile_input >> 24) & 0xFF;
}

/* Test 2: Inline assembly with clobbers to stress reload pass */
void test_asm_clobber(void) {
    int input = g_volatile_input;
    int output;
    
    /* Inline asm that ties C variable to hard register with clobbers */
    asm volatile (
        "mov %1, %%r12\n\t"        /* Input to r12 */
        "add $0x7F, %%r12\n\t"     /* Modify in register */
        "mov %%r12, %0\n\t"        /* Output from r12 */
        : "=r" (output)
        : "r" (input)
        : "%r12", "cc"             /* Clobber r12 and flags */
    );
    
    /* Use output to prevent dead code elimination */
    g_volatile_input = output;
}

/* Test 3: Volatile memory accesses with type-punning for SUBREG/MEM */
void test_mem_subreg(volatile struct mixed_types *mem, int count) {
    for (int i = 0; i < count; i++) {
        /* Volatile write to char member - may generate SUBREG of MEM */
        mem[i].c = (char)(g_volatile_input + i);
        
        /* Volatile write to short member - another SUBREG possibility */
        mem[i].s = (short)(g_volatile_input * i);
        
        /* Full int write - plain MEM */
        mem[i].i = g_volatile_input ^ i;
    }
}

/* Test 4: Complex addressing modes with pointer arithmetic */
void test_complex_addressing(volatile uint32_t *base, int size) {
    union punner p;
    
    for (int i = 0; i < size; i++) {
        /* Type-punning through union */
        p.full = base[i];
        
        /* Access subparts - may generate SUBREG operations */
        p.parts.low = (uint16_t)(p.parts.high + i);
        p.parts.high = (uint16_t)(p.parts.low ^ g_volatile_input);
        
        /* Write back through volatile pointer - MEM with possible SUBREG */
        base[i] = p.full;
    }
}

/* Test 5: Register variable with bit-field manipulation */
void test_register_bitfield(void) {
    /* Register variable tied to specific register */
    register uint32_t reg_var asm("ebx");
    struct bitfield_struct local_bf;
    
    /* Initialize from volatile to prevent constant propagation */
    reg_var = g_volatile_input;
    
    /* Manipulate register variable */
    reg_var = (reg_var << 3) | (reg_var >> 29);
    
    /* Assign to bit-field from register variable */
    /* This combination may create interesting RTL patterns */
    local_bf.b = (reg_var & 0xFF);
    local_bf.c = ((reg_var >> 8) & 0xFFF);
    
    /* Use result */
    g_volatile_input = local_bf.b + local_bf.c;
}

int main(int argc, char *argv[]) {
    /* Initialize with command-line argument for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    /* Allocate test structures */
    struct bitfield_struct bf_array[2] = {{0}};
    struct mixed_types mem_array[5];
    uint32_t buffer[8] = {0};
    
    printf("Testing resource tracking paths...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int i = 0; i < iterations; i++) {
        /* Update volatile input for data dependence */
        g_volatile_input = i * 37 + 123;
        
        /* Test 1: Bit-field operations */
        test_bitfield_ops(bf_array, i);
        
        /* Test 2: Inline assembly with clobbers */
        test_asm_clobber();
        
        /* Test 3: Volatile memory with mixed types */
        test_mem_subreg(mem_array, 5);
        
        /* Test 4: Complex addressing */
        test_complex_addressing(buffer, 8);
        
        /* Test 5: Register variable with bit-field */
        test_register_bitfield();
    }
    
    /* Compute checksum to ensure all operations executed */
    uint32_t checksum = 0;
    checksum ^= bf_array[0].a + bf_array[0].b + bf_array[0].c + bf_array[0].d;
    checksum ^= bf_array[1].a + bf_array[1].b + bf_array[1].c + bf_array[1].d;
    checksum ^= mem_array[0].c + mem_array[0].s + mem_array[0].i;
    
    for (int i = 0; i < 8; i++) {
        checksum ^= buffer[i];
    }
    
    checksum ^= g_volatile_input;
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("Test completed. Compile with -O2 -fdump-rtl-all to see RTL.\n");
    
    return 0;
}
