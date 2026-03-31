/* Target DWARF attributes through various language constructs */
#include <stdio.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include "debug_types.h"

/* Global variables for DW_AT_location and DW_AT_threads_scaled */
volatile int global_volatile = 42;
_Thread_local int thread_local_var = 100;
__attribute__((section(".custom_segment"))) int segmented_var = 200;

/* For DW_AT_prototyped */
void prototyped_func(void);
int variadic_func(int count, ...);
noreturn void noreturn_func(void);

/* For DW_AT_mutable (C++) */
#ifdef __cplusplus
class MutableClass {
    mutable int mutable_member;
public:
    MutableClass() : mutable_member(0) {}
    void modify() const { mutable_member = 42; }  // Can modify mutable in const
};
#endif

/* For DW_AT_explicit (C++) */
#ifdef __cplusplus
struct ExplicitStruct {
    explicit ExplicitStruct(int) {}  // Explicit constructor
    template<typename T>
    explicit operator T() const { return T(); }  // Explicit conversion
};
#endif

/* For DW_AT_string_length attributes */
struct StringInfo {
    char data[100];
    unsigned int length : 7;  // Bit-field for string_length_bit_size
    unsigned int byte_len : 8; // Bit-field for string_length_byte_size
};

/* For DW_AT_lower_bound */
typedef int array_10[10];
typedef int array_dyn[];

/* For DW_AT_ordering */
enum OrderedEnum {
    FIRST,
    SECOND,
    THIRD
};

/* Function definitions */
void prototyped_func(void) {
    printf("Fully prototyped function\n");
}

int variadic_func(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

noreturn void noreturn_func(void) {
    printf("This function never returns\n");
    while(1);  // Infinite loop
}

/* Main function that references everything */
int main(void) {
    /* Reference thread-local variable */
    thread_local_var += 1;
    
    /* Reference segmented variable */
    segmented_var = 300;
    
    /* Use volatile variable */
    int volatile_copy = global_volatile;
    
    /* Create and use mutable object (C++) */
    #ifdef __cplusplus
    MutableClass mc;
    mc.modify();
    
    /* Use explicit constructor */
    ExplicitStruct es(42);
    ExplicitStruct es2 = static_cast<ExplicitStruct>(42);
    #endif
    
    /* Use string structure with bit-fields */
    struct StringInfo si = {"Hello", 5, 5};
    
    /* Use arrays with bounds */
    array_10 arr = {0,1,2,3,4,5,6,7,8,9};
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    
    /* Use ordered enum */
    enum OrderedEnum oe = SECOND;
    
    /* Call prototyped functions */
    prototyped_func();
    int var_sum = variadic_func(3, 1, 2, 3);
    
    /* Reference types from header */
    struct ComplexType ct;
    ct.base = 42;
    
    /* Calculate checksum to prevent optimization */
    unsigned long checksum = 0;
    checksum += (unsigned long)&thread_local_var;
    checksum += (unsigned long)&segmented_var;
    checksum += volatile_copy;
    checksum += sum;
    checksum += oe;
    checksum += var_sum;
    checksum += ct.base;
    #ifdef __cplusplus
    checksum += sizeof(mc);
    #endif
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
