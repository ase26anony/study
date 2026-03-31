/* test_dwarf_attrs.c - Comprehensive test for DWARF attribute generation */
/* Compile with: gcc -g -O0 -dA -fopenmp -std=c++17 test_dwarf_attrs.c -o test_dwarf_attrs */

#include <optional>
#include <string>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
typedef char fixed_string[32];
fixed_string global_string = "test_string";

struct StringStruct {
    std::string cpp_string;  /* Should generate string length attributes */
    char array_string[64];
};

/* For DW_AT_picture_string - COBOL/Fortran PICTURE type simulation */
#ifdef __GNUC__
struct PictureType {
    char data[20];
} __attribute__((picture("9(5)V9(2)")));
#else
struct PictureType {
    char data[20];
};
#endif

/* For DW_AT_threads_scaled - OpenMP thread-related variables */
#ifdef _OPENMP
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)

int omp_thread_scaled_var __attribute__((omp_thread_scaled));
#endif

/* For DW_AT_explicit - C++ explicit constructor */
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    explicit operator bool() const { return value != 0; }
private:
    int value;
};

/* For DW_AT_is_optional - C++17 std::optional */
std::optional<int> global_optional;
std::optional<double> function_optional(std::optional<float> param) {
    return param.has_value() ? std::optional<double>(*param) : std::nullopt;
}

/* For DW_AT_mutable - C++ mutable member */
class MutableClass {
public:
    MutableClass() : normal(0), mutable_counter(0) {}
    void modify() const { mutable_counter++; }  /* Can modify mutable in const method */
private:
    int normal;
    mutable int mutable_counter;
};

/* For DW_AT_ordering - Column-major array (Fortran style) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_segment - Segment-specific pointers (x86 memory models) */
#ifdef __i386__
int __seg_fs *fs_segment_ptr;
int __seg_gs *gs_segment_ptr;
#endif

/* For DW_AT_prototyped - Fully prototyped function */
int fully_prototyped_func(int a, char b, double c);
int (*func_ptr)(int, char, double) = &fully_prototyped_func;

int fully_prototyped_func(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* For DW_AT_small - Packed small types */
struct SmallPacked {
    unsigned int flag : 1;
    unsigned int tiny : 3;
} __attribute__((packed));

/* For DW_AT_lower_bound - Array with non-zero lower bound simulation */
struct ArrayWithBounds {
    int data[10];
    int lower_bound;
    int upper_bound;
};

/* For DW_AT_location - Variables with complex locations */
register int reg_var asm("ebx");  /* Register variable */

/* Complex type with multiple attributes */
class ComplexType {
public:
    explicit ComplexType(int x) : value(x), mut(0) {}
    
    /* Prototyped member function */
    int method(int a, char b) const;
    
private:
    int value;
    mutable int mut;
    std::optional<std::string> opt_str;
};

int ComplexType::method(int a, char b) const {
    mut++;  /* Use mutable member */
    return value + a + b;
}

/* Thread-local storage */
thread_local int thread_specific_var = 100;

/* Volatile and const qualified */
volatile const int volatile_const_var = 0xDEADBEEF;

/* Restrict pointer */
void use_restrict(int* restrict ptr1, int* restrict ptr2) {
    *ptr1 += *ptr2;
}

/* Main function using all constructs */
int main() {
    /* String length attributes */
    fixed_string local_string = "local";
    StringStruct str_struct;
    str_struct.cpp_string = "Hello";
    
    /* Picture string */
    PictureType picture_var;
    
    /* Explicit constructor */
    ExplicitClass expl_obj(42);
    if (expl_obj) {
        /* Do nothing */
    }
    
    /* Optional */
    global_optional = 42;
    auto local_opt = function_optional(3.14f);
    
    /* Mutable */
    MutableClass mut_obj;
    mut_obj.modify();
    
    /* Column-major array */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Segment pointers */
#ifdef __i386__
    fs_segment_ptr = (int __seg_fs*)0x1000;
#endif
    
    /* Function pointer usage */
    int result = func_ptr(10, 'A', 3.14);
    
    /* Small packed */
    SmallPacked small = {1, 7};
    
    /* Array with bounds */
    ArrayWithBounds bounded_array;
    bounded_array.lower_bound = 1;
    bounded_array.upper_bound = 10;
    
    /* Complex type */
    ComplexType complex(100);
    complex.method(50, 'X');
    
    /* Thread operations */
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
        thread_specific_var = omp_get_thread_num() * 10;
    }
#endif
    
    /* Restrict pointers */
    int x = 10, y = 20;
    use_restrict(&x, &y);
    
    /* Register variable attempt */
    /* Note: Actual register assignment depends on compiler */
    
    return result + x + y + small.flag + thread_specific_var;
}

/* Additional prototypes for DW_AT_prototyped coverage */
void another_prototyped(int, float, char*);
int (*another_ptr)(void) = nullptr;
