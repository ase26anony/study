/* test_resource.c - Program to trigger specific RTL patterns in GCC's mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from eliminating our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual memory operations */
volatile int global_counter = 0;

/* ========== ZERO_EXTRACT Pattern ========== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
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
    bf.high_bit = 1;             /* Single bit write */
    
    /* Force compiler to actually generate the operations */
    global_counter += bf.low_bits + bf.middle_bits + bf.high_bit;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These should generate STRICT_LOW_PART for partial register updates */
    s_val = (short)i_val;        /* Store low 16 bits */
    c_val = (char)i_val;         /* Store low 8 bits */
    
    /* Use inline assembly with %L0 modifier for x86 low-part constraint */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (result)
        : "r" (i_val)
        : "%eax"
    );
    
    global_counter += s_val + c_val + result;
}

/* ========== SUBREG Pattern ========== */
/* Using unions and type-punning to generate SUBREG RTL */
union type_punner {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

NOOPT void test_subreg(void) {
    union type_punner u;
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts should generate SUBREG RTL */
    u.halves[0] += 0x1111;       /* Operation on low 16 bits */
    u.bytes[2] = 0xCC;           /* Operation on third byte */
    u.parts.high = u.parts.low;  /* Copy between sub-registers */
    
    /* Vector operations can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];        /* Extract element - may use SUBREG */
    
    global_counter += u.full + element;
}

/* ========== MEM_P with Complex Addressing ========== */
struct nested_struct {
    int data[16];
    struct nested_struct *next;
};

NOOPT void test_complex_mem(void) {
    /* Create complex addressing modes */
    struct nested_struct array[10];
    struct nested_struct *ptr = &array[3];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 16; j++) {
            array[i].data[j] = i * 100 + j;
        }
        if (i < 9) array[i].next = &array[i + 1];
    }
    array[9].next = NULL;
    
    /* Complex memory accesses with multiple index calculations */
    int sum = 0;
    
    /* Multi-dimensional array with complex index */
    sum += array[global_counter % 10].data[(global_counter * 7) % 16];
    
    /* Pointer chain with offset */
    sum += ptr->next->next->data[5];
    
    /* Array with structure pointer arithmetic */
    sum += ((struct nested_struct*)((char*)ptr + sizeof(struct nested_struct)))->data[3];
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (array[0].data[0])
        : "m" (array[1].data[1])
        : "%eax", "memory"
    );
    
    global_counter += sum;
}

/* ========== Combined Test Function ========== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field : 4;
    } bf;
    bf.field = 7;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int x = 0x12345678;
    volatile char *p = (volatile char*)&x;
    *p = 0xAA;  /* Modify low byte */
    
    /* SUBREG via union access */
    union {
        uint32_t i;
        uint16_t s[2];
    } u;
    u.i = 0x87654321;
    u.s[0] = 0x1111;  /* Modify low 16 bits */
    
    /* Complex MEM_P via pointer arithmetic */
    int arr[100];
    for (int i = 0; i < 100; i++) arr[i] = i;
    int idx = global_counter;
    int val = arr[idx * 2 + 3] + arr[(idx + 5) * 3 - 1];
    
    global_counter += bf.field + *p + u.i + val;
}

/* ========== Main Function ========== */
int main(void) {
    /* Call all test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change behavior in loops */
        global_counter++;
    }
    
    /* Dummy computation to prevent dead code elimination */
    int result = global_counter;
    
    /* Use result to satisfy compiler */
    asm volatile ("" : : "r" (result));
    
    return 0;
}
