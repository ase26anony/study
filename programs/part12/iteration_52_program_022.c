/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */
NOOPT void test_zero_extract(void) {
    /* Method 1: Bit-field operations on volatile struct */
    struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 8;
        volatile unsigned int field3 : 20;
    } bitfield;
    
    /* Multiple assignments to ensure RTL generation */
    bitfield.field1 = 5;
    bitfield.field2 = 0xAB;
    bitfield.field3 = 0x12345;
    
    /* Method 2: Using __builtin_bitfield */
    unsigned int value = 0xDEADBEEF;
    unsigned int extracted;
    
    /* Extract bits 8-15 */
    extracted = __builtin_bitfield_extract(value, 8, 8);
    /* Store into bits 16-23 */
    value = __builtin_bitfield_insert(value, extracted, 16, 8);
    
    global_counter += bitfield.field1 + extracted;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */
NOOPT void test_strict_low_part(void) {
    /* Method 1: Inline assembly with %L0 modifier (x86-specific) */
    unsigned int val = 0x12345678;
    unsigned char low_byte;
    
    asm volatile (
        "movb %b1, %0"
        : "=r" (low_byte)
        : "r" (val)
        : "cc"
    );
    
    /* Method 2: Volatile char assignment to force partial register update */
    volatile unsigned short low_word;
    volatile unsigned int source = 0xABCD1234;
    
    /* This should generate STRICT_LOW_PART for the low word */
    low_word = (unsigned short)source;
    
    /* Method 3: Multiple partial assignments */
    volatile struct {
        unsigned char a;
        unsigned char b;
        unsigned short c;
    } parts;
    
    parts.a = 0x11;
    parts.b = 0x22;
    parts.c = 0x3344;
    
    global_counter += low_byte + low_word + parts.c;
}

/* ==================== SUBREG Pattern ==================== */
NOOPT void test_subreg(void) {
    /* Method 1: Packed structure with type punning */
    struct __attribute__((packed)) {
        uint32_t full;
        uint16_t half1;
        uint16_t half2;
    } packed_data;
    
    packed_data.full = 0x88776655;
    packed_data.half1 = 0x1234;
    packed_data.half2 = 0x5678;
    
    /* Method 2: Union for type punning */
    union {
        uint32_t dword;
        uint16_t words[2];
        uint8_t bytes[4];
    } converter;
    
    converter.dword = 0xDEADBEEF;
    converter.words[1] = 0xCAFE;  /* Should generate SUBREG */
    
    /* Method 3: Vector operations */
    typedef uint32_t v2si __attribute__((vector_size(8)));
    v2si vec = {0x11111111, 0x22222222};
    uint32_t element = vec[0];  /* May generate SUBREG */
    
    /* Method 4: Bit manipulation creating subregs */
    uint64_t large = 0x123456789ABCDEF0ULL;
    uint32_t lower = (uint32_t)large;  /* SUBREG extraction */
    uint32_t upper = (uint32_t)(large >> 32);
    
    global_counter += packed_data.half1 + converter.words[0] + element + lower + upper;
}

/* ==================== MEM_P with Complex Addressing ==================== */
NOOPT void test_complex_mem(void) {
    /* Complex multi-dimensional array access */
    volatile int matrix[10][10][10];
    
    /* Initialize to prevent dead code elimination */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                matrix[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex addressing expressions */
    volatile int *ptr = &matrix[0][0][0];
    int index1 = global_counter % 100;
    int index2 = (global_counter * 7) % 100;
    int index3 = (global_counter * 13) % 100;
    
    /* Multiple complex memory operations */
    int val1 = matrix[index1/10][index1%10][index2%10];
    int val2 = *(ptr + index1 + index2 * 10 + index3 * 100);
    int val3 = matrix[5][index2%10][index3%10];
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
        struct node *prev;
    };
    
    struct node nodes[5];
    for (int i = 0; i < 5; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = &nodes[(i + 1) % 5];
        nodes[i].prev = &nodes[(i + 4) % 5];
    }
    
    /* Complex structure pointer access */
    int chain_val = nodes[0].next->next->prev->value;
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %1, %%eax\n"
        "addl $1, %%eax\n"
        "movl %%eax, %0\n"
        : "=m" (matrix[2][3][4])
        : "m" (matrix[1][2][3])
        : "%eax", "memory"
    );
    
    global_counter += val1 + val2 + val3 + chain_val + matrix[2][3][4];
}

/* ==================== Combined Test Function ==================== */
NOOPT void test_combined(void) {
    /* Combine all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
        volatile unsigned int c : 24;
    } bits = {1, 2, 3};
    
    bits.b = global_counter & 0x1F;
    
    /* STRICT_LOW_PART via volatile short */
    volatile unsigned int source = 0x87654321;
    volatile unsigned short low_part = source & 0xFFFF;
    
    /* SUBREG via union type-punning */
    union {
        uint64_t qword;
        uint32_t dwords[2];
    } u;
    u.qword = 0x1122334455667788ULL;
    u.dwords[1] = 0xAABBCCDD;  /* SUBREG store */
    
    /* Complex MEM_P via array with computed indices */
    volatile int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
    }
    
    int idx = (global_counter * 17 + 23) % 100;
    int complex_access = arr[idx] + arr[(idx * 7) % 100] + arr[(idx * 13) % 100];
    
    global_counter += bits.b + low_part + u.dwords[0] + complex_access;
}

/* ==================== Main Function ==================== */
int main(void) {
    /* Call all test functions multiple times with different conditions */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior in loops */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Dummy return to prevent optimization */
    return global_counter == 0 ? 0 : 1;
}
