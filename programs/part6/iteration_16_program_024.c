/* test_dwarf_attributes.c */
/* Compile with: g++ -O0 -g3 -std=c++11 -c test_dwarf_attributes.c -o test.o */

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

/* ====== C constructs for string length attributes ====== */

/* Struct with counted string attribute - triggers DW_AT_string_length* attributes */
struct string_desc {
    int length;
    /* Using count attribute to hint at string length relationship */
    char data[] __attribute__((count(length)));
};

/* Another approach with pointer and bounds attribute */
struct bounded_string {
    int len;
    char *str __attribute__((bnd_variable_size(len)));
};

/* ====== C++ constructs for other attributes ====== */
#ifdef __cplusplus

/* Class with explicit constructor - triggers DW_AT_explicit */
class TestClass {
private:
    int value;
    
public:
    /* Explicit constructor */
    explicit TestClass(int v) : value(v) {}
    
    /* Mutable member - triggers DW_AT_mutable */
    mutable int cache __attribute__((unused));
    
    /* Method with prototype - triggers DW_AT_prototyped */
    int getValue() const;
    
    /* Small/bit-field members - may trigger DW_AT_small */
    struct {
        unsigned int flag : 1;
        unsigned int status : 3;
        unsigned int : 4;  /* padding */
    } __attribute__((packed)) bits;
};

int TestClass::getValue() const {
    cache = 42;  /* Use mutable member */
    return value;
}

/* Function prototypes - ensure DW_AT_prototyped */
void prototype_func1(int, double);
int prototype_func2(const char*);

void prototype_func1(int a, double b) {
    volatile int unused = a + (int)b;
    (void)unused;
}

int prototype_func2(const char* str) {
    return str ? str[0] : 0;
}

/* Multidimensional array - may trigger DW_AT_ordering */
typedef int matrix_t[3][4][5];

/* Enum with specific values - may influence ordering */
enum Ordering {
    ASCENDING,
    DESCENDING
};

/* Template class - may generate additional debug info */
template<typename T>
class SmallContainer {
    T data[16];
public:
    explicit SmallContainer(T init) {
        for (int i = 0; i < 16; i++) data[i] = init + i;
    }
    T get(int idx) const { return data[idx]; }
};

#endif /* __cplusplus */

/* ====== Main function to use all constructs ====== */
int main() {
    volatile int result = 0;
    
    /* Use string descriptor struct */
    struct string_desc *desc = nullptr;
    volatile int str_len = 10;
    (void)str_len;  /* Prevent unused warning */
    
    /* Use bounded string */
    struct bounded_string bstr = {0, nullptr};
    result += bstr.len;
    
#ifdef __cplusplus
    /* Use C++ class with explicit constructor */
    volatile TestClass obj(42);
    result += obj.getValue();
    
    /* Use mutable member */
    obj.cache = 100;
    result += obj.cache;
    
    /* Use bit-fields */
    obj.bits.flag = 1;
    obj.bits.status = 3;
    result += obj.bits.flag + obj.bits.status;
    
    /* Call prototyped functions */
    prototype_func1(1, 2.0);
    result += prototype_func2("test");
    
    /* Use multidimensional array */
    volatile matrix_t matrix = {};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 5; k++)
                matrix[i][j][k] = i + j + k;
    
    result += matrix[0][0][0];
    
    /* Use template class */
    SmallContainer<int> container(10);
    result += container.get(0);
    
    /* Use ordering enum */
    volatile Ordering order = ASCENDING;
    result += (order == ASCENDING) ? 1 : 0;
#endif
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
