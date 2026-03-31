/* main.c - Primary test driver */
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>

/* External declarations from other module */
extern void use_complex_types(void);
extern int compute_checksum(void);

/* Global variables for various attributes */
_Thread_local int thread_local_var = 42;  /* For DW_AT_threads_scaled */
int __attribute__((section(".mysection"))) section_var = 100;  /* For DW_AT_segment */

/* Structure with mutable member (C++) */
#ifdef __cplusplus
struct TestStruct {
    int normal;
    mutable int mutable_member;  /* For DW_AT_mutable */
    volatile int volatile_member;  /* For location variations */
    const int const_member;
    
    /* Bit-fields for string length attributes */
    struct {
        unsigned int length_bits : 7;  /* Could relate to DW_AT_string_length_bit_size */
        unsigned int length_bytes : 8;  /* Could relate to DW_AT_string_length_byte_size */
    } bitfield_struct;
};
#endif

/* Array with explicit bounds */
int bounded_array[5] = {1, 2, 3, 4, 5};  /* May generate DW_AT_lower_bound */

/* Function prototypes */
void prototyped_func(int a, char b);  /* For DW_AT_prototyped */
void variadic_func(int count, ...);   /* Variadic for prototype testing */
noreturn void noreturn_func(void);    /* noreturn attribute */

/* Explicit template specialization (C++ only) */
#ifdef __cplusplus
template<typename T>
class ExplicitTemplate {
public:
    explicit ExplicitTemplate(T val) {}  /* explicit constructor */
};

template<>
class ExplicitTemplate<double> {
public:
    explicit ExplicitTemplate(double val) {}  /* explicit specialization */
};
#endif

/* Picture string simulation (Ada-like) */
typedef struct {
    char picture[32];  /* Simulating DW_AT_picture_string */
    int scale;
} DecimalPicture;

/* Optional parameter simulation */
typedef struct {
    int is_present;
    union {
        int value;
        void *pointer;
    } data;
} OptionalParam;

int main(void) {
    /* Reference all globals to prevent optimization */
    int checksum = 0;
    
    checksum += thread_local_var;
    checksum += section_var;
    checksum += bounded_array[0];
    
    /* Use complex types if C++ */
    #ifdef __cplusplus
    TestStruct ts;
    ts.mutable_member = 10;
    ts.volatile_member = 20;
    checksum += ts.mutable_member;
    
    ExplicitTemplate<int> et1(5);
    ExplicitTemplate<double> et2(3.14);
    #endif
    
    /* Call functions */
    prototyped_func(1, 'A');
    variadic_func(3, 1, 2, 3);
    
    /* Use optional parameter type */
    OptionalParam opt = {1, {.value = 42}};
    checksum += opt.data.value;
    
    /* Use picture string type */
    DecimalPicture dp = {"ZZZ,ZZZ,ZZ9.99", 2};
    checksum += dp.scale;
    
    /* Call external functions */
    use_complex_types();
    checksum += compute_checksum();
    
    /* Prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* Function definitions */
void prototyped_func(int a, char b) {
    /* Empty but prototyped */
    (void)a;
    (void)b;
}

void variadic_func(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        int val = va_arg(args, int);
        (void)val;
    }
    va_end(args);
}

noreturn void noreturn_func(void) {
    while(1) {}  /* Actually noreturn */
}
