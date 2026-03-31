/* dwarf_attributes_test.c - Test program to trigger specific DWARF attribute emission */

/* For DW_AT_explicit and DW_AT_mutable - C++ specific */
#ifdef __cplusplus
class TestClass {
private:
    mutable int mutable_member;  /* DW_AT_mutable */
    int regular_member;
    
public:
    explicit TestClass(int x) : mutable_member(x), regular_member(x) {}  /* DW_AT_explicit */
    
    void modify() const {
        mutable_member = 42;  /* Can modify even in const context */
    }
    
    int get_value() const {
        return mutable_member + regular_member;
    }
};
#endif

/* For DW_AT_is_optional and DW_AT_prototyped - function prototypes */
#ifdef __GNUC__
/* Function with prototype - DW_AT_prototyped */
int prototyped_function(int a, double b, const char* c) __attribute__((noinline));

/* Function that might trigger optional parameter attributes */
void function_with_complex_params(int required, 
#ifdef __cplusplus
                                  int optional = 0  /* DW_AT_is_optional in C++ */
#else
                                  int optional __attribute__((unused))
#endif
                                 );
#endif

/* For DW_AT_lower_bound and DW_AT_ordering - array attributes */
#ifdef __GNUC__
/* Array with GNU extension for specifying range - may trigger DW_AT_lower_bound */
int array_with_range[10][1...4];  /* GNU extension for array range */

/* Multi-dimensional array that might have ordering attribute */
int multi_dim_array[3][4][5];
#endif

/* For DW_AT_picture_string - decimal types (COBOL/Ada style) */
#ifdef __GNUC__
/* Try to create a decimal type using attribute */
typedef struct {
    unsigned char digits[10];
    signed char scale;
    unsigned char sign;
} __attribute__((packed)) decimal_type;
#endif

/* For DW_AT_string_length attributes - string types */
struct pascal_string {
    unsigned int length;  /* DW_AT_string_length */
    char data[];
};

/* For DW_AT_string_length_bit_size and DW_AT_string_length_byte_size */
#ifdef __GNUC__
typedef struct {
    unsigned short length_bits : 12;  /* bitfield for bit size */
    unsigned char length_bytes;
    char str[256];
} __attribute__((packed)) complex_string_type;
#endif

/* For DW_AT_segment - address space/segment attributes */
#ifdef __GNUC__
/* Pointer with address space attribute */
int __attribute__((address_space(256))) *segment_pointer;
#endif

/* For DW_AT_threads_scaled - OpenMP/multithreading */
#ifdef _OPENMP
#include <omp.h>
#endif

/* For DW_AT_small - packed/small types */
struct __attribute__((packed)) small_struct {
    char a;
    int b;
    char c;
};

/* For DW_AT_location - ensure variables have locations */
volatile int global_var = 42;

/* Function implementations */
#ifdef __GNUC__
int prototyped_function(int a, double b, const char* c) {
    return a + (int)b + (c ? *c : 0);
}

void function_with_complex_params(int required, int optional) {
    /* Use parameters to avoid warnings */
    volatile int sum = required + optional;
    (void)sum;
}
#endif

int main(void) {
    int result = 0;
    
    /* Use C++ features if compiling as C++ */
#ifdef __cplusplus
    TestClass obj(10);  /* Uses explicit constructor */
    obj.modify();
    result += obj.get_value();
#endif
    
    /* Call prototyped function */
#ifdef __GNUC__
    result += prototyped_function(1, 2.0, "test");
    function_with_complex_params(10, 20);
#endif
    
    /* Use arrays with special attributes */
#ifdef __GNUC__
    /* Initialize array elements */
    for (int i = 0; i < 10; i++) {
        for (int j = 1; j <= 4; j++) {
            array_with_range[i][j] = i * j;
            result += array_with_range[i][j];
        }
    }
    
    /* Use multi-dimensional array */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                multi_dim_array[i][j][k] = i + j + k;
                result += multi_dim_array[i][j][k];
            }
        }
    }
#endif
    
    /* Use decimal type */
#ifdef __GNUC__
    decimal_type dec = {{'1','2','3','4','5'}, 2, 0};
    result += dec.digits[0];
#endif
    
    /* Use string types */
    struct pascal_string* pstr = (struct pascal_string*)malloc(sizeof(struct pascal_string) + 10);
    if (pstr) {
        pstr->length = 5;
        memcpy(pstr->data, "Hello", 5);
        result += pstr->length;
        free(pstr);
    }
    
#ifdef __GNUC__
    complex_string_type cstr = {123, 5, "Test"};
    result += cstr.length_bits + cstr.length_bytes;
#endif
    
    /* Use segment pointer */
#ifdef __GNUC__
    int normal_var = 100;
    segment_pointer = (int __attribute__((address_space(256)))*)&normal_var;
    /* Note: actual use of address_space pointers requires target support */
#endif
    
    /* OpenMP section for threads_scaled */
#ifdef _OPENMP
    #pragma omp parallel
    {
        int thread_local_var = omp_get_thread_num();  /* May trigger threads_scaled */
        #pragma omp atomic
        result += thread_local_var;
    }
#endif
    
    /* Use small/packed struct */
    struct small_struct small = {'a', 42, 'b'};
    result += small.b;
    
    /* Use global variable with location */
    result += global_var;
    
    /* Return result to prevent optimization */
    return result > 0 ? 0 : 1;
}
