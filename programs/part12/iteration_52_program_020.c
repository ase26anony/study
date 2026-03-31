/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimization from removing our patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ==================== ZERO_EXTRACT Pattern ==================== */
/* Bit-field operations often generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 4;
    volatile unsigned int field2 : 8;
    volatile unsigned int field3 : 12;
    volatile unsigned int padding : 8;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    
    /* Multiple bit-field writes to encourage ZERO_EXTRACT */
    bf.field1 = 5;
    bf.field2 = 0xAB;
    bf.field3 = 0x7FF;
    
    /* Complex bit-field expression */
    bf.field2 = (bf.field1 << 2) | 0x3;
    
    /* Force use to prevent elimination */
    global_counter += bf.field1 + bf.field2 + bf.field3;
}

/* ==================== STRICT_LOW_PART Pattern ==================== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These assignments to smaller types may generate STRICT_LOW_PART */
    s_val = i_val;          /* int to short */
    c_val = i_val;          /* int to char */
    
    /* Inline assembly that explicitly uses low-part modifiers */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (s_val)
        : "r" (i_val)
        : "ax"
    );
    
    /* Another approach: volatile char assignment */
    volatile char *p = (volatile char *)&i_val;
    *p = 0xFF;
    
    global_counter += s_val + c_val + *p;
}

/* ==================== SUBREG Pattern ==================== */
/* Packed structures and type punning often generate SUBREG */
struct __attribute__((packed)) packed_data {
    char a;
    int b;
    short c;
};

NOOPT void test_subreg(void) {
    struct packed_data pd;
    pd.a = 1;
    pd.b = 0xDEADBEEF;
    pd.c = 0x1234;
    
    /* Type punning through union */
    union {
        uint32_t full;
        uint16_t halves[2];
    } u;
    u.full = 0x87654321;
    
    /* Operations on sub-parts that may generate SUBREG */
    u.halves[0] = u.halves[1] + 0x100;
    
    /* Vector operations (may generate SUBREG on some targets) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Extract element (potential SUBREG) */
    int elem = v3[2];
    
    global_counter += pd.b + u.full + elem;
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
    
    /* Multi-dimensional access with computation */
    for (int i = 1; i < ARRAY_SIZE - 1; i++) {
        for (int j = 1; j < ARRAY_SIZE - 1; j++) {
            /* Complex address calculation */
            sum += array[i-1][j] + array[i][j-1] + 
                   array[i+1][j] + array[i][j+1];
        }
    }
    
    /* Pointer chain with offset */
    volatile int ***complex_ptr = (volatile int ***)&ptr_array;
    int val = *(*(*complex_ptr + 10) + 20);
    
    /* Inline assembly with memory clobber */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (array[50][50])
        : "m" (array[50][50])
        : "eax", "memory"
    );
    
    global_counter += sum + val + array[0][0];
}

/* ==================== Combined Test ==================== */
NOOPT void test_combined(void) {
    /* Combine multiple patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned int bits : 10;
    } bf;
    bf.bits = 0x3FF;
    
    /* STRICT_LOW_PART via type conversion */
    volatile int x = 0x12345678;
    volatile short y = x;
    
    /* SUBREG via packed structure */
    struct __attribute__((packed)) {
        char a;
        int b;
    } s;
    s.b = 0xDEADBEEF;
    
    /* Complex MEM access */
    volatile int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex address calculation */
    int idx = 5;
    volatile int *p = &arr[idx][idx];
    int val = *(p + idx * 2 + 3);
    
    global_counter += bf.bits + y + s.b + val;
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
        
        /* Modify global to prevent dead code elimination */
        global_counter += i;
    }
    
    /* Return something based on the computations */
    return (global_counter > 0) ? 0 : 1;
}
