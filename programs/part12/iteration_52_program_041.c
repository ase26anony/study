/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE volatile

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to trigger ZERO_EXTRACT RTL */
struct bitfield_struct {
    VOLATILE uint32_t full;
    struct {
        VOLATILE uint32_t low : 8;
        VOLATILE uint32_t mid : 8;
        VOLATILE uint32_t high : 8;
        VOLATILE uint32_t top : 8;
    } parts;
} NOINLINE;

NOINLINE void test_zero_extract(void) {
    struct bitfield_struct bf;
    bf.full = 0x12345678;
    
    /* Writing to bit-fields should generate ZERO_EXTRACT */
    bf.parts.low = 0xAA;    /* ZERO_EXTRACT of full */
    bf.parts.mid = 0xBB;    /* ZERO_EXTRACT of full */
    bf.parts.high = 0xCC;   /* ZERO_EXTRACT of full */
    bf.parts.top = 0xDD;    /* ZERO_EXTRACT of full */
    
    /* Read back to prevent elimination */
    VOLATILE uint32_t dummy = bf.full;
    (void)dummy;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOINLINE void test_strict_low_part(void) {
    VOLATILE uint16_t short_var;
    VOLATILE uint8_t byte_var;
    VOLATILE uint32_t int_var = 0x12345678;
    
    /* Using inline assembly with % modifier for low part */
    __asm__ volatile (
        "movw %1, %0\n\t"
        : "=r" (short_var)
        : "r" ((uint16_t)int_var)
        : /* no clobbers */
    );
    
    /* Assignment to low byte may generate STRICT_LOW_PART */
    byte_var = (uint8_t)int_var;
    
    /* Another approach: char assignment to volatile */
    VOLATILE char *ptr = (VOLATILE char *)&int_var;
    ptr[0] = 0xFF;  /* STRICT_LOW_PART for byte store */
    
    /* Prevent dead code elimination */
    (void)short_var;
    (void)byte_var;
}

/* ========== SUBREG Pattern ========== */
/* Using unions and type-punning for SUBREG */
union subreg_union {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
} NOINLINE;

NOINLINE void test_subreg(void) {
    union subreg_union u;
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts should generate SUBREG */
    u.halves[0] += 1;      /* SUBREG access */
    u.halves[1] -= 1;      /* SUBREG access */
    
    /* Vector-style operations */
    typedef uint16_t v2hi __attribute__((vector_size(4)));
    v2hi vec = {0x1234, 0x5678};
    vec[0] = vec[1];       /* Likely SUBREG access */
    
    /* Prevent elimination */
    VOLATILE uint32_t dummy = u.full;
    (void)dummy;
    (void)vec;
}

/* ========== MEM_P with Complex Addressing ========== */
struct nested {
    int data[16];
};

struct complex_mem {
    struct nested arrays[8][4];
    int padding[32];
} NOINLINE;

NOINLINE void test_complex_mem(void) {
    static VOLATILE struct complex_mem cm;
    
    /* Complex addressing modes */
    int i = 3, j = 2, k = 1;
    
    /* Multi-dimensional array with computed indices */
    cm.arrays[i][j].data[k] = 0x1234;
    cm.arrays[j][k].data[i] = cm.arrays[k][i].data[j];
    
    /* Pointer arithmetic with multiple offsets */
    VOLATILE int *ptr = &cm.arrays[0][0].data[0];
    ptr[i * 64 + j * 16 + k] = 0x5678;  /* Complex address calculation */
    
    /* Structure pointer chain */
    struct nested *nptr = &cm.arrays[1][2];
    nptr->data[3] = nptr->data[5] + 1;
    
    /* Prevent elimination */
    VOLATILE int dummy = cm.arrays[0][0].data[0];
    (void)dummy;
}

/* ========== Combined Test Function ========== */
NOINLINE void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    VOLATILE struct {
        uint32_t full;
        uint32_t low : 10;
    } bf = {0};
    bf.low = 0x3FF;
    
    /* STRICT_LOW_PART via byte store */
    VOLATILE uint32_t word = 0x12345678;
    *(VOLATILE uint8_t *)&word = 0xAA;
    
    /* SUBREG via union access */
    union {
        uint64_t dword;
        uint32_t words[2];
    } u;
    u.dword = 0x1122334455667788ULL;
    u.words[0] = u.words[1];
    
    /* Complex MEM_P */
    VOLATILE int arr[10][10][10];
    int x = 1, y = 2, z = 3;
    arr[x][y][z] = arr[z][x][y] + arr[y][z][x];
    
    /* Prevent elimination */
    (void)bf.full;
    (void)word;
    (void)u.dword;
    (void)arr[0][0][0];
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    return 0;
}
