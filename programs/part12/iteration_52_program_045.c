/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NO_OPT __attribute__((noinline, noipa))

/* Volatile to prevent dead code elimination */
static volatile int global_counter = 0;

/* ===== ZERO_EXTRACT pattern ===== */
/* Using bit-fields to trigger ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int low_bits : 8;
    unsigned int middle_bits : 16;
    unsigned int high_bit : 1;
    unsigned int reserved : 7;
} __attribute__((packed));

/* Volatile bit-field operations often generate ZERO_EXTRACT */
NO_OPT void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    
    /* Multiple bit-field writes to increase chances of ZERO_EXTRACT */
    bf.low_bits = 0xAB;
    bf.middle_bits = 0xCDEF;
    bf.high_bit = 1;
    
    /* Force use of the values */
    global_counter += bf.low_bits + bf.middle_bits + bf.high_bit;
    
    /* Another approach using __builtin_bitfield */
    unsigned int val = 0x12345678;
    /* Extract bits 8-15 */
    unsigned int extracted = __builtin_bitfield_extract(val, 8, 8);
    /* Insert into bits 16-23 */
    unsigned int inserted = __builtin_bitfield_insert(val, 0xFF, 16, 8);
    
    global_counter += extracted + inserted;
}

/* ===== STRICT_LOW_PART pattern ===== */
/* Using inline assembly with low-part modifiers */
NO_OPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    int temp;
    
    /* Disable optimization for this function to prevent coalescing */
    asm volatile("" : : : "memory");
    
    /* Operations that might generate STRICT_LOW_PART */
    s_val = 0x1234;
    c_val = 0xAB;
    
    /* Force partial register updates */
    {
        int x = 0xDEADBEEF;
        /* Low byte assignment */
        *(volatile char*)&x = 0xCC;
        global_counter += x;
    }
    
    /* Inline assembly with explicit low-part constraint (x86 specific) */
    #ifdef __x86_64__
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "movb $0xAA, %b0\n\t"  /* Low byte modifier %b0 */
        : "=r" (temp)
        :
        : "cc"
    );
    global_counter += temp;
    #endif
    
    /* Another approach: volatile char to int assignment */
    volatile int vi = 0x87654321;
    volatile char* vcp = (volatile char*)&vi;
    *vcp = 0x99;
    global_counter += vi;
}

/* ===== SUBREG pattern ===== */
/* Using type-punning and packed structures */
NO_OPT void test_subreg(void) {
    /* Union for type-punning */
    union pun {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    u.full = 0xDEADBEEF;
    
    /* Operations on sub-parts that might generate SUBREG */
    u.halves[0] = u.halves[0] + u.halves[1];
    u.bytes[2] = u.bytes[0] ^ u.bytes[1];
    
    global_counter += u.full;
    
    /* Using vector types for SUBREG patterns */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    
    /* Extract element - might use SUBREG */
    int elem = vec[2];
    vec[0] = elem * 2;
    
    global_counter += vec[0] + vec[1] + vec[2] + vec[3];
    
    /* Packed structure with misaligned access */
    struct __attribute__((packed)) misaligned {
        char c;
        int i;
        short s;
    } m;
    
    m.i = 0x12345678;
    m.s = m.s + 1;  /* Might involve SUBREG due to misalignment */
    
    global_counter += m.i + m.s;
}

/* ===== MEM_P with complex addressing ===== */
/* Complex memory addressing patterns */
#define ARRAY_SIZE 100

NO_OPT void test_complex_mem(void) {
    volatile int array[ARRAY_SIZE][ARRAY_SIZE];
    volatile int* ptrs[ARRAY_SIZE];
    volatile struct {
        int a;
        int b[10];
        struct {
            int x;
            int y;
        } inner;
    } complex_struct;
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptrs[i] = &array[i][0];
        for (int j = 0; j < ARRAY_SIZE; j++) {
            array[i][j] = i * 1000 + j;
        }
    }
    
    /* Complex addressing patterns */
    int sum = 0;
    
    /* Multi-dimensional with offset */
    sum += array[10][20];
    sum += array[global_counter % 50][(global_counter * 7) % 50];
    
    /* Pointer arithmetic with multiple indices */
    sum += *(ptrs[5] + 15);
    sum += *(ptrs[global_counter % 20] + (global_counter % 30));
    
    /* Structure with nested indexing */
    complex_struct.a = 100;
    complex_struct.b[5] = 200;
    complex_struct.inner.x = 300;
    complex_struct.inner.y = 400;
    
    sum += complex_struct.a;
    sum += complex_struct.b[complex_struct.a % 10];
    sum += complex_struct.inner.x + complex_struct.inner.y;
    
    /* Even more complex: pointer to pointer with offset */
    volatile int** pptr = &ptrs[10];
    sum += *(*pptr + 25);
    
    /* Array of structures */
    volatile struct {
        int data[5];
    } struct_array[20];
    
    for (int i = 0; i < 20; i++) {
        struct_array[i].data[2] = i * 100;
    }
    
    sum += struct_array[10].data[2];
    sum += struct_array[global_counter % 15].data[global_counter % 5];
    
    global_counter += sum;
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl $0, %0\n\t"
        : "=m" (array[50][50])
        :
        : "memory"
    );
}

/* ===== Combined test ===== */
/* Function that combines multiple patterns */
NO_OPT void test_combined(void) {
    /* Bit-field (ZERO_EXTRACT potential) */
    struct {
        unsigned int a : 4;
        unsigned int b : 12;
        unsigned int c : 16;
    } bits = {0};
    
    bits.a = 0xF;
    bits.b = 0xABC;
    bits.c = 0xDEAD;
    
    /* Partial register (STRICT_LOW_PART potential) */
    volatile short vs;
    volatile int vi = 0x12345678;
    vs = vi;  /* Truncation */
    
    /* Type punning (SUBREG potential) */
    union {
        uint64_t full;
        uint32_t parts[2];
    } u;
    u.full = 0x1122334455667788ULL;
    u.parts[0] = u.parts[0] + u.parts[1];
    
    /* Complex memory access */
    volatile int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    int idx1 = global_counter % 10;
    int idx2 = (global_counter * 3) % 10;
    global_counter += bits.a + bits.b + bits.c + vs + u.parts[0] + arr[idx1][idx2];
}

/* Main function to run all tests */
int main(void) {
    /* Run each test multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_complex_mem();
        test_combined();
        
        /* Modify global to change patterns */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final dummy computation to prevent elimination */
    volatile int result = global_counter;
    
    /* Use result to prevent dead code elimination */
    asm volatile ("" : : "r" (result));
    
    return 0;
}
