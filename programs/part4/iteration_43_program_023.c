/* Test for constant-bounded memory operations in GCC expr.cc */
#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count = 1 (triggers count <= 2 path) */
static int test_mem_count_1(void) {
    int src[5] = {1, 2, 3, 4, 5};
    int dst[5] = {0};
    
    /* Constant bounds: copy single element */
    const int lo = 2;
    const int hi = 2;
    dst[lo] = src[hi];
    
    return dst[2];
}

/* Test 2: MEM target with count = 2 (triggers count <= 2 path) */
static int test_mem_count_2(void) {
    struct { int a[3]; } s1 = {{10, 20, 30}};
    struct { int a[3]; } s2 = {{0}};
    
    /* Constant bounds: copy two elements */
    const int start = 0;
    const int end = 1;
    for (int i = start; i <= end; i++) {
        s2.a[i] = s1.a[i];
    }
    
    return s2.a[0] + s2.a[1];
}

/* Test 3: MEM target with count > 2 but small total size (char array) */
static int test_mem_small_total_size(void) {
    char src[10] = "abcdefghi";
    char dst[10] = {0};
    
    /* Constant bounds: copy 5 chars (5 bytes total) */
    const int lo = 1;
    const int hi = 5;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test 4: MEM target with short array (2 bytes each, count=3 → 6 bytes) */
static int test_mem_short_array(void) {
    short src[5] = {100, 200, 300, 400, 500};
    short dst[5] = {0};
    
    /* Constant bounds: copy 3 shorts */
    enum { LO = 1, HI = 3 };
    for (int i = LO; i <= HI; i++) {
        dst[i] = src[i];
    }
    
    return dst[1] + dst[2] + dst[3];
}

/* Test 5: Non-MEM target (bit-field extraction into register) */
static int test_non_mem_target(void) {
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } bf = {1, 2, 3, 4};
    
    /* Extract small constant-sized bit-field - likely goes to register */
    unsigned int extracted = 0;
    extracted |= (bf.a << 12);
    extracted |= (bf.b << 8);
    extracted |= (bf.c << 4);
    extracted |= bf.d;
    
    return extracted;
}

/* Test 6: Compound literal with constant bounds */
static int test_compound_literal(void) {
    int *arr = (int[]){10, 20, 30, 40, 50};
    int sum = 0;
    
    /* Constant bounds access */
    const int start = 1;
    const int stop = 3;
    for (int i = start; i <= stop; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* Test 7: Structure copy with constant size (small struct) */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {'x', 'y', 'z'};
    
    struct small s2;
    
    /* This should trigger constant-bounds analysis for the whole struct */
    s2 = s1;
    
    return s2.a + s2.b + s2.c;
}

/* Test 8: Array initialization with constant loop bounds */
static int test_array_init(void) {
    int arr[8];
    
    /* Constant loop bounds */
    for (int i = 0; i < 4; i++) {
        arr[i] = i * 10;
    }
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 9: Mixed types to test TYPE_SIZE calculation */
static int test_mixed_types(void) {
    typedef struct {
        char c;
        short s;
        int i;
    } mixed;
    
    mixed src[3] = {{'a', 100, 1000}, {'b', 200, 2000}, {'c', 300, 3000}};
    mixed dst[3];
    
    /* Copy 2 elements - each is 8+ bytes (padding), total > 16 bytes */
    const int lo = 0;
    const int hi = 1;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    return dst[0].c + dst[0].s + dst[0].i + dst[1].c + dst[1].s + dst[1].i;
}

/* Test 10: Boolean array (small element size) */
static int test_bool_array(void) {
    _Bool src[16] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
    _Bool dst[16] = {0};
    
    /* Copy 8 bools - total size very small */
    const int lo = 4;
    const int hi = 11;
    for (int i = lo; i <= hi; i++) {
        dst[i] = src[i];
    }
    
    int count = 0;
    for (int i = lo; i <= hi; i++) {
        count += dst[i];
    }
    return count;
}

int main(void) {
    int total = 0;
    
    total += test_mem_count_1();      /* Should hit count=1 path */
    total += test_mem_count_2();      /* Should hit count=2 path */
    total += test_mem_small_total_size(); /* char array, small total */
    total += test_mem_short_array();  /* short array, small total */
    total += test_non_mem_target();   /* Non-MEM target path */
    total += test_compound_literal(); /* Compound literal */
    total += test_struct_copy();      /* Small struct copy */
    total += test_array_init();       /* Array init with const bounds */
    total += test_mixed_types();      /* Mixed type struct */
    total += test_bool_array();       /* Boolean array */
    
    printf("Result: %d\n", total);
    return 0;
}
