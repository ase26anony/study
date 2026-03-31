/* main.c - Main driver program */
#include <stdio.h>
#include <stdarg.h>

/* Forward declarations from other modules */
extern void use_ada_types(void);
extern void use_fortran_types(void);

/* For DW_AT_explicit - C++ explicit constructors */
#ifdef __cplusplus
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    ExplicitClass(double x) : value(static_cast<int>(x)) {}  // implicit
private:
    int value;
};

template<typename T>
class TemplateClass {
public:
    explicit TemplateClass(T val) : data(val) {}
private:
    T data;
};

// Explicit template specialization
template<>
class TemplateClass<float> {
public:
    explicit TemplateClass(float val) : data(val) {}
private:
    float data;
};
#endif

/* For DW_AT_is_optional - Ada optional parameters */
/* Simulated via function pointers that can be NULL */
typedef void (*OptionalFunc)(int, ...);

/* For DW_AT_location - variables with specific storage */
volatile int volatile_var = 42;
const int const_array[] = {1, 2, 3, 4, 5};

/* For DW_AT_lower_bound - array with non-zero lower bound */
/* In C, we can use pointer arithmetic to simulate this */
int array_with_offset[10];
#define ARRAY_LOWER_BOUND 5
int *array_with_lower_bound = &array_with_offset[-ARRAY_LOWER_BOUND];

/* For DW_AT_mutable - C++ mutable members */
#ifdef __cplusplus
class ClassWithMutable {
public:
    ClassWithMutable() : x(0), y(0) {}
    void modify() const { y = 42; }  // Can modify mutable member in const function
private:
    int x;
    mutable int y;  // DW_AT_mutable should be generated for this
};
#endif

/* For DW_AT_ordering - enumeration with specific values */
enum OrderedEnum {
    FIRST = 1,
    SECOND = 2,
    THIRD = 3
};

/* For DW_AT_picture_string - Ada Decimal type simulation */
/* We'll use a struct with scaling factor */
typedef struct {
    long long value;
    int scale;
    int digits;
    int precision;
} DecimalType;

/* For DW_AT_prototyped - functions with prototypes */
void prototyped_func(int a, double b);  // Declaration with prototype
void variadic_func(int count, ...);     // Variadic function

/* For DW_AT_small - Ada 'Small attribute simulation */
typedef struct {
    int value;
    float scale_factor;
} SmallScaledType;

/* For DW_AT_segment - variables in specific segments */
#ifdef __GNUC__
__attribute__((section(".my_segment"))) int segment_var = 100;
__attribute__((section(".data_segment"))) long segment_array[4] = {10, 20, 30, 40};
#endif

/* For DW_AT_string_length - strings with explicit length */
typedef struct {
    char *data;
    size_t length;
} BoundedString;

/* For DW_AT_string_length_bit_size - bit-field strings */
struct BitString {
    unsigned int length : 4;    // 4-bit length field
    unsigned int data : 28;     // 28-bit data field
};

/* For DW_AT_string_length_byte_size - structure with byte size field */
struct ByteSizedString {
    unsigned char length;
    char data[255];
};

/* For DW_AT_threads_scaled - thread-local storage */
#ifdef __STDC_NO_THREADS__
_Thread_local int thread_var = 0;
#else
__thread int thread_var = 0;
#endif

#ifdef __cplusplus
thread_local int cpp_thread_var = 42;
#endif

/* Function definitions */
void prototyped_func(int a, double b) {
    printf("Prototyped: %d, %f\n", a, b);
}

void variadic_func(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        int val = va_arg(args, int);
        printf("Vararg %d: %d\n", i, val);
    }
    va_end(args);
}

/* Function with noreturn attribute */
#ifdef __GNUC__
__attribute__((noreturn)) void fatal_error(void) {
    fprintf(stderr, "Fatal error occurred\n");
    exit(1);
}
#endif

/* Complex structure with various attributes */
struct ComplexStruct {
    int id;
    volatile int status;           // DW_AT_location variations
    const char *name;              // DW_AT_string_length potential
    struct BitString bit_str;      // DW_AT_string_length_bit_size
    struct ByteSizedString byte_str; // DW_AT_string_length_byte_size
#ifdef __cplusplus
    mutable int cache;             // DW_AT_mutable
#endif
};

/* Union with discriminant (like Ada variant records) */
union VariantUnion {
    int int_val;
    double double_val;
    char *string_val;
    struct {
        int type;
        union {
            int i;
            double d;
            char *s;
        } data;
    } tagged;
};

/* Main function that references everything */
int main(void) {
    int checksum = 0;
    
    /* Reference volatile and const variables */
    checksum += volatile_var;
    checksum += const_array[0];
    
    /* Use array with simulated lower bound */
    for (int i = ARRAY_LOWER_BOUND; i < ARRAY_LOWER_BOUND + 5; i++) {
        array_with_lower_bound[i] = i;
        checksum += array_with_lower_bound[i];
    }
    
    /* Use ordered enum */
    enum OrderedEnum e = SECOND;
    checksum += e;
    
    /* Use decimal type */
    DecimalType dec = {123456, 2, 6, 2};
    checksum += dec.value % 1000;
    
    /* Call prototyped functions */
    prototyped_func(10, 3.14159);
    variadic_func(3, 1, 2, 3);
    
    /* Use segment variables */
#ifdef __GNUC__
    checksum += segment_var;
    checksum += segment_array[0];
#endif
    
    /* Use string types */
    BoundedString str = {"Hello", 5};
    checksum += str.length;
    
    struct BitString bit_str = {3, 0xABCD};
    checksum += bit_str.length;
    
    struct ByteSizedString byte_str = {5, "World"};
    checksum += byte_str.length;
    
    /* Use thread-local storage */
    thread_var = 100;
    checksum += thread_var;
    
#ifdef __cplusplus
    /* Use C++ features */
    ExplicitClass obj1(42);
    ExplicitClass obj2 = ExplicitClass(3.14);  // Explicit construction
    // ExplicitClass obj3 = 42;  // Would fail - constructor is explicit
    
    ClassWithMutable mutable_obj;
    mutable_obj.modify();  // Modifies mutable member in const function
    
    cpp_thread_var = 200;
    checksum += cpp_thread_var;
    
    TemplateClass<int> t1(10);
    TemplateClass<float> t2(3.14f);  // Explicit specialization
#endif
    
    /* Complex structure usage */
    struct ComplexStruct cs = {
        .id = 1,
        .status = 0,
        .name = "Test",
        .bit_str = {4, 0x1234},
        .byte_str = {4, "Data"}
    };
    checksum += cs.id;
    
    /* Union usage */
    union VariantUnion vu;
    vu.tagged.type = 1;
    vu.tagged.data.i = 42;
    checksum += vu.tagged.data.i;
    
    /* Try to call Ada/Fortran specific functions if available */
    /* These would normally be in separate files */
    use_ada_types();
    use_fortran_types();
    
    /* Print checksum to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    /* Ensure variables are used */
    return checksum == 0 ? 1 : 0;
}
