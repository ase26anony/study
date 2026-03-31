/* test_resource.c - Program to trigger specific RTL patterns for coverage testing */

#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */
/* Using bit-fields to trigger ZERO_EXTRACT in SET destination */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT RTL */
    bf.low_bits = 0xAB;          /* Writing to bit-field within larger int */
    bf.middle_bits = 0xCDEF;     /* Another bit-field write */
    bf.high_bit = 1;             /* Single bit assignment */
    
    /* Force compiler to generate code */
    global_counter += bf.low_bits + bf.middle_bits + bf.high_bit;
    
    /* Alternative using __builtin_bitfield */
    unsigned int val = 0x12345678;
    unsigned int extracted = __builtin_bitfield_extract(val, 4, 12);
    unsigned int inserted = __builtin_bitfield_insert(val, 0xABC, 8, 12);
    
    global_counter += extracted + inserted;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These assignments to smaller types may generate STRICT_LOW_PART */
    s_val = (short)i_val;        /* Store low 16 bits */
    c_val = (char)i_val;         /* Store low 8 bits */
    
    /* Inline assembly with low-part modifier for x86 */
    int x = 42;
    int y;
    
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax"
    );
    
    global_counter += s_val + c_val + y;
    
    /* Force partial register update through pointer */
    volatile char *byte_ptr = (volatile char *)&i_val;
    *byte_ptr = 0xFF;            /* Modify low byte only */
    
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
    pd.b = 0x1234;
    pd.c = 0x12345678;
    pd.d = 2;
    
    /* Operations on packed fields may generate SUBREG */
    int temp = pd.b;             /* Loading from misaligned short */
    temp += pd.c;                /* Combined operation */
    
    /* Type punning via union */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u;
    
    u.full = 0xDEADBEEF;
    u.halves[0] = 0xCAFE;        /* This may use SUBREG */
    
    /* Vector operations (if supported) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Extract element - may use SUBREG */
    int element = v3[2];
    
    global_counter += temp + u.full + element;
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
    
    /* Complex addressing patterns */
    int sum = 0;
    
    /* Multi-dimensional array with complex index */
    sum += array[global_counter % ARRAY_SIZE][(global_counter * 3) % ARRAY_SIZE];
    
    /* Pointer arithmetic with multiple offsets */
    volatile int *ptr = &array[0][0];
    ptr += global_counter;
    sum += *(ptr + (global_counter % 10) * 7);
    
    /* Structure pointer chain */
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
    
    struct node *current = &nodes[0];
    while (current) {
        sum += current->value;
        current = current->next;
    }
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+m" (array[5][5])
        :
        : "%eax", "memory"
    );
    
    global_counter += sum + array[5][5];
}

/* ==================== Combined Test Function ==================== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function to maximize coverage */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field : 10;
    } bf;
    bf.field = 0x3FF;
    
    /* STRICT_LOW_PART via byte store */
    volatile int x = 0x12345678;
    volatile char *p = (volatile char *)&x;
    *p = 0x42;
    
    /* SUBREG via packed struct */
    struct __attribute__((packed)) {
        char a;
        int b;
    } ps;
    ps.b = 0xDEADBEEF;
    int temp = ps.b;
    
    /* Complex MEM_P via array with complex index */
    volatile int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    int idx1 = global_counter % 10;
    int idx2 = (global_counter * 7) % 10;
    global_counter += bf.field + x + temp + arr[idx1][idx2];
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
    
    /* Dummy computation to prevent dead code elimination */
    volatile int result = global_counter;
    
    return result != 0 ? 0 : 1;  /* Always return 0 since global_counter won't be 0 */
}
