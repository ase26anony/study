/* test_gengtype_coverage.h - Complex type definitions for gengtype parser coverage */
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
int (*(*complex_func_ptr)(double))(char);

/* Function pointer with nested parameter containing array */
void (*signal_handler)(int sig, void (*handler)(int, char[10]));

/* Triple nested function pointer */
char *(*(**nested_fp)(int (*)(float)))(double);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4][5];  /* 4D array */
    char strings[5][100];
    int flexible_array[];  /* Flexible array member */
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    unsigned int flags : 4;
    signed int value : 16;
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned char a : 2;
            unsigned char b : 3;
            unsigned char c : 3;
        } bits;
        unsigned char byte;
    } data;
    
    /* Nested anonymous struct */
    struct {
        long x : 8;
        long y : 8;
        long z : 16;
    } coordinates;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))(void)

/* Using macros to generate complex types */
PTR_FUNC(int) *macro_func_ptr;
ARRAY_TYPE(ARRAY_TYPE(int, 5), 3) macro_array;
NESTED_PTR(char) nested_macro_ptr;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data[4];
} __attribute__((deprecated));

int variable __attribute__((used, section(".special")));

void __attribute__((noreturn)) 
__attribute__((format(printf, 1, 2)))
attributed_function(const char *fmt, ...);

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Function returning pointer to array of function pointers */
    int (*(*get_handlers(void))[5])(int, char);
    
    /* Complex signal handler declaration */
    void (*signal(int sig, void (*func)(int)))(int);
    
    /* Array of function pointers with complex parameter */
    int (*pfa[2])(int (*)(char[10]), float (*)[3]);
    
    /* Union with bit-fields and anonymous struct */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 2;
            unsigned int c : 29;
        } bits;
        unsigned int full;
    } flags;
    
    /* Multi-dimensional array */
    double coords[2][3][4];
    
    /* Flexible array member of structs */
    struct {
        int id;
        char name[20];
    } items[];
} __attribute__((aligned(32)));

/* Additional complex combinations */

/* Typedef with all bracket types */
typedef union {
    struct {
        int (*compare)(const void *, const void *);
        void (*free)(void *);
    } ops;
    void *data[2];
} GenericContainer;

/* Function with complex return type and parameters */
GenericContainer *(*factory_create(
    int count,
    void (**initializers)(GenericContainer *)
))(
    int (*filter)(const char **),
    char buffer[][100]
);

/* Nested type definitions triggering multiple consume_balanced calls */
struct Level1 {
    struct Level2 {
        struct Level3 {
            int (*level3_func[2])(
                struct Level4 {
                    union {
                        int x;
                        long y;
                    } data;
                    int arr[2][2];
                } *param
            );
            int matrix[2][2][2];
        } level3;
        void (*level2_callback)(
            int,
            char (*)[10],
            void (*)(struct Level3*)
        );
    } level2;
};

/* Edge case: empty brackets */
struct EmptyBrackets {
    int empty_array[0];
    void (*empty_func_ptr)(void);
    struct {} empty_struct;
    union {} empty_union;
};

/* Pointer to array of pointers to functions returning pointers */
int *(*(*pointer_soup[5])(float))[10];

/* __attribute__ with deeply nested parentheses */
int __attribute__(( 
    aligned(
        sizeof(
            struct {
                int x;
                double y;
            }
        )
    )
)) deeply_aligned_var;

/* Macro generating complex nested brackets */
#define CREATE_NESTED(N) \
    struct Nested##N { \
        int (*func##N)(int arr[N][N]); \
        struct { \
            char data[N]; \
        } inner; \
    }

CREATE_NESTED(3);
CREATE_NESTED(5);
CREATE_NESTED(10);

/* Final catch-all type using every possible bracket pattern */
struct FinalTest {
    /* 1. Parentheses in function pointers */
    int (*(*(*func1)(int (*(*)(char[10]))(float)))(double))(long);
    
    /* 2. Brackets in arrays */
    void *ptr_array[2][3][4];
    
    /* 3. Braces in nested structs/unions */
    union {
        struct {
            unsigned int a : 1, b : 1, c : 30;
        } bits;
        struct {
            int x[2];
            struct {
                char a;
                char b;
            } chars;
        } data;
    } variant;
    
    /* 4. All combined */
    struct {
        int (*methods[2])(
            union {
                int i;
                float f;
            } param,
            int matrix[2][2]
        );
    } obj;
    
    /* 5. With attributes */
    int special __attribute__(( 
        aligned(8),
        deprecated("use new_field instead")
    ));
    
    /* 6. Flexible array of complex type */
    struct {
        int id;
        char *(*get_name)(void);
    } entries[];
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
