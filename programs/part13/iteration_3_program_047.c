/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. MACRO EXPANSIONS GENERATING BRACKETS */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T[n]
#define NESTED_PTR(T) T (*(*)(T (*)(T)))(T)
#define ATTR_WRAP(...) __attribute__((__VA_ARGS__))

/* 2. FUNCTION POINTER DECLARATIONS WITH VARIED SIGNATURES */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(int (*)(char[10]), void*);
typedef char* (*(*nested_func_ptr)(int (*)(double)))(float);

/* 3. COMPLEX NESTED TYPE DEFINITIONS */
struct OuterStruct {
    /* Function pointer with nested parentheses */
    void (*signal_handler)(int sig, void (*cleanup)(void*));
    
    /* Multi-dimensional arrays */
    int matrix[3][4][5];
    
    /* Array of function pointers */
    int (*func_array[5])(int, char**);
    
    /* Nested anonymous struct with bit-fields */
    struct {
        unsigned int flags : 4;
        unsigned int status : 2;
        signed int value : 8;
    } bitfield_container;
    
    /* Union with complex members */
    union {
        /* Pointer to function returning pointer to array */
        int (*(*union_func_ptr)(void))[10];
        
        /* Nested struct in union */
        struct {
            double coordinates[2][3];
            void (*transform)(double (*)[3]);
        } geometric_data;
        
        /* Flexible array member simulation */
        struct {
            size_t count;
            int data[];
        } dynamic_array;
    } variant_data;
    
    /* Attribute syntax with parentheses */
    int aligned_buffer[32] ATTR_WRAP(aligned(64));
};

/* 4. SINGLE DECLARATION WITH ALL BRACKET TYPES */
struct UltimateType {
    /* Combines all: () [] {} */
    void (*(*complex_member[2])(
        int (*param_func)(char[5][10]),
        struct { int x; double y; } nested_struct
    ))(int, ...) ATTR_WRAP(packed);
    
    /* Multi-level array with function pointers */
    int (*(*nested_array[3][2])(float))[4];
    
    /* Deeply nested parentheses */
    char* (*(*(*deep_func_ptr)(int (*(*)(double[2]))(float)))(long))[5];
    
    /* Anonymous union with bit-fields and array */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 3;
            unsigned int c : 4;
        } bits;
        
        unsigned char bytes[2];
        
        /* Function pointer inside anonymous union */
        void (*action)(void);
    } control;
    
    /* Flexible array member at end */
    struct UltimateType* recursive_links[];
};

/* 5. MORE COMPLEX TYPE DEFINITIONS */
typedef union {
    /* Array of pointers to functions with array parameters */
    int (*(*callback_table[10])(int[][3]))(void);
    
    /* Struct with nested attribute */
    struct ATTR_WRAP(packed) {
        short s;
        char c;
        long l ATTR_WRAP(aligned(8));
    } packed_data;
    
    /* Macro-expanded type */
    PTR_FUNC(double) math_func;
} PolyUnion;

/* 6. FUNCTION PROTOTYPES WITH COMPLEX PARAMETERS */
extern void register_callback(
    int (*(*get_processor)(void))(int),
    char* (*(*handlers[]))(int, ...)
) ATTR_WRAP(nonnull(1, 2));

extern struct UltimateType* create_type(
    int dimensions[][4],
    void (**callbacks)(int, struct OuterStruct*)
);

/* 7. VARIABLE DECLARATIONS USING COMPLEX TYPES */
static complex_callback global_cb ATTR_WRAP(used);
static int (*(*global_func_array[3])[2])(float) ATTR_WRAP(section(".data")) = {0};

/* 8. NESTED TYPEDEFS WITH ALL BRACKET TYPES */
typedef struct {
    /* Triple nested function pointer */
    int (*(*(*triple_ptr)(int (*)(int[5])))(int[][3]))[2];
    
    /* Array of structs containing arrays of function pointers */
    struct {
        void (*actions[5])(void);
        int (*computations[3])(double, double);
    } processor_units[2];
    
    /* Union with anonymous struct containing bit-fields */
    union {
        struct {
            unsigned int field1 : 5;
            unsigned int field2 : 11;
            unsigned int field3 : 16;
        };
        unsigned int raw;
    } packed_bits;
} DeeplyNestedType;

/* 9. MACRO-GENERATED COMPLEX DECLARATION */
#define CREATE_COMPLEX_TYPE(name, T) \
    struct name { \
        T (*processor)(T (*)(T[2]), T**); \
        union { \
            T data[10]; \
            struct { T x; T y[3]; } point; \
        } value; \
        T (*getter)(void) ATTR_WRAP(const); \
    }

CREATE_COMPLEX_TYPE(ComplexInt, int);
CREATE_COMPLEX_TYPE(ComplexDouble, double);

/* 10. FINAL COMPREHENSIVE STRUCTURE */
struct ComprehensiveTest {
    /* All bracket types in sequence */
    int (*func_ptr_with_array_param(int x[][5]))(char);
    
    /* Nested array declaration */
    struct OuterStruct* object_array[4][3];
    
    /* Anonymous struct with everything */
    struct {
        /* Function pointer returning pointer to array */
        int (*(*get_matrix)(int size))[][4];
        
        /* Union with bit-fields and function pointer */
        union {
            void (*void_func)(void);
            struct {
                unsigned int a : 2;
                unsigned int b : 6;
                unsigned int c : 8;
            } flags;
        } switcher;
        
        /* Multi-dimensional flexible array */
        int dynamic_matrix[][3][2];
    } anonymous_member;
    
    /* Attribute with multiple parentheses */
    int special_value ATTR_WRAP((aligned(32), packed, deprecated));
    
    /* Macro-expanded type with all brackets */
    NESTED_PTR(long) recursive_ptr;
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
