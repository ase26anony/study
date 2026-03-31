/* dwarf_attributes_test.c
 * Compile with: gcc -O0 -g3 -fdebug-types-section -fopenmp dwarf_attributes_test.c -o test
 * For C++ version: g++ -O0 -g3 -fdebug-types-section -fopenmp -std=c++11 dwarf_attributes_test.cpp -o test
 */

#ifdef __cplusplus
#include <iostream>
#else
#include <stdio.h>
#endif

/* ========== DW_AT_explicit and DW_AT_mutable (C++ only) ========== */
#ifdef __cplusplus
class TestClass {
private:
    mutable int mutable_member;  /* Should trigger DW_AT_mutable */
    int regular_member;
    
public:
    explicit TestClass(int x) : mutable_member(x), regular_member(x) {}  /* Should trigger DW_AT_explicit */
    
    void modify() const {
        mutable_member = 42;  /* Can modify even in const function */
    }
    
    int get_value() const {
        return mutable_member + regular_member;
    }
};
#endif

/* ========== DW_AT_is_optional and DW_AT_prototyped ========== */
/* Function prototypes - should trigger DW_AT_prototyped */
int function_with_args(int a, float b, char c);
void another_function(void);

/* Simulating optional parameter (language extension) */
#ifdef __GNUC__
int optional_param_func(int required, ...) 
    __attribute__((sentinel));  /* Sentinel indicates optional variadic args */
#endif

/* ========== DW_AT_lower_bound and DW_AT_ordering ========== */
/* Array with non-standard bounds using GNU extension */
#ifdef __GNUC__
typedef int array_with_bounds[10][1...4];  /* GNU extension for array bounds */
#endif

/* Multi-dimensional array that might trigger ordering attribute */
int multi_dim_array[3][4][5];

/* ========== DW_AT_picture_string (decimal type) ========== */
/* Simulating decimal type - using GNU decimal float extension */
#ifdef __GNUC__
typedef _Decimal32 decimal_type __attribute__((mode(SD)));  /* Decimal float type */
#endif

/* ========== DW_AT_string_length attributes ========== */
/* Pascal-style string structure */
struct pascal_string {
    int length;            /* Should trigger DW_AT_string_length */
    unsigned char data[];  /* Flexible array member */
};

/* Fixed-length string buffer */
struct fixed_string {
    char data[256];
#ifdef __GNUC__
} __attribute__((packed));  /* Packed might relate to DW_AT_small */
#else
};
#endif

/* ========== DW_AT_segment (address space) ========== */
/* Pointer with address space attribute */
#ifdef __GNUC__
typedef int __attribute__((address_space(1))) *ptr_in_address_space;
#endif

/* ========== DW_AT_threads_scaled (OpenMP) ========== */
/* Variable in OpenMP context */
int global_counter = 0;

/* ========== DW_AT_small (packed/bitfield) ========== */
/* Packed structure with bitfields */
struct packed_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int value : 14;
    unsigned char small_data;
#ifdef __GNUC__
} __attribute__((packed, aligned(1)));
#else
};
#endif

/* ========== Function implementations ========== */
int function_with_args(int a, float b, char c) {
    return a + (int)b + c;
}

void another_function(void) {
    /* Empty but needed for prototype */
}

#ifdef __GNUC__
int optional_param_func(int required, ...) {
    return required * 2;
}
#endif

/* ========== Main function ========== */
int main() {
    int result = 0;
    
#ifdef __cplusplus
    /* Test explicit constructor and mutable member */
    TestClass obj(10);  /* Explicit constructor */
    obj.modify();
    result += obj.get_value();
    std::cout << "C++ object value: " << obj.get_value() << std::endl;
#else
    printf("C version (no C++ features)\n");
#endif
    
    /* Test function with prototype */
    result += function_with_args(1, 2.5f, 'A');
    
#ifdef __GNUC__
    /* Test optional parameter function */
    result += optional_param_func(5, NULL);
    
    /* Test array with bounds */
    array_with_bounds arr;
    for (int i = 0; i < 10; i++) {
        for (int j = 1; j <= 4; j++) {
            arr[i][j] = i * j;
            result += arr[i][j];
        }
    }
#endif
    
    /* Test multi-dimensional array */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                multi_dim_array[i][j][k] = i + j + k;
                result += multi_dim_array[i][j][k];
            }
        }
    }
    
#ifdef __GNUC__
    /* Test decimal type */
    decimal_type dec_val = 123.456df;
    result += (int)dec_val;
#endif
    
    /* Test packed structure */
    struct packed_struct ps = {1, 0, 42, 99};
    result += ps.value + ps.small_data;
    
    /* Test address space pointer */
#ifdef __GNUC__
    int normal_var = 100;
    ptr_in_address_space asp = (ptr_in_address_space)&normal_var;
    result += *((int*)asp);  /* Cast back to access */
#endif
    
    /* OpenMP section for threads_scaled */
    #pragma omp parallel
    {
        int thread_local_var = omp_get_thread_num();  /* Might trigger threads_scaled */
        #pragma omp atomic
        global_counter += thread_local_var;
    }
    
    result += global_counter;
    
    /* Final output to prevent optimization */
#ifdef __cplusplus
    std::cout << "Final result: " << result << std::endl;
#else
    printf("Final result: %d\n", result);
#endif
    
    return result > 0 ? 0 : 1;
}
