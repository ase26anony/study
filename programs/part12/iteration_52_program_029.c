/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force memory operations */
volatile int global_counter = 0;

/* Structure for bit-field operations */
struct bit_container {
    volatile unsigned int full : 32;
    volatile unsigned int low_bits : 8;
    volatile unsigned int middle_bits : 16;
    volatile unsigned int high_bit : 1;
    volatile unsigned int padding : 7;
};

/* Packed structure for SUBREG generation */
struct __attribute__((packed)) packed_data {
    char a;
    short b;
    int c;
    char d;
};

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Union for type-punning */
union type_pun {
    uint32_t full;
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint8_t bytes[4];
};

/* ========== ZERO_EXTRACT Pattern ========== */
NOOPT void test_zero_extract(void) {
    /* Using volatile bit-fields to force ZERO_EXTRACT RTL */
    struct bit_container bc;
    
    /* These assignments should generate ZERO_EXTRACT for bit-field stores */
    bc.low_bits = 0xAB;
    bc.middle_bits = 0xCDEF;
    bc.high_bit = 1;
    
    /* Force compiler to actually generate the code */
    global_counter += bc.low_bits + bc.middle_bits;
}

/* ========== STRICT_LOW_PART Pattern ========== */
NOOPT void test_strict_low_part(void) {
    volatile char byte_var;
    volatile short short_var;
    volatile int int_var = 0x12345678;
    
    /* These should generate STRICT_LOW_PART for partial register updates */
    byte_var = (char)int_var;          /* Low byte assignment */
    short_var = (short)int_var;        /* Low word assignment */
    
    /* Inline assembly with % modifier for low part (x86 specific) */
    #ifdef __x86_64__
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (byte_var)
        : "r" (int_var)
        : "%eax"
    );
    #endif
    
    global_counter += byte_var + short_var;
}

/* ========== SUBREG Pattern ========== */
NOOPT void test_subreg(void) {
    /* Operations that should generate SUBREG RTL */
    union type_pun pun;
    pun.full = 0xDEADBEEF;
    
    /* Accessing sub-parts through union */
    uint16_t low_part = pun.parts.low;
    uint16_t high_part = pun.parts.high;
    
    /* Packed structure access */
    struct packed_data pd = {1, 2, 3, 4};
    short extracted = pd.b;  /* Should involve SUBREG */
    
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    int first_element = vec3[0];  /* Vector element access uses SUBREG */
    
    /* Type punning through pointer casts */
    uint32_t value = 0x12345678;
    uint16_t *half_ptr = (uint16_t*)&value;
    uint16_t half_value = *half_ptr;  /* Likely SUBREG */
    
    global_counter += low_part + high_part + extracted + first_element + half_value;
}

/* ========== Complex MEM_P Pattern ========== */
NOOPT void test_complex_mem(void) {
    /* Complex addressing modes for MEM_P with non-trivial address */
    volatile int array[100][100];
    volatile int *ptr_array[50];
    volatile struct {
        int a[10];
        struct {
            int x[5];
            int y[5];
        } nested;
    } complex_struct;
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    /* Complex array access with multiple indices */
    int sum = 0;
    for (int i = 1; i < 99; i++) {
        for (int j = 1; j < 99; j++) {
            /* Complex addressing: base + (i*100 + j)*sizeof(int) */
            sum += array[i][j] + array[i-1][j] + array[i][j-1];
        }
    }
    
    /* Pointer arithmetic with multiple offsets */
    volatile int *base_ptr = &array[0][0];
    for (int i = 0; i < 1000; i++) {
        /* Complex address calculation */
        int val = *(base_ptr + i + (i % 10) * 10);
        sum += val;
    }
    
    /* Structure with nested array access */
    for (int i = 0; i < 10; i++) {
        complex_struct.a[i] = i;
        if (i < 5) {
            complex_struct.nested.x[i] = i * 2;
            complex_struct.nested.y[i] = i * 3;
        }
    }
    
    /* Access with complex addressing */
    sum += complex_struct.nested.x[sum % 5];
    sum += complex_struct.nested.y[sum % 5];
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+m" (sum)
        :
        : "%eax", "memory"
    );
    
    global_counter += sum;
}

/* ========== Combined Test Function ========== */
NOOPT void test_combined(void) {
    /* Test all patterns in one function to maximize coverage */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int field : 4;
    } bf = {0};
    bf.field = 7;
    
    /* STRICT_LOW_PART via char assignment */
    volatile int src = 0x89ABCDEF;
    volatile char dst;
    dst = (char)src;
    
    /* SUBREG via union access */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u = {0x12345678};
    uint16_t half = u.halves[0];
    
    /* Complex MEM via multi-dimensional array */
    volatile int md_array[3][3][3];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                md_array[i][j][k] = i + j + k;
    
    int complex_access = md_array[1][1][1] + md_array[2][0][1];
    
    global_counter += bf.field + dst + half + complex_access;
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
        
        /* Modify global to prevent dead code elimination */
        global_counter += i;
    }
    
    /* Return the counter to prevent optimization */
    return global_counter == 0 ? 0 : 0;
}
