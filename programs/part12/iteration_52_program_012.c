/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Bit-field operations often generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to force ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0x7FF;
    bf.field4 = 0xCD;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | 0x3;
    
    /* Use __builtin_bitfield for explicit control */
    unsigned int value = 0x12345678;
    unsigned int extracted = __builtin_bitfield_extract(value, 8, 12);
    unsigned int inserted = __builtin_bitfield_insert(value, 0xAB, 4, 8);
    
    global_counter += bf.field1 + bf.field2 + extracted + inserted;
}

/* ========== STRICT_LOW_PART Pattern ========== */
/* Partial register updates on x86 */
NOOPT void test_strict_low_part(void) {
    volatile uint16_t short_var;
    volatile uint8_t char_var;
    volatile uint32_t int_var = 0x12345678;
    
    /* These assignments may generate STRICT_LOW_PART */
    short_var = (uint16_t)int_var;  /* Low 16-bit assignment */
    char_var = (uint8_t)(int_var >> 8); /* Byte assignment */
    
    /* Inline assembly with low-part modifier for x86 */
    uint32_t reg_val;
    __asm__ volatile (
        "movl $0x9ABCDEF0, %0\n\t"
        "movw $0x1234, %w0"  /* %w0 for low 16-bit */
        : "=r" (reg_val)
        :
        : "cc"
    );
    
    /* More partial assignments */
    volatile struct {
        uint8_t a;
        uint8_t b;
        uint16_t c;
    } __attribute__((packed)) packed_struct;
    
    packed_struct.a = 0xAA;
    packed_struct.b = 0xBB;
    packed_struct.c = 0xCCDD;
    
    global_counter += short_var + char_var + reg_val + packed_struct.c;
}

/* ========== SUBREG Pattern ========== */
/* Type punning and packed structures */
NOOPT void test_subreg(void) {
    /* Union for type punning */
    union pun {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts that may generate SUBREG */
    u.halves[0] = u.halves[1] + 0x100;
    u.bytes[2] = u.bytes[0] * 2;
    
    /* Vector operations (may generate SUBREG for element access) */
    typedef uint32_t v4ui __attribute__((vector_size(16)));
    v4ui vec = {1, 2, 3, 4};
    uint32_t element = vec[2];  /* Element extraction */
    
    /* Packed structure with misaligned access */
    struct __attribute__((packed)) misaligned {
        char a;
        int b;
        short c;
    } m;
    
    m.a = 'X';
    m.b = 0x12345678;
    m.c = 0x9ABC;
    
    /* Cast through different pointer types */
    uint32_t val32 = 0x87654321;
    uint16_t *ptr16 = (uint16_t*)&val32;
    uint16_t val16 = ptr16[1];  /* Access high 16 bits */
    
    global_counter += u.full + element + m.b + val16;
}

/* ========== MEM_P with Complex Addressing ========== */
/* Complex memory addressing patterns */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int arr[10][20][30];
    
    /* Complex address calculation */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                /* Multiple operations in address calculation */
                arr[i + 1][j * 2][k % 3] = 
                    arr[i][j][k] + 
                    arr[(i + j) % 10][(j + k) % 20][(k + i) % 30];
            }
        }
    }
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
        struct node *prev;
    };
    
    struct node nodes[10];
    
    /* Initialize linked list */
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = &nodes[i + 1];
        nodes[i].prev = (i > 0) ? &nodes[i - 1] : NULL;
    }
    nodes[9].value = 900;
    nodes[9].next = NULL;
    nodes[9].prev = &nodes[8];
    
    /* Complex pointer arithmetic */
    struct node *current = &nodes[0];
    int sum = 0;
    while (current) {
        /* Multiple dereferences with offset */
        sum += current->value;
        sum += (current->next) ? current->next->value : 0;
        sum += (current->prev) ? current->prev->value : 0;
        
        /* Array-like access through pointer */
        int *val_ptr = &current->value;
        sum += val_ptr[0];  /* Redundant but creates MEM_P pattern */
        
        current = current->next;
    }
    
    /* Inline assembly with memory operand */
    int mem_var = 42;
    __asm__ volatile (
        "addl $1, %0\n\t"
        "movl %0, %%eax\n\t"
        "addl %%eax, %1"
        : "+m" (mem_var), "+m" (global_counter)
        :
        : "eax", "cc"
    );
    
    global_counter += sum + arr[0][0][0] + mem_var;
}

/* ========== Combined Test Function ========== */
/* Function that combines multiple patterns */
NOOPT void test_combined(void) {
    /* Combined bit-field and memory access */
    struct combined {
        volatile unsigned int bits : 10;
        volatile int array[5];
    } comb;
    
    comb.bits = 0x3FF;  /* ZERO_EXTRACT */
    comb.array[2] = comb.array[1] + comb.bits;  /* Complex MEM_P */
    
    /* Union with partial access */
    union mix {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
    } u;
    
    u.full = 0x123456789ABCDEF0ULL;
    u.parts.low = u.parts.high & 0xFFFF;  /* STRICT_LOW_PART potential */
    
    /* Cast to different sizes */
    uint32_t *ptr32 = (uint32_t*)&u;
    uint16_t val16 = *(uint16_t*)ptr32;  /* SUBREG potential */
    
    global_counter += comb.bits + comb.array[2] + val16;
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Prevent dead code elimination */
    if (global_counter > 1000) {
        return 0;
    }
    
    return global_counter != 0 ? 0 : 1;
}
