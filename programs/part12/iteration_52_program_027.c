/* test_resource.c - Coverage test for mark_referenced_resources patterns */

#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force memory operations */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int low3 : 3;
    volatile unsigned int mid5 : 5;
    volatile unsigned int high8 : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT in RTL */
    bf.low3 = 5;           /* Writing to bit-field within larger int */
    bf.mid5 = 13;          /* Another bit-field write */
    bf.high8 = 0xFF;       /* 8-bit field within 32-bit container */
    
    /* Use __builtin_bitfield for explicit ZERO_EXTRACT */
    unsigned int val = 0x12345678;
    unsigned int field = __builtin_bitfield_extract(val, 4, 8);  /* Extract bits 4-11 */
    __builtin_bitfield_insert(val, 0xAB, 12, 4);                 /* Insert at bits 12-15 */
    
    global_counter += bf.low3 + field;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* These partial register writes may generate STRICT_LOW_PART */
    char_var = 0x42;        /* Low byte assignment */
    short_var = 0xABCD;     /* Low word assignment */
    
    /* Inline assembly with low-part modifier (x86 specific) */
    int result;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movb $0x99, %%al\n\t"      /* Modify only low byte */
        "movl %%eax, %0"
        : "=r" (result)
        : "r" (int_var)
        : "%eax"
    );
    
    /* Force compiler to generate STRICT_LOW_PART with volatile */
    *(volatile char*)&int_var = 0x77;  /* Store to low byte */
    
    global_counter += char_var + short_var + result;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Use unions for type-punning to generate SUBREG */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } pun;
    
    pun.full = 0xDEADBEEF;
    
    /* Operations on sub-parts should generate SUBREG */
    pun.halves[0] += 0x1234;      /* Low 16-bit operation */
    pun.bytes[3] = 0xAA;          /* High byte operation */
    
    /* Vector types can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];          /* Extract element via SUBREG */
    
    /* Packed structure */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } ps;
    
    ps.b = 0x12345678;            /* Misaligned access may use SUBREG */
    
    global_counter += pun.halves[0] + element + ps.b;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int arr[10][10][10];
    int i, j, k;
    
    /* Complex addressing expression */
    for (i = 1; i < 9; i++) {
        for (j = 1; j < 9; j++) {
            for (k = 1; k < 9; k++) {
                /* This should generate complex address calculation */
                arr[i][j][k] = arr[i-1][j][k] + 
                               arr[i][j-1][k] + 
                               arr[i][j][k-1];
            }
        }
    }
    
    /* Structure pointer chain */
    struct node {
        int value;
        struct node *next;
        int data[5];
    };
    
    struct node nodes[10];
    struct node *ptr = &nodes[0];
    
    /* Chain of pointer accesses */
    for (i = 0; i < 9; i++) {
        nodes[i].next = &nodes[i+1];
        nodes[i].value = i * 100;
    }
    
    /* Complex memory access through pointer chain */
    ptr->next->next->next->data[2] = 999;
    
    /* Inline assembly with memory clobber */
    int temp = 0;
    __asm__ volatile (
        "movl $1, %0\n\t"
        : "=m" (arr[5][5][5])  /* Memory output */
        :
        : "memory"
    );
    
    global_counter += arr[5][5][5] + ptr->next->value;
}

/* ===== Combined Test Function ===== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned bits : 4;
    } bf;
    bf.bits = 7;
    
    /* STRICT_LOW_PART via char store */
    volatile int x = 0x12345678;
    *(volatile char*)&x = 0x42;
    
    /* SUBREG via union access */
    union {
        uint32_t i;
        uint16_t s;
    } u;
    u.i = 0xABCD1234;
    u.s = 0x5678;
    
    /* MEM_P with complex address */
    volatile int array[100];
    int idx = global_counter % 100;
    array[idx * 2 + 1] = array[idx * 3] + array[idx + 10];
    
    global_counter += bf.bits + x + u.i + array[0];
}

/* ===== Main Driver ===== */
int main(void) {
    /* Call all test functions multiple times to ensure coverage */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Dummy computation to prevent dead code elimination */
    int sum = global_counter;
    
    /* Use the result to prevent optimization */
    volatile int *sink = &sum;
    
    return sum == 0 ? 0 : 1;
}
