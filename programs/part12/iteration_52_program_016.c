/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */
/* Bit-field operations often generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to ensure ZERO_EXTRACT generation */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0x7FF;
    bf.field4 = 0xCD;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | (bf.field4 & 0x3);
    
    global_counter += bf.field1 + bf.field2 + bf.field3 + bf.field4;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */
/* Partial register updates often generate STRICT_LOW_PART */
NOOPT void test_strict_low_part(void) {
    volatile uint32_t full_reg;
    volatile uint16_t half_reg;
    volatile uint8_t byte_reg;
    
    /* Initialize */
    full_reg = 0x12345678;
    
    /* Partial register writes - may generate STRICT_LOW_PART */
    half_reg = 0xABCD;
    byte_reg = 0xEF;
    
    /* Force partial register update through pointer casting */
    *(volatile uint16_t*)&full_reg = 0x9876;
    *(volatile uint8_t*)&full_reg = 0x42;
    
    /* Inline assembly for explicit low-part constraint (x86 specific) */
    #ifdef __x86_64__
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movw $0xABCD, %%ax\n\t"
        "movb $0xEF, %%al\n\t"
        : : : "eax", "memory"
    );
    #endif
    
    global_counter += full_reg + half_reg + byte_reg;
}

/* ==================== SUBREG Pattern ==================== */
/* Type punning and packed structures often generate SUBREG */
struct packed_data {
    uint32_t a;
    uint16_t b;
    uint8_t c;
} __attribute__((packed));

NOOPT void test_subreg(void) {
    struct packed_data pd;
    uint32_t combined;
    
    /* Initialize */
    pd.a = 0xDEADBEEF;
    pd.b = 0xCAFE;
    pd.c = 0x42;
    
    /* Type punning through union - often generates SUBREG */
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } pun;
    
    pun.full = 0x12345678;
    pun.parts.low = 0xABCD;  /* This may generate SUBREG */
    
    /* Extract and manipulate sub-parts */
    combined = (pd.b << 8) | pd.c;
    combined = (pun.parts.high << 16) | pun.parts.low;
    
    /* Vector operations can also generate SUBREG */
    typedef uint32_t v2u32 __attribute__((vector_size(8)));
    v2u32 vec = {0x11111111, 0x22222222};
    uint32_t elem = vec[0];  /* May generate SUBREG for extraction */
    
    global_counter += pd.a + combined + elem;
}

/* ==================== MEM_P with Complex Addressing Pattern ==================== */
/* Complex memory addressing expressions */
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
    
    /* Complex addressing patterns */
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Multiple complex memory accesses */
            result += array[i-1][j-1] + array[i][j] + array[i+1][j+1];
            result += *(ptr_array[i] + j);
            result += array[j][i];  /* Transposed access */
        }
    }
    
    /* Structure with pointer chains */
    struct node {
        volatile int value;
        volatile struct node *next;
    };
    
    struct node nodes[10];
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i+1];
    }
    nodes[9].value = 90;
    nodes[9].next = NULL;
    
    /* Pointer chasing with complex addressing */
    volatile struct node *current = &nodes[0];
    while (current) {
        result += current->value;
        current = current->next;
    }
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl $0, %%eax\n\t"
        "addl array+400(%%rip), %%eax\n\t"
        "addl array+804(%%rip), %%eax\n\t"
        : : : "eax", "memory"
    );
    
    global_counter += result;
}

/* ==================== Combined Test ==================== */
/* Function that combines multiple patterns */
NOOPT void test_combined(void) {
    /* Bit-field (ZERO_EXTRACT) */
    struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
    } bf = {0};
    
    bf.a = 3;
    bf.b = 7;
    
    /* Partial register (STRICT_LOW_PART) */
    volatile uint32_t reg = 0xFFFFFFFF;
    *(volatile uint16_t*)&reg = 0x1234;
    
    /* Type punning (SUBREG) */
    union {
        uint64_t full;
        uint32_t halves[2];
    } u;
    u.full = 0x1122334455667788ULL;
    u.halves[0] = 0xAABBCCDD;
    
    /* Complex memory access */
    volatile int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 5 + j;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += matrix[i][i];  /* Diagonal access */
        sum += matrix[4-i][i];  /* Anti-diagonal */
    }
    
    global_counter += bf.a + bf.b + reg + u.halves[0] + sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    /* Call all test functions multiple times to ensure execution */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Use the global counter to prevent dead code elimination */
    if (global_counter > 1000) {
        return 0;
    }
    
    return 0;
}
