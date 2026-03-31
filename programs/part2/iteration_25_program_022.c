/* test_suite.h - Complex type definitions to exercise gengtype parser */

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

/* ========== 1. Deeply Nested Struct/Union Definitions ========== */

/* Struct with anonymous nested struct and union */
struct OuterStruct {
    struct {
        int a;
        union {
            char b;
            long c;
            struct {
                short d;
                unsigned e;
            } inner;
        } u;
    } nested;
    
    /* Array with multiple dimensions */
    int arr[5][7][3];
    
    /* Pointer to nested anonymous struct */
    struct {
        float x;
        double y;
    } *ptr_to_anon;
};

/* Union with bit-fields and nested structs */
union ComplexUnion {
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        unsigned int : 4;  /* Padding */
        unsigned int value : 24;
    } bits;
    
    struct {
        char data[16];
        struct {
            int counter;
            union {
                short s;
                long l;
            } val;
        } meta;
    } buffer;
    
    long long as_int64;
};

/* ========== 2. Function Pointer Chains ========== */

/* Simple function pointer */
typedef int (*SimpleFunc)(int, char);

/* Function returning pointer to array */
int (*func_returning_array_ptr(void))[10];

/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(double))(float);

/* Even more complex: function taking function pointer returning function pointer */
void (*(*accepts_func_ptr(int (*(*arg)(void))[5]))(void))(char);

/* Array of function pointers */
void (*func_array[5])(int, char);

/* Function returning pointer to array of function pointers */
int (*(*get_callback_table(void))[10])(void);

/* ========== 3. Mixed Delimiter Constructs ========== */

/* Declaration combining all three delimiters */
struct MixedDelimiters {
    /* Array of pointers to functions */
    int (*(*callbacks[3])(void))[5];
    
    /* Nested initializer in declaration */
    int matrix[2][3];
    
    /* Function pointer with complex signature */
    void (*(*signal_handler)(int, void*))(int);
};

/* Initializer with designators and nested braces */
int initialized_array[3][2][4] = {
    [0] = { {1, 2, 3, 4}, {5, 6, 7, 8} },
    { {9, 10, 11, 12}, {13, 14, 15, 16} },
    [2] = { {17, 18}, {19, 20} }
};

/* ========== 4. GCC Attributes ========== */

struct __attribute__((aligned(32), packed)) AlignedStruct {
    char data;
    int value __attribute__((aligned(16)));
    short flags __attribute__((bitwise));
};

typedef int __attribute__((vector_size(16))) v4si;

/* Function with attributes */
void important_func(void) __attribute__((noreturn, noinline, 
    __format__(__printf__, 1, 2)));

/* ========== 5. Macro-based Complex Types ========== */

#define PTR_TO_FUNC(ret, args) ret (*)args
#define ARRAY_OF_PTRS(type, size) type *[size]
#define NESTED_PTR_ARR(type) type (*(*)[10])[5]

/* Use the macros to create complex types */
PTR_TO_FUNC(int, (int, char)) global_func_ptr;

ARRAY_OF_PTRS(struct OuterStruct, 7) struct_ptr_array;

/* Really complex macro expansion */
NESTED_PTR_ARR(float) ultra_complex_var;

#endif /* TEST_SUITE_H */
