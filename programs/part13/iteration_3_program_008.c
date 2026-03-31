/* test_gengtype_coverage.h - Complex type definitions for gengtype parser coverage */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(void);
    
    /* Pointer to function returning pointer to array */
    char (*(*complex_func)(int))[10];
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int flag3:4;
        } bits;
        unsigned char bytes[2];
    } nested_union;
    
    /* Multi-dimensional array */
    double matrix[4][5][6];
    
    /* Anonymous struct with flexible array member */
    struct {
        int count;
        int data[];
    } flex_struct;
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function taking function pointer as parameter */
typedef void (*SignalHandler)(int);
typedef SignalHandler (*SignalFunc)(int, SignalHandler);

/* Triple-nested function pointer */
typedef int (*(*(*TripleFuncPtr)(void))(int))(char);

/* Function returning pointer to function returning pointer to array */
char (*(*(*ultra_complex)(int x, double y))(void))[20];

/* 3. Multi-dimensional Arrays and Complex Declarations */
/* Array of pointers to arrays */
int (*array_of_pointers[5])[10];

/* Pointer to 3D array */
int (*ptr_to_3d_array)[7][8][9];

/* 4. Nested Anonymous Structs/Unions with Bit-fields */
struct BitFieldContainer {
    struct {
        unsigned int a:2;
        unsigned int b:4;
        unsigned int c:6;
        unsigned int d:8;
    } field_group1;
    
    union {
        struct {
            unsigned int x:1;
            unsigned int y:1;
            unsigned int z:1;
        } flags;
        unsigned int all_flags:3;
    } field_group2;
    
    /* Mixed bit-fields and normal members */
    unsigned char normal_member;
    unsigned int more_bits:5;
    unsigned int :3; /* Unnamed bit-field */
    unsigned int last_bits:7;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_TO_FUNC(T) T (*)(T)
#define ARRAY_OF_PTRS(T, N) T *[N]
#define FUNC_RETURNING_PTR(T) T *(*)(void)

/* Using the macros to create complex types */
PTR_TO_FUNC(int) func_ptr_macro;
ARRAY_OF_PTRS(char, 10) string_array;
FUNC_RETURNING_PTR(double) get_double_ptr;

/* More complex macro combination */
#define COMPLEX_MACRO(T) T (*(*[2])(T (*)(T)))(T)
COMPLEX_MACRO(int) very_complex_decl;

/* 6. Attribute Syntax with Parentheses */
/* Struct with alignment attribute */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int a;
    double b;
    char c;
} __attribute__((deprecated));

/* Function with attributes */
void __attribute__((noreturn, format(printf, 1, 2))) 
attributed_function(const char *fmt, ...);

/* Variable with section attribute */
int __attribute__((section(".data"), used)) 
global_var __attribute__((aligned(8)));

/* Type attribute */
typedef int __attribute__((may_alias)) aliasing_int;

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Function pointer with complex signature */
    void (*(*signal_handler)(int sig, void (*callback)(int)))(int);
    
    /* Array of function pointers taking function pointers */
    int (*(*func_ptr_array[3])(int (*)(char[10])))(double);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:2;
            unsigned int b:2[4]; /* Bit-field array (GCC extension) */
        } s;
        unsigned long long value;
    } data;
    
    /* Multi-dimensional pointer array */
    int *(*(*ptr_matrix[2][3]))[4][5];
    
    /* Flexible array member of function pointers */
    void (*flex_funcs[])(int, char);
};

/* Additional complex combinations */

/* Typedef chain with all bracket types */
typedef struct {
    int (*member_func)(struct { int x; int y; } point);
    union {
        char *(*get_name)(void);
        int (*get_id)(int (*verify)(int));
    } accessors;
} ComplexTypedef;

/* Nested type definitions */
typedef union OuterUnion {
    struct InnerStruct {
        int (*(*func_ptr)(void))[5];
        struct {
            unsigned int bits:4;
            unsigned int :4; /* Padding */
        } packed;
    } inner;
    
    struct {
        char (*string_array[10])(int, char **);
        void (*void_func)(void);
    } func_group;
} OuterUnion;

/* Pointer to array of pointers to functions returning pointers */
char *(*(*(*insanity_level)[10])(int))[20];

/* Final test: Everything combined */
struct __attribute__((packed)) FinalTest {
    /* Macro expanded type */
    COMPLEX_MACRO(void*) macro_type;
    
    /* Attributed member */
    int __attribute__((aligned(32))) aligned_member;
    
    /* Complex nested declaration */
    struct {
        int (*(*nested[2])(union { int a; float b; } param))(char[]);
        unsigned int flags:8 __attribute__((packed));
    } ultimate_nested;
    
    /* End with flexible array member containing function pointers */
    void (*(*end_array[])(int (*)(int[][10])))(char*);
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
