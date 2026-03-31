/* test_resource.c - Program to trigger uncovered RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations that might eliminate our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */

/* Bit-field structure that should generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Writing to bit-fields often generates ZERO_EXTRACT in RTL */
    bf.low_bits = 0xAB;        /* Should generate ZERO_EXTRACT for 8-bit field */
    bf.middle_bits = 0xCDEF;   /* Should generate ZERO_EXTRACT for 16-bit field */
    bf.high_bit = 1;           /* Should generate ZERO_EXTRACT for 1-bit field */
    
    /* Mix with computation to prevent optimization */
    global_counter += bf.low_bits + bf.middle_bits + bf.high_bit;
    
    /* Alternative: Using __builtin_bitfield */
    unsigned int val = 0x12345678;
    /* Set bits 8-15 to 0xFF */
    unsigned int result = __builtin_bitfield_insert(val, 0xFF, 8, 8);
    global_counter += result;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */

NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These assignments to smaller types may generate STRICT_LOW_PART */
    s_val = (short)i_val;      /* Low 16-bit part */
    c_val = (char)i_val;       /* Low 8-bit part */
    
    /* Inline assembly with %L0 modifier for x86 low-part constraint */
    int x = 42;
    int y;
    
    /* Force low-byte operation */
    asm volatile (
        "movb %b1, %b0"
        : "=r" (y)
        : "r" (x)
        : "cc"
    );
    
    global_counter += s_val + c_val + y;
    
    /* Another approach: volatile char assignment */
    volatile char *ptr = (volatile char *)&i_val;
    *ptr = 0xAA;  /* Modify low byte */
    
    global_counter += i_val;
}

/* ==================== SUBREG Pattern ==================== */

/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) packed_data {
    char a;
    short b;
    int c;
    char d;
};

NOOPT void test_subreg(void) {
    struct packed_data pd;
    pd.a = 1;
    pd.b = 2;
    pd.c = 3;
    pd.d = 4;
    
    /* Accessing members of packed struct often uses SUBREG */
    int sum = pd.a + pd.b + pd.c + pd.d;
    global_counter += sum;
    
    /* Type punning via union */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    u.full = 0xDEADBEEF;
    /* Accessing sub-parts may generate SUBREG */
    uint16_t low_half = u.halves[0];
    uint16_t high_half = u.halves[1];
    
    global_counter += low_half + high_half;
    
    /* Vector operations (SIMD) can generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Extract element - may use SUBREG */
    int elem = v3[2];
    global_counter += elem;
}

/* ==================== MEM_P with Complex Addressing ==================== */

#define ARRAY_SIZE 100

NOOPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile int *ptr_array[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
        ptr_array[i] = &array[i][0];
    }
    
    /* Complex addressing: multi-dimensional array with computation */
    int sum = 0;
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Complex address calculation */
            sum += array[i-1][j-1] + array[i][j] + array[i+1][j+1];
        }
    }
    global_counter += sum;
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
        struct node *prev;
    };
    
    volatile struct node nodes[10];
    for (int i = 0; i < 10; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = (i < 9) ? &nodes[i+1] : NULL;
        nodes[i].prev = (i > 0) ? &nodes[i-1] : NULL;
    }
    
    /* Complex memory access through pointer chain */
    volatile struct node *current = &nodes[0];
    int chain_sum = 0;
    while (current) {
        chain_sum += current->value;
        current = current->next;
    }
    global_counter += chain_sum;
    
    /* Inline assembly with memory clobber */
    int temp = 42;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (array[10][10])  /* Complex memory destination */
        : "r" (temp)
        : "%eax", "memory"
    );
}

/* ==================== Combined Test ==================== */

NOOPT void test_combined(void) {
    /* Combine multiple patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field1 : 4;
        volatile unsigned int field2 : 12;
    } bits;
    bits.field1 = 0xF;
    bits.field2 = 0xABC;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int x = 0x12345678;
    volatile char *cptr = (volatile char *)&x;
    *cptr = 0x42;  /* Modify low byte */
    
    /* SUBREG via packed struct */
    struct __attribute__((packed)) {
        char a;
        int b;
    } packed;
    packed.a = 1;
    packed.b = 2;
    int packed_sum = packed.a + packed.b;
    
    /* Complex MEM_P via array with index calculation */
    volatile int arr[50][50];
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            arr[i][j] = i * 50 + j;
        }
    }
    
    /* Complex addressing mode */
    int idx = global_counter % 25;
    int complex_access = arr[idx*2][idx+10] + arr[idx+5][idx*3];
    
    global_counter += bits.field1 + bits.field2 + x + packed_sum + complex_access;
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Use the global counter to prevent dead code elimination */
    if (global_counter > 1000) {
        return 1;  /* Should never happen, but prevents optimization */
    }
    
    return 0;
}
