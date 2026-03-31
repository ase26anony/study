/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force actual memory operations */
volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */

/* Using bit-fields to trigger ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int part1 : 8;
    unsigned int part2 : 8;
    unsigned int part3 : 8;
    unsigned int part4 : 8;
} __attribute__((packed));

/* Also try __builtin_bitfield */
NOOPT void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    
    /* Writing to bit-field should generate ZERO_EXTRACT */
    bf.part1 = 0xAA;
    bf.part2 = 0xBB;
    bf.part3 = 0xCC;
    bf.part4 = 0xDD;
    
    /* Another approach using bit operations */
    volatile uint32_t value = 0;
    uint32_t mask = 0xFF;
    
    /* This might generate ZERO_EXTRACT when optimized */
    value = (value & ~(mask << 8)) | ((0x55 & mask) << 8);
    
    /* Use __builtin_bitfield if available */
    #ifdef __has_builtin
    #if __has_builtin(__builtin_bitfield)
    uint32_t x = 0x12345678;
    __builtin_bitfield(x, 8, 16) = 0xAA;
    #endif
    #endif
    
    global_counter += bf.part1 + bf.part2;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */

NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t byte_var;
    volatile uint32_t int_var = 0x12345678;
    
    /* Writing to low parts of variables - may generate STRICT_LOW_PART */
    short_var = 0xABCD;      /* Low 16-bit assignment */
    byte_var = 0xEF;         /* Low 8-bit assignment */
    
    /* Inline assembly with low-part modifier for x86 */
    #ifdef __x86_64__
    uint32_t reg_var;
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "movb $0xAA, %b0\n\t"    /* %b0 = low byte */
        : "=r" (reg_var)
        :
        : "cc"
    );
    global_counter += reg_var;
    #endif
    
    /* Another approach: char assignment to volatile */
    volatile char *char_ptr = (volatile char *)&int_var;
    *char_ptr = 0xFF;  /* Modify low byte */
    
    global_counter += short_var + byte_var;
}

/* ==================== SUBREG Pattern ==================== */

NOOPT void test_subreg(void) {
    /* Using unions for type-punning */
    union pun {
        uint32_t full;
        uint16_t half[2];
        uint8_t byte[4];
    } u;
    
    u.full = 0x12345678;
    
    /* Operations on sub-parts that might generate SUBREG */
    u.half[0] = u.half[0] + 1;    /* Modify low half */
    u.half[1] = u.half[1] - 1;    /* Modify high half */
    
    /* Using packed structures */
    struct __attribute__((packed)) mixed {
        char a;
        int b;
        short c;
    } s;
    
    s.a = 'x';
    s.b = 42;
    s.c = 1000;
    
    /* Vector operations can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* Extract element */
    
    global_counter += u.full + s.b + element;
}

/* ==================== MEM_P with Complex Addressing ==================== */

NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int arr[10][10][10];
    
    /* Complex addressing expression */
    int i = global_counter % 10;
    int j = (global_counter + 1) % 10;
    int k = (global_counter + 2) % 10;
    
    /* This should generate complex address computation */
    arr[i][j][k] = arr[j][k][i] + arr[k][i][j];
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
    };
    
    volatile struct node nodes[5];
    for (int idx = 0; idx < 4; idx++) {
        nodes[idx].next = &nodes[idx + 1];
        nodes[idx].value = idx * 10;
    }
    
    /* Complex pointer chain access */
    volatile int sum = 0;
    volatile struct node *current = &nodes[0];
    for (int idx = 0; idx < 5; idx++) {
        if (current) {
            sum += current->value;
            current = current->next;
        }
    }
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl $42, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (arr[0][0][0])
        :
        : "eax", "memory"
    );
    
    global_counter += arr[0][0][0] + sum;
}

/* ==================== Combined Test ==================== */

NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 24;
    } bits = {0};
    
    bits.a = 5;
    bits.b = 10;
    
    /* STRICT_LOW_PART via byte assignment */
    volatile uint32_t val = 0x87654321;
    *(volatile uint8_t *)&val = 0xFF;
    
    /* SUBREG via union */
    union {
        uint64_t dword;
        uint32_t words[2];
    } u;
    u.dword = 0x1122334455667788ULL;
    u.words[0] = u.words[0] ^ 0xFFFF;
    
    /* Complex MEM access */
    volatile int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex addressing */
    int idx1 = (global_counter * 3) % 5;
    int idx2 = (global_counter * 7) % 5;
    matrix[idx1][idx2] = matrix[idx2][idx1] * 2;
    
    global_counter += bits.a + bits.b + (val & 0xFF) + u.words[0] + matrix[0][0];
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Initialize to prevent dead code elimination */
    volatile int result = 0;
    
    /* Call all test functions */
    test_zero_extract();
    result += global_counter;
    
    test_strict_low_part();
    result += global_counter;
    
    test_subreg();
    result += global_counter;
    
    test_complex_mem();
    result += global_counter;
    
    test_combined();
    result += global_counter;
    
    /* Dummy computation to ensure code isn't optimized away */
    volatile int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    /* Return something based on all computations */
    return (result + sum) == 0 ? 0 : 0;
}
