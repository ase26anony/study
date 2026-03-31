/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count <= 2 */
static int test_small_array_init(void) {
    /* Initialize first 2 elements of array - count = 2 */
    int arr[10] = {1, 2};  /* Only first 2 initialized explicitly */
    
    /* Force compiler to consider constant bounds for initialization */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        sum += arr[i];
    }
    return sum;  /* Should be 3 */
}

/* Test 2: MEM target with count > 2 but small total size */
static int test_char_array_init(void) {
    /* 10 chars = 10 bytes total, small enough for scalar expansion */
    char buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[i];
    }
    return sum;  /* Should be 55 */
}

/* Test 3: Non-MEM target (register operations) */
static int test_bitfield_extract(void) {
    /* Create a constant value with known bit pattern */
    unsigned long value = 0x12345678ABCDEF00UL;
    
    /* Extract small constant-sized bitfields into registers */
    unsigned int part1 = (value >> 0) & 0xFF;   /* Extract byte 0 */
    unsigned int part2 = (value >> 8) & 0xFF;   /* Extract byte 1 */
    unsigned int part3 = (value >> 16) & 0xFF;  /* Extract byte 2 */
    
    return part1 + part2 + part3;  /* Should be 0x00 + 0xEF + 0xCD = 0x1BC (444) */
}

/* Test 4: Array slice copy with constant bounds */
static int test_array_slice_copy(void) {
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5];
    
    /* Copy slice with constant bounds: indices 1-3 inclusive (count = 3) */
    const int lo = 1;
    const int hi = 3;
    
    /* This should create a constant-bounded copy operation */
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[1] + dst[2] + dst[3];  /* Should be 20+30+40 = 90 */
}

/* Test 5: Structure copy with constant size */
static int test_struct_copy(void) {
    struct SmallStruct {
        char a;
        char b;
        char c;
    };
    
    struct SmallStruct s1 = {1, 2, 3};
    struct SmallStruct s2;
    
    /* Structure copy - constant size of 3 bytes */
    s2 = s1;
    
    return s2.a + s2.b + s2.c;  /* Should be 6 */
}

/* Test 6: Mixed types with sizeof for bounds */
static int test_mixed_types(void) {
    /* Use sizeof to create constant bounds */
    short data[4];
    
    /* Initialize using constant loop bounds derived from sizeof */
    for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
        data[i] = (short)(i * 10);
    }
    
    return data[0] + data[1] + data[2] + data[3];  /* Should be 0+10+20+30=60 */
}

/* Test 7: Nested constant operations */
static int test_nested_constants(void) {
    enum { START = 0, END = 2 };  /* Compile-time constants */
    int values[5];
    
    /* Constant-bounded initialization */
    for (int i = START; i <= END; i++) {
        values[i] = i * 100;
    }
    
    return values[0] + values[1] + values[2];  /* Should be 0+100+200=300 */
}

/* Test 8: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int array[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int *ptr = array;
    
    /* Access with constant offsets */
    int a = ptr[0];  /* offset 0 */
    int b = ptr[2];  /* offset 2*sizeof(int) */
    int c = ptr[4];  /* offset 4*sizeof(int) */
    
    return a + b + c;  /* Should be 0+2+4=6 */
}

int main(void) {
    int total = 0;
    
    total += test_small_array_init();      /* +3 */
    total += test_char_array_init();       /* +55 */
    total += test_bitfield_extract();      /* +444 */
    total += test_array_slice_copy();      /* +90 */
    total += test_struct_copy();           /* +6 */
    total += test_mixed_types();           /* +60 */
    total += test_nested_constants();      /* +300 */
    total += test_pointer_arithmetic();    /* +6 */
    
    printf("Total: %d\n", total);  /* Should print 964 */
    
    /* Verify expected total */
    if (total == 964) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Test failed! Expected 964, got %d\n", total);
        return 1;
    }
}
