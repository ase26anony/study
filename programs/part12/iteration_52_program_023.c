/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_VAR volatile

/* Structure for ZERO_EXTRACT pattern */
struct bitfield_struct {
    VOLATILE_VAR uint32_t full;
    struct {
        VOLATILE_VAR uint32_t low : 8;
        VOLATILE_VAR uint32_t mid : 8;
        VOLATILE_VAR uint32_t high : 16;
    } bits;
};

/* Packed structure for SUBREG pattern */
struct __attribute__((packed)) packed_data {
    uint8_t a;
    uint16_t b;
    uint8_t c;
    uint32_t d;
};

/* Complex structure for MEM_P pattern */
struct complex_mem {
    int data[256];
    struct complex_mem *next;
};

/* Test function for ZERO_EXTRACT pattern */
NOINLINE void test_zero_extract(void) {
    struct bitfield_struct bf = {0};
    
    /* Writing to bitfields often generates ZERO_EXTRACT in RTL */
    bf.bits.low = 0xAB;
    bf.bits.mid = 0xCD;
    bf.bits.high = 0xEF01;
    
    /* Force multiple ZERO_EXTRACT operations */
    bf.bits.low = bf.bits.mid ^ 0xFF;
    bf.bits.high = bf.bits.low | 0x8000;
    
    /* Use __builtin_bitfield for explicit ZERO_EXTRACT */
    uint32_t val = 0x12345678;
    uint32_t field = __builtin_bitfield_extract(val, 8, 8);
    __builtin_bitfield_insert(val, 0xAA, 16, 8);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bf.full), "r"(field));
}

/* Test function for STRICT_LOW_PART pattern */
NOINLINE void test_strict_low_part(void) {
    VOLATILE_VAR uint32_t global_word;
    VOLATILE_VAR uint16_t global_half;
    VOLATILE_VAR uint8_t global_byte;
    
    /* These assignments may generate STRICT_LOW_PART */
    global_half = 0x1234;
    global_byte = 0xAB;
    
    /* Inline assembly with low-part modifier for x86 */
    uint32_t temp;
    asm volatile(
        "movl $0xDEADBEEF, %0\n\t"
        "movb $0xCC, %%al\n\t"
        "movb %%al, %b0"
        : "=r"(temp)
        :
        : "eax"
    );
    
    /* More low-part operations */
    uint16_t *ptr16 = (uint16_t*)&global_word;
    *ptr16 = 0xFACE;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(temp), "m"(global_word));
}

/* Test function for SUBREG pattern */
NOINLINE void test_subreg(void) {
    /* Packed structure access forces SUBREG */
    struct packed_data pd;
    pd.a = 0x11;
    pd.b = 0x2233;  /* This may involve SUBREG due to misalignment */
    pd.c = 0x44;
    pd.d = 0x55667788;
    
    /* Type punning via union */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    u.full = 0x11223344;
    u.halves[0] = 0xAAAA;  /* Potential SUBREG */
    u.bytes[1] = 0xBB;     /* Another potential SUBREG */
    
    /* Vector operations (SIMD) can generate SUBREG */
    typedef uint32_t v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    uint32_t element = vec[2];  /* May use SUBREG for extraction */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(pd.b), "r"(u.full), "r"(element));
}

/* Test function for MEM_P with complex addressing */
NOINLINE void test_complex_mem(void) {
    VOLATILE_VAR struct complex_mem array[10];
    VOLATILE_VAR int indices[20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 256; j++) {
            array[i].data[j] = i * 100 + j;
        }
        array[i].next = (i < 9) ? &array[i + 1] : NULL;
    }
    
    for (int i = 0; i < 20; i++) {
        indices[i] = (i * 7) % 256;
    }
    
    /* Complex memory accesses that generate non-trivial addresses */
    int sum = 0;
    
    /* Multi-dimensional array with variable indices */
    sum += array[1].data[indices[0]];
    sum += array[2].data[indices[1] + 5];
    sum += array[3].data[indices[2] * 2];
    
    /* Pointer chain with offset */
    struct complex_mem *ptr = &array[0];
    sum += ptr->next->next->data[100];
    sum += ptr->next->data[indices[3]];
    
    /* Structure pointer arithmetic */
    sum += array[4].data[(int)(&array[4].data[10] - &array[4].data[0])];
    
    /* Prevent optimization */
    asm volatile("" : : "r"(sum));
}

/* Combined test function that uses all patterns */
NOINLINE void test_combined(void) {
    /* ZERO_EXTRACT via bitfield */
    VOLATILE_VAR struct {
        uint32_t a : 4;
        uint32_t b : 12;
        uint32_t c : 16;
    } bf;
    
    bf.a = 0xF;
    bf.b = 0xABC;
    bf.c = bf.a | (bf.b << 4);
    
    /* STRICT_LOW_PART via byte store */
    VOLATILE_VAR uint32_t word;
    uint8_t *byte_ptr = (uint8_t*)&word;
    byte_ptr[0] = 0x11;
    byte_ptr[1] = 0x22;
    
    /* SUBREG via packed access */
    struct __attribute__((packed)) {
        uint16_t a;
        uint32_t b;
    } packed = {0x1234, 0x56789ABC};
    
    uint32_t extracted = packed.b;  /* May involve SUBREG */
    
    /* Complex MEM access */
    VOLATILE_VAR int arr[10][20];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr[i][j] = i * 20 + j;
        }
    }
    
    int idx1 = 5, idx2 = 15;
    int val = arr[idx1][idx2] + arr[idx2 % 10][idx1 % 20];
    
    /* Prevent optimization */
    asm volatile("" : : "r"(bf.c), "r"(word), "r"(extracted), "r"(val));
}

/* Main function that calls all tests */
int main(void) {
    /* Initialize some volatile variables to prevent optimization */
    VOLATILE_VAR int counter = 0;
    
    /* Run individual pattern tests */
    test_zero_extract();
    counter++;
    
    test_strict_low_part();
    counter++;
    
    test_subreg();
    counter++;
    
    test_complex_mem();
    counter++;
    
    /* Run combined test */
    test_combined();
    counter++;
    
    /* Dummy computation to ensure code isn't eliminated */
    VOLATILE_VAR int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * counter;
    }
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    /* Return something based on computations */
    return sum > 0 ? 0 : 1;
}
