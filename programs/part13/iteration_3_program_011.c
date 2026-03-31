/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct Level1 {
    /* Function pointer with nested parentheses */
    int (*func_ptr1)(void);
    
    /* Array of function pointers */
    void (*func_array[5])(int);
    
    /* Nested struct with bit-fields */
    struct {
        unsigned int flag1:1;
        unsigned int flag2:3;
        unsigned int flag3:4;
    } bits;
};

/* 2. Deeply nested structure combining all bracket types */
struct MasterType {
    /* Multi-dimensional array */
    int matrix[3][4][5];
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(int))[10];
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned char a:2;
            unsigned char b:3;
            unsigned char c:3;
        } parts;
        unsigned char whole;
    } byte_union;
    
    /* Flexible array member */
    int flexible_array[];
};

/* 3. Function pointer declarations with varied signatures */
/* Pointer to function taking function pointer as parameter */
typedef void (*SignalHandler)(int);
typedef SignalHandler (*SignalFunc)(int, SignalHandler);

/* Pointer to function returning pointer to function */
int (*(*nested_func_ptr)(void))(int);

/* Function pointer with array parameter */
void (*array_processor)(int arr[10], char *str);

/* 4. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))(void)

/* Use the macros to create complex types */
PTR_FUNC(int) *macro_func_ptr;
ARRAY_TYPE(ARRAY_TYPE(int, 5), 3) macro_array;
NESTED_PTR(void) complex_nested_ptr;

/* 5. Union with deeply nested structures */
union SuperUnion {
    struct {
        /* Array of pointers to functions with different signatures */
        int (*func_ptrs[3])(int);
        
        /* Nested anonymous struct */
        struct {
            long (*calculator)(double, double);
            short data[7];
        } calc_data;
        
        /* Bit-field struct */
        struct {
            unsigned int mode:4;
            unsigned int count:12;
            unsigned int :0;  /* Force alignment */
            unsigned int flags:8;
        } control;
    } s;
    
    /* Alternative view as raw bytes */
    unsigned char raw[256];
    
    /* Pointer to another complex type */
    struct MasterType *master_ptr;
};

/* 6. GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    char buffer[64] __attribute__((aligned(32)));
} __attribute__((deprecated));

/* Function with attributes */
void __attribute__((noreturn, format(printf, 1, 2)))
attributed_function(const char *fmt, ...);

/* 7. Single declaration combining all bracket types (the ultimate test) */
struct UltimateType {
    /* Complex function pointer declaration: 
     * Pointer to function taking:
     *   - integer
     *   - pointer to function taking char array and returning int
     * and returning pointer to function taking int and returning void
     */
    void (*(*ultimate_func)(int, int (*)(char[10])))(int);
    
    /* Array of the above function pointers */
    void (*(*func_array[2])(int, int (*)(char[10])))(int);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:5;
            unsigned int b:5;
            unsigned int c:5;
            unsigned int d:5;
            unsigned int e:12;
        } fields;
        unsigned long long value;
    } data;
    
    /* Multi-dimensional array with pointer elements */
    int *ptr_array[3][4];
    
    /* Flexible array member of structs */
    struct {
        int x;
        double y;
    } flex_structs[];
} __attribute__((aligned(64)));

/* 8. Typedef chain with increasing complexity */
typedef int basic_t;
typedef basic_t *ptr_basic_t;
typedef ptr_basic_t (*func_returning_ptr_t)(void);
typedef func_returning_ptr_t array_of_funcs_t[5];
typedef struct {
    array_of_funcs_t funcs;
    int (*more_funcs[2][3])(int, char *);
} nested_typedef_struct_t;

/* 9. More edge cases */
/* Empty struct */
struct EmptyStruct {};

/* Struct with only bit-fields */
struct OnlyBitfields {
    unsigned int:16;
    unsigned int field1:8;
    unsigned int:0;
    unsigned int field2:8;
};

/* Struct with zero-length array */
struct WithZeroArray {
    int count;
    int data[0];
};

/* 10. Forward declarations that might be processed */
struct ForwardDecl;
union ForwardUnion;
enum ForwardEnum;

/* Incomplete array types */
extern int incomplete_array[];
extern struct ForwardDecl *ptr_array[];

/* 11. Enum with complex expressions (parentheses in initializers) */
enum ComplexEnum {
    ENUM_A = (1 << 0),
    ENUM_B = (1 << 1) | (1 << 2),
    ENUM_C = (ENUM_A | ENUM_B) & ~(1 << 3)
};

/* 12. Variable declarations using all the complex types */
extern struct UltimateType global_ultimate;
extern union SuperUnion global_union_array[10];
extern const volatile struct AttributedStruct *cv_attributed_ptr;

/* 13. Function prototypes with complex parameters */
int process_ultimate(struct UltimateType *ut,
                     void (*(*callback)(int))(int),
                     int matrix[][10]);

void handle_nested_arrays(int (*array_ptrs[])[5],
                          char *(*(*string_funcs[3])(void))[10]);

/* 14. One more deeply nested example */
struct {
    struct {
        union {
            int (*func1)(int (*)(char), double);
            struct {
                short s;
                long l;
            } data;
        } u;
        int arr[2][2];
    } inner;
    void (*finalizer)(struct UltimateType **, int);
} outermost;

#endif /* TEST_GENGTYPE_COVERAGE_H */
