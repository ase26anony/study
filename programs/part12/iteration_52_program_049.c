/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT for bit-field writes */
    bf.low_bits = 0xAB;          /* Writing to 8-bit field within 32-bit */
    bf.middle_bits = 0xCDEF;     /* Writing to 16-bit field */
    bf.high_bit = 1;             /* Writing to single bit */
    
    /* Also try with __builtin_bitfield operations if available */
    unsigned int value = 0x12345678;
    unsigned int mask = 0xFF;
    
    /* This may generate ZERO_EXTRACT when compiled */
    value = (value & ~mask) | (0xAA & mask);
    
    global_counter += bf.low_bits + bf.middle_bits + bf.high_bit + value;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */
NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* These assignments to partial registers may generate STRICT_LOW_PART */
    char_var = (char)0xAB;        /* Low byte assignment */
    short_var = (short)0xCDEF;    /* Low word assignment */
    
    /* Inline assembly with %L0 modifier for x86 low-part constraint */
    int result;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movb %b2, %%al\n\t"      /* %b2 = low byte constraint */
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (int_var), "r" ((char)0x99)
        : "%eax"
    );
    
    global_counter += char_var + short_var + result;
}

/* ==================== SUBREG Pattern ==================== */
/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) packed_data {
    char a;
    short b;
    int c;
    char d;
};

NOOPT void test_subreg(void) {
    struct packed_data pd;
    pd.a = 1;
    pd.b = 2;
    pd.c = 3;
    pd.d = 4;
    
    /* Type punning through union may generate SUBREG */
    union {
        uint32_t full;
        uint16_t half[2];
        uint8_t byte[4];
    } converter;
    
    converter.full = 0xDEADBEEF;
    converter.half[0] = 0x1234;  /* This may use SUBREG */
    converter.byte[1] = 0x56;    /* This may use SUBREG */
    
    /* Vector operations with sub-register extraction */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];         /* May generate SUBREG for extraction */
    
    global_counter += pd.c + converter.full + element;
}

/* ==================== MEM_P with Complex Addressing ==================== */
#define ARRAY_SIZE 100

NOOPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile int *ptr_array[ARRAY_SIZE];
    volatile int result = 0;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
        ptr_array[i] = &array[i][0];
    }
    
    /* Complex addressing modes - should generate non-trivial MEM addresses */
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Multi-dimensional array with offset */
            result += array[i+1][j-1] + array[i-1][j+1];
            
            /* Pointer arithmetic with multiple indices */
            result += *(ptr_array[i] + j * 2 - 1);
            
            /* Structure-like access pattern */
            result += array[i][j] * 3 - array[j][i];
        }
    }
    
    /* Inline assembly with memory clobber to force complex addressing */
    int temp = 0;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (temp)
        : "m" (array[10][20])    /* Memory operand */
        : "%eax"
    );
    
    global_counter += result + temp;
}

/* ==================== Combined Test ==================== */
NOOPT void test_combined(void) {
    /* Combine multiple patterns in one function */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 12;
        unsigned int field3 : 16;
    } bits = {0};
    
    /* ZERO_EXTRACT pattern */
    bits.field2 = 0xABC;
    
    /* STRICT_LOW_PART pattern via char assignment */
    volatile char low_byte = 0x7F;
    
    /* SUBREG pattern via union */
    union {
        long long big;
        int parts[2];
    } u;
    u.big = 0x123456789ABCDEF0LL;
    u.parts[0] = 0x11111111;  /* May use SUBREG */
    
    /* Complex MEM pattern */
    volatile int matrix[4][4] = {{0}};
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sum += matrix[(i+j)%4][(i*j)%4];
        }
    }
    
    global_counter += bits.field2 + low_byte + u.parts[0] + sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    /* Call all test functions to generate RTL patterns */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Prevent dead code elimination */
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
