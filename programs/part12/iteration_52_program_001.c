/* test_resource.c - Comprehensive test for resource.cc coverage */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
};

/* Also using __builtin_bitfield for modern GCC */
NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to increase chances of ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0x7FF;
    bf.field4 = 0xCD;
    
    /* Complex bit-field operation */
    bf.field2 = (bf.field1 << 2) | (bf.field3 & 0xF);
    
    /* Using __builtin_bitfield for explicit ZERO_EXTRACT */
    unsigned int value = 0x12345678;
    unsigned int result;
    
    /* Extract and insert bits */
    result = __builtin_bitfield_insert(value, 0xABC, 8, 12);
    result = __builtin_bitfield_extract(result, 4, 8);
    
    /* Prevent elimination */
    global_counter += bf.field1 + bf.field2 + bf.field3 + bf.field4 + result;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* Assign to low parts - may generate STRICT_LOW_PART */
    char_var = (char)int_var;          /* Low byte */
    short_var = (short)int_var;        /* Low word */
    
    /* Inline assembly with %L0 modifier for x86 */
    int temp;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (temp)
        : "r" (int_var)
        : "%eax"
    );
    
    /* More low-part operations */
    volatile struct {
        char a;
        short b;
        int c;
    } s;
    
    s.a = 0x42;
    s.b = 0x1234;
    s.c = 0x87654321;
    
    /* Chain of low-part assignments */
    char_var = s.a;
    s.b = (short)s.c;
    
    global_counter += char_var + short_var + temp + s.a + s.b;
}

/* ===== SUBREG Pattern ===== */
/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
    char d;
};

/* Union for type-punning */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
};

NOOPT void test_subreg(void) {
    struct packed_struct ps;
    ps.a = 1;
    ps.b = 0xDEADBEEF;
    ps.c = 0x1234;
    ps.d = 2;
    
    /* Access misaligned members - likely generates SUBREG */
    int b_copy = ps.b;  /* May require subreg due to packing */
    short c_copy = ps.c;
    
    /* Type-punning through union */
    union type_pun pun;
    pun.full = 0x12345678;
    
    /* Operations on sub-parts */
    pun.parts.low = pun.parts.high + 1;
    pun.bytes[1] = pun.bytes[3] * 2;
    
    /* Vector-style operations (may generate SUBREG) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Extract element - may use SUBREG */
    int elem = vec1[2];
    vec1[1] = elem + vec2[3];
    
    global_counter += ps.a + b_copy + c_copy + pun.full + elem;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
#define ARRAY_SIZE 100

struct complex_struct {
    int data[10][10];
    int extra[5];
    struct complex_struct *next;
};

NOOPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile struct complex_struct cs[5];
    volatile int *ptr_array[10];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                cs[i].data[x][y] = i * 100 + x * 10 + y;
            }
        }
        cs[i].next = (i < 4) ? &cs[i + 1] : NULL;
    }
    
    /* Complex addressing patterns */
    int sum = 0;
    
    /* Multi-dimensional with variable indices */
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Complex address calculation */
            sum += array[i-1][j] + array[i][j-1] + array[i+1][j] + array[i][j+1];
            
            /* Even more complex: array of pointers */
            ptr_array[i % 10] = &array[i][j];
            sum += *ptr_array[i % 10];
        }
    }
    
    /* Structure pointer chains with complex addressing */
    struct complex_struct *current = &cs[0];
    int depth = 0;
    while (current && depth < 5) {
        /* Complex nested array access */
        sum += current->data[depth % 10][(depth * 7) % 10];
        
        /* Pointer arithmetic in structure access */
        sum += current->extra[depth % 5];
        
        current = current->next;
        depth++;
    }
    
    /* Inline assembly with memory clobber */
    __asm__ volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+m" (sum)
        :
        : "%eax", "memory"
    );
    
    global_counter += sum;
}

/* ===== Combined Test Function ===== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    volatile struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 24;
    } bf_combined;
    
    bf_combined.a = 3;
    bf_combined.b = 0x1F;
    bf_combined.c = 0xFFFFFF;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int base = 0x89ABCDEF;
    volatile char low_byte = (char)base;
    
    /* SUBREG via packed structure */
    struct __attribute__((packed)) {
        char x;
        int y;
    } packed_combined;
    
    packed_combined.x = 42;
    packed_combined.y = 0x87654321;
    int y_copy = packed_combined.y;  /* Likely SUBREG */
    
    /* Complex MEM_P via array with index calculation */
    volatile int arr[50];
    for (int i = 0; i < 50; i++) {
        arr[i] = i * i;
    }
    
    int idx = (global_counter * 17) % 50;
    int complex_sum = arr[idx] + arr[(idx + 10) % 50] + arr[(idx + 25) % 50];
    
    /* Mix everything */
    bf_combined.b = low_byte;
    low_byte = (char)packed_combined.y;
    complex_sum += bf_combined.a + y_copy;
    
    global_counter += complex_sum;
}

/* ===== Main Function ===== */
int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Vary inputs slightly each iteration */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final dummy computation to prevent elimination */
    volatile int result = global_counter;
    
    /* Use result to prevent dead code elimination */
    __asm__ volatile ("" : : "r" (result));
    
    return 0;
}
