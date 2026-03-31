/* test_dwarf_attributes.c - Combined C/C++ test for uncovered DWARF attributes */

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

/* ============================================
   C constructs for string length attributes
   ============================================ */
struct string_desc {
    int length;
    /* Flexible array member with count attribute for DW_AT_string_length */
    char data[] __attribute__((count(length)));
};

/* Alternative: pointer with bounds attribute */
struct bounded_string {
    int size;
    char *ptr __attribute__((access(read_only, 2, 1)));
};

/* ============================================
   C++ constructs for specialized attributes
   ============================================ */
#ifdef __cplusplus

/* Class with explicit constructor for DW_AT_explicit */
class TestClass {
private:
    int value;
    
public:
    /* Explicit constructor */
    explicit TestClass(int v) : value(v) {}
    
    /* Mutable member for DW_AT_mutable */
    mutable int mutable_counter;
    
    /* Bit-field for DW_AT_small */
    unsigned int flag : 1;
    unsigned int small_field : 3;
    
    int getValue() const { return value; }
    void increment() const { mutable_counter++; }
};

/* Function prototypes for DW_AT_prototyped */
void prototype_func1(int x, double y);
void prototype_func2(const char* str);
int prototype_func3(void);

/* Function definitions */
void prototype_func1(int x, double y) {
    volatile int result = x + (int)y;
    (void)result;
}

void prototype_func2(const char* str) {
    volatile char first = str[0];
    (void)first;
}

int prototype_func3(void) {
    return 42;
}

/* Multidimensional array for potential DW_AT_ordering */
typedef int matrix_t[3][4][5];

/* Enum with specific ordering */
enum ByteOrder {
    LITTLE_ENDIAN,
    BIG_ENDIAN,
    PDP_ENDIAN
};

#endif /* __cplusplus */

/* ============================================
   Main function using all constructs
   ============================================ */
int main(void) {
    volatile int dummy = 0;
    
    /* 1. Use string descriptor struct for string length attributes */
    struct string_desc *str_desc = 0;
    struct bounded_string *bnd_str = 0;
    
    /* Force compiler to consider these types */
    dummy += (int)(long)str_desc;
    dummy += (int)(long)bnd_str;
    
#ifdef __cplusplus
    /* 2. Use C++ class with explicit constructor */
    volatile TestClass obj(42);
    obj.increment();
    
    /* Use bit-field */
    obj.flag = 1;
    obj.small_field = 3;
    
    dummy += obj.getValue();
    
    /* 3. Call prototyped functions */
    prototype_func1(10, 3.14);
    prototype_func2("test");
    dummy += prototype_func3();
    
    /* 4. Use multidimensional array */
    volatile matrix_t matrix = {};
    matrix[1][2][3] = dummy;
    
    /* 5. Enum for ordering */
    volatile enum ByteOrder order = LITTLE_ENDIAN;
    dummy += (int)order;
#endif
    
    /* Prevent optimization */
    printf("Result: %d\n", dummy);
    
    return 0;
}

/* Additional global variables to ensure external linkage */
#ifdef __cplusplus
extern "C" {
#endif

/* Packed struct for DW_AT_small */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

volatile struct PackedStruct packed_global = {'x', 123, 'y'};

/* Thread-related variable for DW_AT_threads_scaled */
#ifdef __cplusplus
thread_local int thread_var = 0;
#else
_Thread_local int thread_var = 0;
#endif

#ifdef __cplusplus
}
#endif
