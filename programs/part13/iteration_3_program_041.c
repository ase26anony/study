/* gengtype_test.h - Complex type definitions to test consume_balanced() */
#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* 1. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T[n]
#define NESTED_PTR(T) T (*(*)(void))[]
#define ATTR_ALIGNED(n) __attribute__((aligned(n)))

/* 2. Function pointer declarations with varied signatures */
/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*complex_func_ptr)(double))(char);

/* Function pointer with nested parentheses in parameters */
typedef void (*signal_handler)(int sig, void (*cleanup)(void*));

/* Pointer to function taking array of function pointers */
typedef int (*(*dispatcher)(void (*handlers[])(int)))(void);

/* 3. Multi-dimensional arrays and flexible array members */
struct array_container {
    int multi_dim[3][4][5];
    char* string_array[10];
    long double matrix[2][ATTR_ALIGNED(32) 3];
    int flexible_array[];
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct bitfield_mess {
    unsigned int flags : 4;
    signed int value : 12;
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned char a : 2;
            unsigned char b : 3;
            unsigned char c : 3;
        } bits;
        unsigned char byte;
    } ATTR_ALIGNED(2);
    
    /* Nested anonymous struct */
    struct {
        long : 16;  /* Unnamed bit-field */
        long field1 : 8;
        long field2 : 8;
        struct {
            unsigned int nested_bit : 1;
        } deeper;
    };
};

/* 5. Complex struct combining all bracket types */
struct ultimate_type {
    /* Function pointer member with complex signature */
    void (*(*signal)(int sig, void (*handler)(int)))(int);
    
    /* Array of function pointers */
    int (*callbacks[5])(int (*)(char[10]), void*);
    
    /* Pointer to array of pointers to functions */
    char (*(*(*string_proc)[10])(int))[];
    
    /* Nested union with bit-fields and array */
    union {
        struct {
            int x : 8;
            int y : 8;
            int z[3];
        } point;
        long packed;
    } data ATTR_ALIGNED(16);
    
    /* Multi-dimensional array with attribute */
    volatile int matrix[2][3] ATTR_ALIGNED(64);
    
    /* Flexible array member of function pointers */
    void (*flex_funcs[])(struct ultimate_type*);
};

/* 6. Using macros to create complex types */
typedef PTR_FUNC(int) int_func_ptr_t;
typedef ARRAY_DECL(PTR_FUNC(double), 5) func_ptr_array_t;

/* 7. Even more nesting */
struct recursive_nesting {
    /* Function returning pointer to array of pointers to functions */
    int (*(*(*get_handler)(void))[5])(int, ...);
    
    /* Struct with anonymous union containing anonymous struct */
    union {
        struct {
            int (*compare)(const void*, const void*);
            void (*swap)(void*, void*, size_t);
        } ops;
        void* vtable[2];
    } methods;
    
    /* Array of structs containing arrays */
    struct {
        int coords[3][3];
        float weights[];
    } elements[10];
};

/* 8. Type with GCC attributes in multiple places */
struct ATTR_ALIGNED(32) attributed_type {
    int data ATTR_ALIGNED(8);
    void (* ATTR_ALIGNED(16) operation)(struct attributed_type* ATTR_ALIGNED(32));
    
    /* __attribute__ with double parentheses */
    char buffer[100] __attribute__((aligned(16), packed));
    
    /* Nested attribute usage */
    union {
        long raw;
        struct {
            unsigned int : 24;
            unsigned int value : 8;
        } __attribute__((packed));
    } __attribute__((aligned(8))) payload;
};

/* 9. Complex typedef with all brackets */
typedef struct {
    /* Pointer to function taking pointer to array returning pointer to function */
    void (*(*(*lookup)(int (*array)[10]))(int))(void);
    
    /* Multi-dimensional pointer array */
    int *(*(*ptr_matrix[2])[3])[4];
    
    /* Anonymous struct with bit-fields and function pointer */
    struct {
        unsigned int flag : 1;
        void (*toggle)(void);
    } controller;
    
    /* Flexible array of pointers to functions with complex signatures */
    char (*(*flex_array[])(int (*)(char), void*))[];
} mega_type_t;

/* 10. Edge case: empty brackets */
struct empty_brackets {
    int empty_array[0];
    struct {} empty_struct;
    union {} empty_union;
    void (*empty_func_ptr)(void);
};

/* 11. Declaration combining everything in one line (for maximum parser stress) */
struct one_liner { void (*(*func_array[2])(int (*(*)(char[10]))(double), union { int x; struct { short a; short b; } s; } u))(int); int matrix[3][4]; struct { unsigned bit:1; } flags; } ATTR_ALIGNED(64) one_liner_var;

/* 12. Additional complex declarations to ensure coverage */
/* Function prototype with nested parentheses */
extern int (*(*register_callback(void (*(*provider)(int))(char)))(int))(void);

/* Typedef with attributes */
typedef volatile struct {
    const int (*(*get)(void))[];
    void (*(*set)(int (*value)[]))();
} api_t ATTR_ALIGNED(128);

#endif /* GENGYPE_TEST_H */
