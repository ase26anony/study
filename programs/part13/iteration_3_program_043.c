/* test_gengtype_coverage.h
 * Complex type definitions to exercise consume_balanced() parser
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Nested parentheses: function pointers */
    int (*func_ptr1)(void);
    
    /* Nested brackets: multi-dimensional arrays */
    int matrix[3][4][5];
    
    /* Nested braces: anonymous struct */
    struct {
        int x;
        int y;
    } point;
    
    /* Combination: array of function pointers */
    void (*callbacks[10])(int, char);
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_fp1)(int))(char);

/* Function pointer with nested parameter */
void (*signal_handler)(int sig, void (*cleanup)(void*));

/* Even more nested */
char (*(*(*nested_fp)(int (*)(float)))[5])(double);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4];
    char strings[5][256];
    
    /* Flexible array member */
    int data[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    unsigned int flags : 4;
    signed int value : 16;
    
    union {
        struct {
            unsigned char a : 2;
            unsigned char b : 3;
            unsigned char c : 3;
        } bits;
        unsigned char byte;
    } u;
    
    struct {
        long x : 8;
        long y : 8;
        long z : 16;
    } __attribute__((packed)) coord;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))[]

/* Use the macros */
PTR_FUNC(int) *func_ptr_macro;
ARRAY_TYPE(ARRAY_TYPE(int, 5), 3) array_macro;
NESTED_PTR(char) complex_macro;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int a;
    double b __attribute__((aligned(8)));
} __attribute__((deprecated));

int variable __attribute__((weak, used)) = 0;

void __attribute__((noreturn, format(printf, 1, 2))) 
attributed_func(const char *fmt, ...);

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Function returning pointer to function with array parameter */
    void (*(*func1)(int (*handler)(char[10])))(float);
    
    /* Array of function pointers with complex signatures */
    int (*(*pfa[2])(int (*)(char[10])))(double);
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 2;
            unsigned int c : 29;
        } bits;
        unsigned int full;
    } u __attribute__((packed));
    
    /* Multi-dimensional array */
    long triple_array[2][3][4];
    
    /* Flexible array member of structs */
    struct {
        int id;
        char name[20];
    } items[];
} __attribute__((aligned(32)));

/* Additional complex cases */

/* Typedef with nested parentheses */
typedef int (*Callback)(void (*)(int), char *argv[]);

/* Struct containing pointer to array of function pointers */
struct Container {
    Callback (*get_callbacks(void))[5];
    void (*(*signal_table[10])(int, void (*)(int)))(void);
};

/* Union with anonymous struct containing bit-fields */
union MixedUnion {
    struct {
        unsigned short a : 4;
        unsigned short b : 4;
        unsigned short c : 8;
    };
    struct {
        unsigned char x;
        unsigned char y;
    } bytes;
    unsigned int word;
};

/* Function prototype with __attribute__ and complex return type */
struct OuterStruct *(*factory_func(int count, 
                                   void (*init)(struct OuterStruct*)))
                                   __attribute__((malloc))
                                   (int size, 
                                    char name[]);

/* Nested array of pointers to functions returning pointers to arrays */
int (*(*(*nested_array[2][3])(float))[4])(double);

/* Final test: everything combined in a typedef */
typedef struct {
    union {
        int (*(*fp_arr[5])(int[3]))(void);
        struct {
            unsigned int : 16;
            unsigned int field1 : 8;
            unsigned int field2 : 8;
        };
    } data;
    
    char (*(*string_processor)(const char *input, 
                               int (*filter)(char)))[256];
                               
    void (*cleanup[10])(void *ptr, 
                        int (*validator)(const void*),
                        char error_msg[][256]);
} UltimateType __attribute__((aligned(64)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
