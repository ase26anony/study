/* Test program to trigger specific RTL patterns in mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to trigger ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 4;
    volatile unsigned int padding : 16;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to increase chances of ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 42;
    bf.field3 = 3;
    
    /* Complex bit-field manipulation */
    bf.field2 = bf.field1 + bf.field3;
    
    global_counter += bf.field1 + bf.field2 + bf.field3;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These assignments may generate STRICT_LOW_PART */
    s_val = (short)i_val;          /* Low 16-bit part */
    c_val = (char)i_val;           /* Low 8-bit part */
    
    /* Inline assembly with low-part modifier for x86 */
    #ifdef __x86_64__
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (c_val)
        : "r" (i_val)
        : "%eax"
    );
    #endif
    
    global_counter += s_val + c_val;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Using unions for type-punning */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } data;
    
    data.full = 0xDEADBEEF;
    
    /* Operations on sub-parts that may generate SUBREG */
    data.halves[0] += data.halves[1];
    data.bytes[1] = data.bytes[2] ^ data.bytes[3];
    
    /* Vector types can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    int element = vec1[2];  /* May involve SUBREG extraction */
    
    global_counter += data.full + element;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int array[10][10][10];
    volatile int *ptr_array[100];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex memory accesses with multiple indices */
    int sum = 0;
    for (int i = 1; i < 9; i++) {
        for (int j = 1; j < 9; j++) {
            for (int k = 1; k < 9; k++) {
                /* Complex addressing expression */
                sum += array[i+1][j-1][k] + 
                       array[i][j+1][k-1] + 
                       array[i-1][j][k+1];
            }
        }
    }
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
    };
    
    struct node nodes[10];
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].value = 90;
    nodes[9].next = NULL;
    
    /* Complex pointer dereferencing */
    struct node *current = &nodes[0];
    while (current && current->next && current->next->next) {
        sum += current->value + current->next->next->value;
        current = current->next;
    }
    
    global_counter += sum;
}

/* ===== Combined Test Function ===== */
NOOPT void test_combined(void) {
    /* Mix different patterns in one function */
    
    /* Bit-field (ZERO_EXTRACT) */
    struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
    } bits;
    bits.a = 3;
    bits.b = bits.a << 1;
    
    /* Low-part assignment (STRICT_LOW_PART) */
    volatile short low_part;
    volatile int src = 0xABCD;
    low_part = src;  /* May generate STRICT_LOW_PART */
    
    /* SUBREG through union */
    union {
        uint64_t dword;
        uint32_t words[2];
    } u;
    u.dword = 0x1122334455667788ULL;
    u.words[0] = u.words[1] & 0xFFFF;
    
    /* Complex memory access */
    volatile int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 5 + j;
        }
    }
    
    int total = 0;
    for (int i = 1; i < 4; i++) {
        for (int j = 1; j < 4; j++) {
            /* Complex addressing */
            total += matrix[i-1][j] + matrix[i][j+1] + matrix[i+1][j-1];
        }
    }
    
    global_counter += bits.a + bits.b + low_part + u.words[0] + total;
}

/* Main function to run all tests */
int main(void) {
    /* Initialize to prevent dead code elimination */
    global_counter = 0;
    
    /* Run individual pattern tests */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    
    /* Run combined test */
    test_combined();
    
    /* Additional iterations to increase coverage */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_complex_mem();
    }
    
    /* Return something based on computations to prevent optimization */
    return global_counter == 0 ? 1 : 0;
}
