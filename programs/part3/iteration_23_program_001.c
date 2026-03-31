/* dwarf_attributes_test.c - Test program to trigger specific DWARF attributes */

/* For DW_AT_explicit and DW_AT_mutable - C++ specific */
#ifdef __cplusplus
class TestClass {
private:
    mutable int mutable_member;  /* DW_AT_mutable */
    int regular_member;
    
public:
    /* DW_AT_explicit for constructor */
    explicit TestClass(int x) : mutable_member(x), regular_member(x) {}
    
    void modify() const {
        mutable_member = 42;  /* Can modify even in const context */
    }
    
    int get_value() const {
        return mutable_member + regular_member;
    }
};
#endif

/* For DW_AT_is_optional and DW_AT_prototyped */
/* Function prototype - DW_AT_prototyped */
int prototype_function(int a, double b, char c);

/* Variadic function also has prototyped attribute */
int variadic_func(const char *fmt, ...);

/* For DW_AT_lower_bound and DW_AT_ordering - array attributes */
#ifdef __GNUC__
/* GNU extension for array range - may trigger lower_bound */
int gnu_array[10][1...4];  /* Range specified */

/* Fortran-style array ordering hint */
typedef int fortran_array_t[10][20]
    __attribute__((fortran_array));  /* May trigger ordering attribute */
#endif

/* For DW_AT_picture_string - decimal types */
#ifdef __GNUC__
/* COBOL/Ada-style decimal type using attribute */
typedef int decimal_type 
    __attribute__((decimal(9,2)));  /* 9 digits, 2 decimal places */
#endif

/* For DW_AT_string_length attributes */
struct pascal_string {
    int length;  /* DW_AT_string_length */
    char data[]; /* Flexible array member */
};

/* Fixed-length string buffer */
struct fixed_string {
    char data[256];
    /* Compiler might add string_length attributes */
} __attribute__((packed));

/* For DW_AT_segment - address space pointers */
#ifdef __GNUC__
/* Different address spaces for segment attribute */
typedef int __attribute__((address_space(1))) *ptr_addr1;
typedef int __attribute__((address_space(256))) *far_ptr;  /* Far pointer simulation */
#endif

/* For DW_AT_threads_scaled - OpenMP threading */
#ifdef _OPENMP
#include <omp.h>
#endif

/* For DW_AT_small - packed/small types */
struct small_struct {
    unsigned int a : 3;  /* Bit-field */
    unsigned int b : 5;
    unsigned int c : 8;
} __attribute__((packed));

/* Complex type with multiple attributes */
typedef struct {
    int lower_bound;  /* Simulate array descriptor */
    int upper_bound;
    int stride;
} array_descriptor;

/* Main test function */
int main() {
    int result = 0;
    
#ifdef __cplusplus
    /* Trigger DW_AT_explicit */
    TestClass obj(10);  /* Explicit constructor call */
    obj.modify();
    result += obj.get_value();
#endif
    
    /* Trigger DW_AT_location through various scopes */
    {
        int local_var = 42;
        result += local_var;
        
        /* Nested scope for location tracking */
        {
            int nested_var = local_var * 2;
            result += nested_var;
        }
    }
    
    /* Array operations for bounds/ordering */
    int normal_array[5][3] = {{1,2,3},{4,5,6}};
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            result += normal_array[i][j];
        }
    }
    
#ifdef __GNUC__
    /* Use GNU array extension if available */
    for (int i = 0; i < 10; i++) {
        for (int j = 1; j <= 4; j++) {
            gnu_array[i][j] = i * j;
            result += gnu_array[i][j];
        }
    }
#endif
    
    /* String operations */
    struct pascal_string *pstr = (struct pascal_string *)
        malloc(sizeof(struct pascal_string) + 100);
    if (pstr) {
        pstr->length = 100;
        strcpy(pstr->data, "Test string");
        result += pstr->length;
        free(pstr);
    }
    
    /* Packed/small structure */
    struct small_struct small = {1, 2, 3};
    result += small.a + small.b + small.c;
    
    /* Address space pointers */
#ifdef __GNUC__
    int normal_int = 100;
    ptr_addr1 addr_ptr = (ptr_addr1)&normal_int;
    result += *((int*)addr_ptr);  /* Cast back to access */
#endif
    
    /* OpenMP section for threads_scaled */
#ifdef _OPENMP
    #pragma omp parallel
    {
        int thread_local_var = omp_get_thread_num();
        #pragma omp atomic
        result += thread_local_var;
        
        /* Private variable in parallel region */
        int private_var = thread_local_var * 10;
        #pragma omp barrier
    }
#endif
    
    /* Function calls for prototyped attribute */
    result += prototype_function(1, 2.0, 'a');
    result += variadic_func("Test %d %f", 42, 3.14);
    
    /* Decimal type usage */
#ifdef __GNUC__
    decimal_type decimal_val = 12345;  /* Represents 123.45 */
    result += decimal_val / 100;
#endif
    
    printf("Final result: %d\n", result);
    return result;
}

/* Function definitions */
int prototype_function(int a, double b, char c) {
    return a + (int)b + c;
}

int variadic_func(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int sum = 0;
    
    /* Process some arguments */
    if (fmt[0]) sum += fmt[0];
    
    va_end(args);
    return sum;
}

/* Additional type in separate compilation unit context */
/* This would normally be in a separate file to force type emission */
typedef struct complex_type {
    int matrix[3][3];  /* Multi-dimensional for ordering */
    struct pascal_string *str;
    volatile int volatile_member;  /* Volatile for location tracking */
} complex_type_t;

/* Use the complex type */
complex_type_t global_complex = {
    .matrix = {{1,2,3},{4,5,6},{7,8,9}},
    .str = NULL,
    .volatile_member = 99
};
