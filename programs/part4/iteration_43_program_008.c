/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 4;
    } s = {5, 123, 9};
    
    /* Extract small constant-sized bitfield - should trigger !MEM_P(target) path */
    unsigned int extracted = s.b;  /* 8-bit field extraction */
    return extracted == 123 ? 1 : 0;
}

/* Test 2: MEM target with count <= 2 - small array initialization */
static int test_small_array_init(void) {
    int arr[5];
    
    /* Initialize first 2 elements - count = 2 */
    arr[0] = 42;
    arr[1] = 43;
    
    /* Also test with compound literal copy */
    int src[2] = {100, 200};
    arr[2] = src[0];
    arr[3] = src[1];
    
    return (arr[0] == 42 && arr[1] == 43 && arr[2] == 100 && arr[3] == 200) ? 2 : 0;
}

/* Test 3: MEM target with count = 1 - single element copy */
static int test_single_element(void) {
    struct Point {
        int x, y;
    } p1 = {10, 20}, p2;
    
    /* Single structure copy - count = 1 for the structure */
    p2 = p1;
    
    return (p2.x == 10 && p2.y == 20) ? 3 : 0;
}

/* Test 4: MEM target with larger count but small total size - char array */
static int test_small_char_array(void) {
    char buffer[10];
    
    /* Initialize all 10 chars - count = 10, but element size = 1 byte */
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A' + i;
    }
    
    /* Also test with memcpy of constant size */
    char src[10] = "0123456789";
    char dest[10];
    
    /* This should be recognized as constant-bounded copy */
    for (int i = 0; i < 10; i++) {
        dest[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += dest[i] - '0';
    }
    
    return (sum == 45) ? 4 : 0;  /* 0+1+2+...+9 = 45 */
}

/* Test 5: MEM target with short array - medium element size */
static int test_short_array(void) {
    short data[4] = {100, 200, 300, 400};
    short copy[4];
    
    /* Copy 4 shorts - count = 4, element size = 2 bytes, total = 8 bytes */
    for (int i = 0; i < 4; i++) {
        copy[i] = data[i];
    }
    
    return (copy[0] == 100 && copy[1] == 200 && 
            copy[2] == 300 && copy[3] == 400) ? 5 : 0;
}

/* Test 6: Mixed operations to encourage different code paths */
static int test_mixed_operations(void) {
    int result = 0;
    
    /* Multiple small constant operations */
    int a[3] = {1, 2, 3};
    int b[3];
    
    /* Copy 3 elements - might trigger different threshold */
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
    
    result += (b[0] == 1 && b[1] == 2 && b[2] == 3) ? 1 : 0;
    
    /* Bitfield operation */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 1;
        unsigned int value : 6;
    } flags = {1, 0, 32};
    
    unsigned int val = flags.value;
    result += (val == 32) ? 1 : 0;
    
    return result == 2 ? 6 : 0;
}

/* Test 7: Constant indices with computed bounds */
static int test_constant_indices(void) {
    int array[10];
    
    /* Use constant expressions for bounds */
    const int start = 2;
    const int end = 5;
    
    /* Initialize slice - count = end - start + 1 = 4 */
    for (int i = start; i <= end; i++) {
        array[i] = i * 10;
    }
    
    return (array[2] == 20 && array[3] == 30 && 
            array[4] == 40 && array[5] == 50) ? 7 : 0;
}

/* Test 8: Boolean array - very small element size */
static int test_bool_array(void) {
    _Bool flags[16];  /* _Bool is typically 1 byte */
    
    /* Initialize all 16 bools */
    for (int i = 0; i < 16; i++) {
        flags[i] = (i % 2) == 0;
    }
    
    int true_count = 0;
    for (int i = 0; i < 16; i++) {
        if (flags[i]) true_count++;
    }
    
    return (true_count == 8) ? 8 : 0;  /* Half should be true */
}

int main(void) {
    int total = 0;
    
    total += test_bitfield_extraction();      /* Should trigger !MEM_P(target) */
    total += test_small_array_init();         /* Should trigger count <= 2 for MEM */
    total += test_single_element();           /* Should trigger count = 1 */
    total += test_small_char_array();         /* Should trigger TYPE_SIZE * count check */
    total += test_short_array();              /* Medium element size */
    total += test_mixed_operations();         /* Mixed patterns */
    total += test_constant_indices();         /* Computed constant bounds */
    total += test_bool_array();               /* Very small element type */
    
    printf("Result: %d\n", total);
    
    /* Expected result: 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 = 36 */
    return total == 36 ? 0 : 1;
}
