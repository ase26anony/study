/* test_dwarf_attributes.c */
/* Compile with: g++ -O0 -g3 -std=c++11 -c test_dwarf_attributes.c -o test.o */

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

/* ============================================
   C constructs for string length attributes
   ============================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* Struct with counted string for DW_AT_string_length attributes */
struct string_desc {
    int length;
    /* Using GNU attribute to specify counted array */
    char data[] __attribute__((count(length)));
};

/* Alternative with pointer and bounds attribute */
struct bounded_string {
    int len;
    char *str __attribute__((bnd_variable_size(len)));
};

/* For DW_AT_ordering - multidimensional array */
typedef int matrix_t[3][4][5];

/* For DW_AT_small - bitfields and packed struct */
struct small_struct {
    unsigned int flag : 1;
    unsigned int value : 7;
    char c;
} __attribute__((packed));

/* For DW_AT_segment - use pointer with segment specifier (GCC extension) */
#ifdef __i386__
struct far_ptr {
    char __far *ptr;  /* Far pointer for segmented architectures */
};
#endif

/* For DW_AT_threads_scaled - thread-local storage */
__thread int thread_local_var;

#ifdef __cplusplus
}
#endif

/* ============================================
   C++ constructs for C++-specific attributes
   ============================================ */
#ifdef __cplusplus

/* For DW_AT_explicit - class with explicit constructor */
class ExplicitClass {
private:
    int value;
public:
    explicit ExplicitClass(int v) : value(v) {}
    int getValue() const { return value; }
};

/* For DW_AT_mutable - class with mutable member */
class MutableClass {
private:
    int regular;
    mutable int counter;  /* mutable member */
public:
    MutableClass() : regular(0), counter(0) {}
    void increment() const { counter++; }  /* Can modify mutable in const method */
    int getCounter() const { return counter; }
};

/* For DW_AT_is_optional - template with optional parameter */
template<typename T, typename U = void>
class OptionalTemplate {
    T value;
public:
    OptionalTemplate(T v) : value(v) {}
};

/* For DW_AT_prototyped - ensure prototypes are used */
void prototyped_function(int x, double y);  /* Declaration/prototype */

void prototyped_function(int x, double y) {  /* Definition */
    volatile int result = x + (int)y;
    (void)result;
}

/* For DW_AT_picture_string - use string literal with special format */
/* This might generate picture string attributes for certain types */
const char *picture_string = "999.999";

/* For DW_AT_lower_bound - array with specified bounds (GCC extension) */
int bounded_array[10] __attribute__((bounds(0, 9)));

/* For DW_AT_location - complex object with location lists */
class LocationTest {
    static int static_member;
    int instance_member;
public:
    LocationTest() : instance_member(0) {
        static_member++;
    }
    ~LocationTest() {
        static_member--;
    }
    int getValue() const { 
        /* Complex expression that might need location list */
        return instance_member + static_member;
    }
};

int LocationTest::static_member = 0;

#endif /* __cplusplus */

/* ============================================
   Main function to use all constructs
   ============================================ */
int main() {
    volatile int dummy = 0;
    
    /* Use string descriptor struct (C) */
    struct string_desc *desc = 0;
    dummy += (int)(long)desc;  /* Prevent optimization */
    
    /* Use bounded string */
    struct bounded_string bs = {0, 0};
    dummy += bs.len;
    
    /* Use multidimensional array for ordering */
    volatile matrix_t matrix;
    dummy += matrix[0][0][0];
    
    /* Use small struct with bitfields */
    struct small_struct small;
    small.flag = 1;
    small.value = 42;
    small.c = 'A';
    dummy += small.value;
    
    /* Use thread-local variable */
    thread_local_var = 123;
    dummy += thread_local_var;
    
#ifdef __cplusplus
    /* Use explicit constructor class */
    volatile ExplicitClass explicit_obj(42);
    dummy += explicit_obj.getValue();
    
    /* Use mutable class */
    volatile MutableClass mutable_obj;
    mutable_obj.increment();
    dummy += mutable_obj.getCounter();
    
    /* Use optional template */
    OptionalTemplate<int> opt_int(10);
    OptionalTemplate<int, void> opt_int2(20);
    dummy += sizeof(opt_int);
    
    /* Call prototyped function */
    prototyped_function(1, 2.0);
    
    /* Use picture string */
    dummy += (int)picture_string[0];
    
    /* Use bounded array */
    bounded_array[0] = 1;
    dummy += bounded_array[0];
    
    /* Use location test class */
    LocationTest loc_test;
    dummy += loc_test.getValue();
#endif
    
    /* Print dummy to prevent dead code elimination */
    printf("Dummy value: %d\n", dummy);
    
    return 0;
}
