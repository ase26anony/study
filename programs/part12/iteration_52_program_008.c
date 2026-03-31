/* test_resource.c - Coverage test for mark_referenced_resources patterns */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual operations */
volatile int global_counter = 0;

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
    
    /* Multiple bit-field writes to force ZERO_EXTRACT patterns */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0x7FF;
    bf.field4 = 0xCD;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | (bf.field4 & 0x3);
    
    /* Use __builtin_bitfield for explicit ZERO_EXTRACT generation */
    unsigned int val = 0x12345678;
    unsigned int mask = 0xF00;
    unsigned int field = __builtin_bitfield_extract(val, 8, 4);
    __builtin_bitfield_insert(val, 0x5, 8, 4);
    
    global_counter += bf.field1 + bf.field2 + bf.field3 + bf.field4 + field;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */
/* Partial register updates often generate STRICT_LOW_PART */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These partial writes may generate STRICT_LOW_PART */
    s_val = i_val;          /* Low 16-bit write */
    c_val = i_val;          /* Low 8-bit write */
    
    /* Inline assembly with low-part modifier for x86 */
    int x = 42;
    int y = 99;
    
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (c_val)
        : "r" (x)
        : "%eax"
    );
    
    /* Another approach: volatile char assignment to force partial update */
    volatile char *p = (volatile char *)&i_val;
    p[0] = 0xFF;
    p[1] = 0xEE;
    
    global_counter += s_val + c_val + i_val;
}

/* ==================== SUBREG Pattern ==================== */
/* Type punning and packed structures generate SUBREG */
typedef struct __attribute__((packed)) {
    uint16_t a;
    uint32_t b;
    uint16_t c;
} packed_struct;

typedef union {
    uint64_t full;
    struct {
        uint32_t low;
        uint32_t high;
    } parts;
    uint16_t words[4];
} type_punning_union;

NOOPT void test_subreg(void) {
    packed_struct ps;
    ps.a = 0x1234;
    ps.b = 0x56789ABC;
    ps.c = 0xDEF0;
    
    /* Access through different types forces SUBREG */
    uint32_t *ptr = (uint32_t *)&ps.a;  /* Misaligned access */
    uint32_t val = *ptr;
    
    /* Union type punning */
    type_punning_union u;
    u.full = 0x123456789ABCDEF0ULL;
    u.words[1] = 0x5555;  /* SUBREG write to middle of register */
    
    /* Vector operations (may generate SUBREG on some targets) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* Extract element - may use SUBREG */
    
    /* Complex expression with SUBREG */
    uint64_t big = 0x1122334455667788ULL;
    uint32_t half = (big >> 16) & 0xFFFFFFFF;
    
    global_counter += ps.a + ps.c + val + u.words[0] + element + half;
}

/* ==================== MEM_P with Complex Addressing ==================== */
/* Complex memory addressing modes */
#define ARRAY_SIZE 100

typedef struct {
    int data[10][10];
    int more_data[5][20];
} complex_struct;

NOOPT void test_complex_mem(void) {
    volatile complex_struct cs[ARRAY_SIZE];
    volatile int multi_array[10][20][30];
    
    /* Initialize to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                cs[i].data[j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex addressing expressions */
    int sum = 0;
    volatile int * volatile ptr = &cs[0].data[0][0];
    
    /* Multiple complex memory accesses */
    sum += cs[5].data[3][7];
    sum += cs[global_counter % ARRAY_SIZE].data[2][4];
    sum += multi_array[3][7][11];
    sum += *(ptr + 50 + global_counter);
    
    /* Even more complex: pointer arithmetic with multiple indices */
    int idx1 = global_counter % 10;
    int idx2 = (global_counter / 10) % 20;
    int idx3 = (global_counter / 200) % 30;
    sum += multi_array[idx1][idx2][idx3];
    
    /* Structure pointer chain */
    complex_struct *cptr = &cs[10];
    sum += cptr->data[5][5];
    sum += (cptr + 5)->data[2][3];
    
    /* Inline assembly with memory clobber for complex addressing */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (cs[2].data[1][1])
        : "m" (cs[1].data[9][9])
        : "%eax", "memory"
    );
    
    global_counter += sum;
}

/* ==================== Combined Test Function ==================== */
/* Function that combines all patterns in sequence */
NOOPT void test_combined(void) {
    /* ZERO_EXTRACT pattern */
    struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
        volatile unsigned int c : 24;
    } bits;
    bits.a = 3;
    bits.b = bits.a << 2;
    bits.c = 0xFFFFFF;
    
    /* STRICT_LOW_PART pattern */
    volatile short low_part;
    volatile int source = 0x87654321;
    low_part = source;
    
    /* SUBREG pattern through union */
    union {
        uint32_t dword;
        uint16_t words[2];
    } reg;
    reg.dword = 0x12345678;
    reg.words[0] = 0xABCD;
    
    /* Complex MEM_P pattern */
    volatile int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    int complex_sum = arr[5][5] + arr[global_counter % 10][(global_counter / 10) % 10];
    
    global_counter += bits.a + bits.b + bits.c + low_part + reg.dword + complex_sum;
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
        global_counter += i;
    }
    
    /* Final dummy computation to prevent optimization */
    volatile int result = global_counter;
    
    return result != 0 ? 0 : 1;  /* Always return 0 unless everything was optimized away */
}
