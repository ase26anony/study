/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NO_OPT __attribute__((noinline, noipa))

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using volatile bit-fields to force ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
} NO_OPT;

/* Also test with __builtin_bitfield operations */
NO_OPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    volatile unsigned int temp = 0;
    
    /* These assignments should generate ZERO_EXTRACT RTL */
    bf.field1 = 5;          /* Writing to bit-field */
    bf.field2 = 0xAB;       /* Another bit-field write */
    bf.field3 = 0x7FF;      /* 12-bit field */
    
    /* Force use of the bit-fields to prevent elimination */
    temp = bf.field1 + bf.field2 + bf.field3;
    
    /* Alternative approach using bit operations */
    volatile uint32_t value = 0x12345678;
    /* Extract bits 8-15 and write back modified bits */
    uint32_t extracted = (value >> 8) & 0xFF;
    extracted = (extracted + 1) & 0xFF;
    value = (value & ~(0xFF << 8)) | (extracted << 8);
    
    /* Use the results */
    asm volatile("" : "+r" (temp), "+r" (value));
}

/* ========== STRICT_LOW_PART Pattern ========== */
/* Using inline assembly with %L0 modifier on x86 */
NO_OPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t char_var;
    uint32_t full_reg;
    
    /* Method 1: Inline assembly with low-part modifier */
    asm volatile(
        "movl $0x12345678, %0\n\t"
        "movw $0xABCD, %w0\n\t"  /* Low 16-bit part */
        : "=r" (full_reg)
        :
        : "memory"
    );
    
    /* Method 2: Volatile char/short assignments */
    uint32_t reg = 0xDEADBEEF;
    char_var = (uint8_t)(reg >> 16);  /* Force partial register update */
    
    /* Method 3: More explicit low-part operations */
    asm volatile(
        "movb %b1, %0\n\t"  /* Low byte */
        : "=m" (char_var)
        : "r" (reg)
        : "memory"
    );
    
    /* Use the variables */
    short_var = (uint16_t)full_reg;
    asm volatile("" : "+r" (reg), "+m" (short_var), "+m" (char_var));
}

/* ========== SUBREG Pattern ========== */
/* Using unions and type-punning for SUBREG generation */
typedef union {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
} subreg_union NO_OPT;

/* Using vector types for SUBREG operations */
typedef uint32_t v2u16 __attribute__((vector_size(8)));
typedef uint16_t v4u16 __attribute__((vector_size(8)));

NO_OPT void test_subreg(void) {
    subreg_union u;
    u.full = 0x12345678;
    
    /* These operations should generate SUBREG RTL */
    u.parts.low = 0xABCD;      /* Access low 16 bits */
    u.parts.high = 0x4321;     /* Access high 16 bits */
    u.bytes[1] = 0x99;         /* Access single byte */
    
    /* Vector operations that generate SUBREG */
    v2u16 vec32 = {0x12345678, 0x9ABCDEF0};
    v4u16 vec16;
    
    /* Type punning through memcpy to force SUBREG */
    memcpy(&vec16, &vec32, sizeof(vec16));
    
    /* Extract and modify individual elements */
    uint16_t elem = vec16[1];
    elem += 0x1111;
    vec16[2] = elem;
    
    /* Use packed structures */
    struct __attribute__((packed)) packed_struct {
        uint8_t a;
        uint32_t b;
        uint8_t c;
    } packed;
    
    packed.b = 0x87654321;
    uint32_t* ptr = &packed.b;
    uint32_t val = *ptr;  /* May generate SUBREG due to misalignment */
    
    /* Force use of all variables */
    asm volatile("" : "+r" (u.full), "+r" (elem), "+r" (val));
}

/* ========== MEM_P with Complex Addressing ========== */
/* Complex memory addressing patterns */
#define ARRAY_SIZE 100

struct nested {
    int data[10];
    struct nested* next;
};

NO_OPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile int* ptr_array[ARRAY_SIZE];
    struct nested complex_struct[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr_array[i] = &array[i][0];
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 1000 + j;
        }
        complex_struct[i].next = &complex_struct[(i + 1) % ARRAY_SIZE];
    }
    
    /* Complex addressing pattern 1: Multi-dimensional with offset */
    volatile int sum = 0;
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Complex address calculation */
            sum += array[i + 1][j - 1] + 
                   array[i - 1][j + 1] + 
                   *(ptr_array[i] + j);
        }
    }
    
    /* Complex addressing pattern 2: Pointer chains */
    struct nested* current = &complex_struct[0];
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        sum += current->data[i % 10];
        current = current->next;
    }
    
    /* Complex addressing pattern 3: Inline assembly with memory operands */
    int index = 50;
    int offset = 25;
    asm volatile(
        "movl (%1, %2, 4), %0\n\t"  /* array base + index * 4 */
        : "=r" (sum)
        : "r" (&array[0][0]), "r" (index + offset)
        : "memory"
    );
    
    /* Use volatile to prevent elimination */
    asm volatile("" : "+r" (sum));
}

/* ========== Combined Test Function ========== */
/* Function that combines all patterns */
NO_OPT void test_combined(void) {
    /* ZERO_EXTRACT pattern */
    struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
        volatile unsigned int c : 24;
    } bits;
    
    bits.a = 3;
    bits.b = 7;
    bits.c = 0xFFFFFF;
    
    /* STRICT_LOW_PART pattern */
    volatile uint16_t low_part;
    uint32_t reg32 = 0x12345678;
    asm volatile("movw %w1, %0" : "=m" (low_part) : "r" (reg32));
    
    /* SUBREG pattern */
    union {
        uint64_t dword;
        uint32_t words[2];
    } u;
    u.dword = 0x123456789ABCDEF0ULL;
    u.words[1] = 0x87654321;  /* High 32-bit part */
    
    /* MEM_P with complex addressing */
    volatile int matrix[10][10];
    int idx1 = 5, idx2 = 5;
    int result = matrix[idx1 + 1][idx2 - 1] + 
                 matrix[idx1 - 1][idx2 + 1];
    
    /* Use all results */
    asm volatile("" : "+r" (reg32), "+r" (u.dword), "+r" (result));
}

/* ========== Main Function ========== */
int main(void) {
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Call all test functions */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Dummy computation to prevent dead code elimination */
    volatile int dummy = 0;
    for (int i = 0; i < 100; i++) {
        dummy += i * i;
    }
    
    printf("Test completed. Dummy result: %d\n", dummy);
    return 0;
}
