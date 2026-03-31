/* dwarf_attributes.c - Program to trigger specific DWARF attribute emission */

/* For DW_AT_explicit and DW_AT_mutable */
#ifdef __cplusplus
class TestClass {
private:
    mutable int mutable_member;  /* Should trigger DW_AT_mutable */
    int regular_member;
    
public:
    explicit TestClass(int x) : mutable_member(x), regular_member(x) {}  /* Should trigger DW_AT_explicit */
    
    void modify() const {
        mutable_member = 42;  /* Can modify even in const context */
    }
    
    int get_value() const {
        return regular_member;
    }
};
#endif

/* For DW_AT_is_optional and DW_AT_prototyped */
/* Function prototype - should trigger DW_AT_prototyped */
int prototype_function(int a, double b, char c);

/* For optional parameters - using GNU extension */
#ifdef __GNUC__
int optional_param_function(int a, ...) __attribute__((sentinel));
#endif

/* For DW_AT_lower_bound and DW_AT_ordering */
/* Using GNU extension for array with specified bounds */
#ifdef __GNUC__
typedef int bounded_array[1...10];  /* Lower bound of 1 */
#endif

/* Multi-dimensional array that might trigger ordering attribute */
int multi_dim_array[3][4][5];

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
#ifdef __GNUC__
typedef struct {
    unsigned char digits[10];
    signed char scale;
    unsigned char sign;
} __attribute__((packed)) decimal_type;
#endif

/* For DW_AT_string_length attributes */
typedef struct {
    unsigned int length;
    char data[];
} pascal_string;

/* Fixed-length string buffer */
typedef struct {
    char data[256];
} fixed_string;

/* For DW_AT_segment - address space qualified pointer */
#ifdef __GNUC__
typedef int __attribute__((address_space(1))) *segment_ptr;
#endif

/* For DW_AT_small - packed structure */
#ifdef __GNUC__
struct packed_struct {
    char a;
    int b __attribute__((packed));
    char c;
} __attribute__((packed));
#endif

/* For DW_AT_threads_scaled - OpenMP variable */
#ifdef _OPENMP
#include <omp.h>
#endif

/* Main function that uses all these types */
int main() {
    int result = 0;
    
#ifdef __cplusplus
    /* Use class with explicit constructor and mutable member */
    TestClass obj(10);
    obj.modify();
    result += obj.get_value();
#endif
    
    /* Call function with prototype */
    result += prototype_function(1, 2.0, 'a');
    
#ifdef __GNUC__
    /* Call function with optional parameters */
    result += optional_param_function(1, 2, 3, NULL);
#endif
    
    /* Use array with bounded dimensions */
#ifdef __GNUC__
    bounded_array arr;
    for (int i = 1; i <= 10; i++) {
        arr[i] = i;
        result += arr[i];
    }
#endif
    
    /* Use multi-dimensional array */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                multi_dim_array[i][j][k] = i + j + k;
                result += multi_dim_array[i][j][k];
            }
        }
    }
    
    /* Use decimal type */
#ifdef __GNUC__
    decimal_type dec;
    dec.scale = 2;
    dec.sign = 0;
    result += dec.scale;
#endif
    
    /* Use string types */
    pascal_string *ps = (pascal_string*)malloc(sizeof(pascal_string) + 100);
    if (ps) {
        ps->length = 100;
        result += ps->length;
        free(ps);
    }
    
    fixed_string fs;
    fs.data[0] = 'H';
    fs.data[1] = 'i';
    result += fs.data[0];
    
    /* Use segment pointer */
#ifdef __GNUC__
    int normal_var = 42;
    segment_ptr ptr = (segment_ptr)&normal_var;
    result += *ptr;
#endif
    
    /* Use packed structure */
#ifdef __GNUC__
    struct packed_struct ps2;
    ps2.a = 'A';
    ps2.b = 123;
    ps2.c = 'C';
    result += ps2.b;
#endif
    
    /* OpenMP section for threads_scaled */
#ifdef _OPENMP
    #pragma omp parallel
    {
        int thread_local_var = omp_get_thread_num();
        #pragma omp atomic
        result += thread_local_var;
    }
#endif
    
    /* Force location attribute by taking address */
    int location_test = 123;
    int *location_ptr = &location_test;
    result += *location_ptr;
    
    return result;
}

/* Function definitions */
int prototype_function(int a, double b, char c) {
    return a + (int)b + c;
}

#ifdef __GNUC__
int optional_param_function(int a, ...) {
    va_list args;
    va_start(args, a);
    int sum = a;
    int val;
    while ((val = va_arg(args, int)) != 0) {
        sum += val;
    }
    va_end(args);
    return sum;
}
#endif
