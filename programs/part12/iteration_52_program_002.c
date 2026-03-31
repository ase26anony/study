/* test_resource.c - Test program to trigger uncovered lines in resource.cc */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Prevent optimizations that might eliminate our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */

/* Using bit-fields to generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int full : 32;
    volatile unsigned int part1 : 8;
    volatile unsigned int part2 : 4;
    volatile unsigned int part3 : 12;
    volatile unsigned int part4 : 8;
};

/* Using __builtin_bitfield for ZERO_EXTRACT */
NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to encourage ZERO_EXTRACT */
    bf.part1 = 0xAB;
    bf.part2 = 0x7;
    bf.part3 = 0xABC;
    bf.part4 = 0xCD;
    
    /* Write to full field to ensure all bits are touched */
    bf.full = 0xDEADBEEF;
    
    /* Use __builtin_bitfield if available */
    unsigned int value = 0x12345678;
    
    /* Extract and modify bits using bit operations that might generate ZERO_EXTRACT */
    unsigned int masked = value & 0xFF00FF00;
    unsigned int shifted = (value >> 8) & 0x00FF00FF;
    
    /* Combine operations */
    global_counter += bf.part1 + bf.part2 + bf.part3 + bf.part4 + masked + shifted;
}

/* Alternative approach with union for bit manipulation */
union bit_manip {
    volatile uint32_t full;
    struct {
        volatile uint32_t low16 : 16;
        volatile uint32_t high16 : 16;
    } parts;
};

NOOPT void test_zero_extract_union(void) {
    union bit_manip u;
    u.full = 0x12345678;
    
    /* Write to bit-fields - likely generates ZERO_EXTRACT */
    u.parts.low16 = 0xABCD;
    u.parts.high16 = 0xEF01;
    
    global_counter += u.full;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */

NOOPT void test_strict_low_part(void) {
    volatile char char_var;
    volatile short short_var;
    volatile int int_var;
    
    /* These assignments to smaller types may generate STRICT_LOW_PART */
    char_var = 0x42;
    short_var = 0x1234;
    
    /* Use inline assembly with % modifier for low part on x86 */
    int result;
    __asm__ volatile (
        "movl $0x12345678, %0\n\t"
        "movb $0x42, %b0\n\t"  /* %b0 for low byte */
        : "=r" (result)
        :
        : "cc"
    );
    
    /* More low-part operations */
    int_var = result;
    char_var = int_var & 0xFF;  /* Force low byte extraction */
    
    global_counter += char_var + short_var + int_var;
}

/* x86-specific low part operations */
NOOPT void test_strict_low_part_x86(void) {
    int x = 0;
    
    /* Inline assembly that explicitly uses low parts */
    __asm__ volatile (
        "movl $0xDEADBEEF, %0\n\t"
        "movw $0x1234, %w0\n\t"   /* %w0 for low word (16-bit) */
        "movb $0x56, %b0\n\t"     /* %b0 for low byte (8-bit) */
        : "=r" (x)
        :
        : "cc"
    );
    
    /* Force compiler to generate partial register stores */
    volatile short* sp = (volatile short*)&x;
    *sp = 0x789A;
    
    global_counter += x;
}

/* ==================== SUBREG Pattern ==================== */

/* Packed structure to force SUBREG accesses */
struct __attribute__((packed)) packed_data {
    char a;
    short b;
    int c;
    char d;
};

NOOPT void test_subreg(void) {
    struct packed_data pd;
    
    /* Initialize */
    pd.a = 1;
    pd.b = 2;
    pd.c = 3;
    pd.d = 4;
    
    /* Access misaligned members - may generate SUBREG */
    short b_copy = pd.b;  /* Could require SUBREG due to packing */
    int c_copy = pd.c;    /* Similarly misaligned */
    
    /* Type punning through union */
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } pun;
    
    pun.i = 0x12345678;
    
    /* Access sub-parts - likely generates SUBREG */
    uint16_t low_word = pun.s[0];
    uint16_t high_word = pun.s[1];
    uint8_t first_byte = pun.b[0];
    
    /* Vector operations that might use SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    
    /* Extract element - may use SUBREG */
    int elem = vec[2];
    
    global_counter += pd.a + b_copy + c_copy + pd.d + low_word + high_word + 
                     first_byte + elem;
}

/* ==================== MEM_P with Complex Addressing ==================== */

#define ARRAY_SIZE 100

NOOPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile int* ptr_array[ARRAY_SIZE];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 100 + j;
        }
        ptr_array[i] = &array[i][0];
    }
    
    /* Complex addressing patterns */
    int sum = 0;
    
    /* Multi-dimensional access with computation */
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Complex address computation */
            sum += array[i-1][j-1] + array[i][j] + array[i+1][j+1];
            
            /* Pointer arithmetic with multiple offsets */
            int* ptr = &array[i][j];
            sum += *(ptr - ARRAY_SIZE - 1) +  /* i-1, j-1 */
                   *(ptr + ARRAY_SIZE + 1);   /* i+1, j+1 */
        }
    }
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node* next;
        struct node* prev;
    };
    
    volatile struct node nodes[10];
    
    /* Initialize linked list */
    for (int i = 0; i < 10; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = (i < 9) ? &nodes[i+1] : NULL;
        nodes[i].prev = (i > 0) ? &nodes[i-1] : NULL;
    }
    
    /* Complex memory access through pointer chain */
    volatile struct node* current = &nodes[0];
    while (current && current->next && current->next->next) {
        sum += current->value + current->next->value + current->next->next->value;
        current = current->next;
    }
    
    /* Inline assembly with memory clobber */
    int temp = 0;
    __asm__ volatile (
        "movl $0x12345678, %0\n\t"
        "addl $0x11111111, %0\n\t"
        : "=m" (temp)
        :
        : "memory"
    );
    
    global_counter += sum + temp;
}

/* ==================== Combined Test Function ==================== */

NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field1 : 10;
        volatile unsigned int field2 : 22;
    } bits;
    bits.field1 = 0x3FF;
    bits.field2 = 0x3FFFFF;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int x = 0x12345678;
    volatile char* cp = (volatile char*)&x;
    *cp = 0x42;  /* Low byte assignment */
    
    /* SUBREG via packed struct */
    struct __attribute__((packed)) {
        char a;
        int b;
    } packed;
    packed.a = 1;
    packed.b = 2;
    int b_val = packed.b;  /* Likely SUBREG access */
    
    /* Complex MEM access */
    volatile int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i + j;
        }
    }
    
    /* Complex addressing */
    int complex_sum = 0;
    for (int i = 1; i < 9; i++) {
        for (int j = 1; j < 9; j++) {
            complex_sum += arr[i-1][j-1] + arr[i][j] + arr[i+1][j+1];
        }
    }
    
    global_counter += bits.field1 + bits.field2 + x + b_val + complex_sum;
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Call all test functions multiple times to ensure they're not optimized away */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_zero_extract_union();
        test_strict_low_part();
        test_strict_low_part_x86();
        test_subreg();
        test_complex_mem();
        test_combined();
    }
    
    /* Use the global counter to prevent dead code elimination */
    printf("Result: %d\n", global_counter);
    
    return 0;
}
