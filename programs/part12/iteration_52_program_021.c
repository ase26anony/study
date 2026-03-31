/* test_resource.c - Program to trigger uncovered lines in resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int part1 : 8;
    unsigned int part2 : 4;
    unsigned int part3 : 12;
    unsigned int part4 : 8;
} __attribute__((packed));

volatile struct bitfield_struct bf;

NOOPT void test_zero_extract(void) {
    /* Writing to bit-fields often generates ZERO_EXTRACT */
    bf.part1 = 0xAB;
    bf.part2 = 0x7;
    bf.part3 = 0xDEF;
    bf.part4 = 0xCD;
    
    /* Also try with __builtin_bitfield */
    unsigned int val = 0x12345678;
    unsigned int field;
    
    /* Extract bit field (might generate ZERO_EXTRACT on some architectures) */
    field = (val >> 8) & 0xFFF;
    bf.part3 = field;
    
    /* Complex bit-field assignment with volatile */
    volatile unsigned int *p = (volatile unsigned int*)&bf;
    *p = (*p & ~(0xFF << 16)) | ((field & 0xFF) << 16);
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    /* Using inline assembly with %L0 modifier for low part */
    int x = 0x12345678;
    int y;
    
    /* Force STRICT_LOW_PART through inline asm */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax"
    );
    
    /* Another approach: volatile char assignment */
    volatile char *cptr = (volatile char*)&x;
    *cptr = 0x42;
    
    /* Using short to trigger partial register update */
    volatile short *sptr = (volatile short*)&x;
    *sptr = 0xABCD;
}

/* ========== SUBREG Pattern ========== */
/* Union for type-punning to generate SUBREG */
union type_pun {
    uint32_t full;
    uint16_t half[2];
    uint8_t byte[4];
};

NOOPT void test_subreg(void) {
    union type_pun u;
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts that might generate SUBREG */
    u.half[0] = u.half[1] + 0x100;
    u.byte[2] = u.byte[0] * 2;
    
    /* Using vector types can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* Might use SUBREG for extraction */
    
    /* Cast through different types */
    uint64_t big = 0x1122334455667788ULL;
    uint32_t *small = (uint32_t*)&big;
    small[1] = small[0] + 0x1000;
}

/* ========== MEM_P with Complex Addressing ========== */
struct nested {
    int data[16];
    struct nested *next;
};

volatile struct nested complex_array[10][10];

NOOPT void test_mem_complex_address(void) {
    /* Complex addressing modes */
    int i = global_counter % 10;
    int j = (global_counter + 1) % 10;
    int k = (global_counter + 2) % 10;
    
    /* Multi-dimensional array with index calculations */
    complex_array[i][j].data[k] = 
        complex_array[j][k].data[i] + 
        complex_array[k][i].data[j];
    
    /* Pointer chain with offset */
    struct nested *ptr = &complex_array[0][0];
    for (int idx = 0; idx < 5; idx++) {
        ptr->data[idx * 2] = ptr->data[idx * 2 + 1] * 3;
        /* Simulate linked list traversal */
        ptr = (struct nested*)((char*)ptr + sizeof(struct nested));
    }
    
    /* Inline asm with memory clobber for complex addressing */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (complex_array[1][1].data[0])
        : "m" (complex_array[0][0].data[4]),
          "i" (0x100)
        : "%eax", "memory"
    );
}

/* ========== Combined Test Function ========== */
NOOPT void test_combined(void) {
    /* Mix patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int a : 10;
        unsigned int b : 6;
        unsigned int c : 16;
    } bits = {0};
    
    bits.a = 0x3FF;
    bits.c = bits.b << 4;
    
    /* STRICT_LOW_PART via volatile byte store */
    volatile int val = 0x87654321;
    *(volatile char*)&val = 0x77;
    
    /* SUBREG via union */
    union {
        double d;
        int i[2];
    } u;
    u.d = 3.14159;
    u.i[0] = u.i[1] ^ 0x5555;
    
    /* MEM_P with complex address */
    static int arr[100];
    int idx = global_counter;
    arr[idx * 3 + 1] = arr[idx * 2] + arr[idx + 10];
}

/* ========== Main Driver ========== */
int main(void) {
    /* Initialize to prevent dead code elimination */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 16; k++) {
                complex_array[i][j].data[k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Execute all test patterns */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_mem_complex_address();
    test_combined();
    
    /* Dummy computation to use results */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 16; k++) {
                sum += complex_array[i][j].data[k];
            }
        }
    }
    
    /* Use bit-field values */
    sum += bf.part1 + bf.part2 + bf.part3 + bf.part4;
    
    return sum & 0xFF;  /* Return non-zero to prevent optimization */
}
