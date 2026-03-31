/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * ( (2) + (1) ) )
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define NESTED_PARENS(x) (((x) + 1) * 2)

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* More complex parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Even more nested parentheses in skip expression */
struct s3 GTY((skip(("NODE") ("next") ("prev")))) {
    struct s3 *next;
    struct s3 *prev;
    char *data;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s4 GTY((user)) {
    int arr GTY((length("N")))[ (2 * 3) ];
    int N;
};

/* Pointer to array with nested brackets */
typedef int (*array_ptr GTY((tag("ARRAY_PTR"))))[ (sizeof(int) > 4) ? 10 : 20 ];

/* Multi-dimensional array with complex expressions */
struct s5 GTY(()) {
    int matrix GTY((length("rows * cols")))[ (2 + 3) ][ (4 * 2) ];
    int rows;
    int cols;
};

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct s6 GTY((user)) {
    struct inner GTY((tag("INNER"))) {
        int x;
        int y;
    } nested;
    int value;
};

/* Union with array containing brace-enclosed initializer in macro expansion */
union u1 GTY((desc("(%1.type == UNION_TYPE_A) ? UNION_A : UNION_B"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        int a;
        int b;
    } s;
};

/* Function pointer with complex argument list containing brackets */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ]),
    void * GTY((skip)) 
);

/* Nested structure with multiple GTY markers */
struct outer GTY((user)) {
    struct middle GTY((chain_next("m_next"))) {
        struct middle *m_next;
        struct inner GTY((tag("INNER_TAG"))) {
            int values[ (1 << 3) ];
            char *name;
        } inner_struct;
    } mid;
    int count;
};

/* Template-like macro usage with parentheses */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name GTY((length(#size))) { \
        unsigned char data[(size)]; \
    }

/* Instantiate the macro with complex expression */
DEFINE_GTY_STRUCT(s7, (10 + (2 * 3)));

/* Variable length array with nested expression */
struct s8 GTY((length("(len + (sizeof(int)-1)) / sizeof(int)"))) {
    int items[];
    size_t len;
};

/* Conditional expressions in GTY options */
struct s9 GTY((param_is_isomorphic_to("(IS_PTR ? typeof(*%h) : typeof(%h))"))) {
    void *ptr;
    int is_ptr;
};

/* Nested switch-case like expression in desc */
struct s10 GTY((desc("((%1.type == TYPE_A) ? 0 : (%1.type == TYPE_B) ? 1 : 2)"))) {
    enum { TYPE_A, TYPE_B, TYPE_C } type;
    union {
        int a;
        float b;
        double c;
    } value;
};

/* Multiple levels of nested parentheses */
struct s11 GTY((skip((("skip1") ("skip2")) (("skip3") ("skip4"))))) {
    void *skip1;
    void *skip2;
    void *skip3;
    void *skip4;
    int value;
};

/* Array with macro containing parentheses */
struct s12 GTY((length("NESTED_EXPR"))) {
    int data[];
};

/* Complex function pointer type with nested brackets and parentheses */
typedef int (*(*complex_func_ptr GTY((callback)))[ (2 + 3) ])(
    int (*)(int [ (4) ]),
    void * GTY((skip))
);

/* Union with GTY and nested structure */
union u2 GTY((user)) {
    struct {
        int x GTY((tag("X_FIELD")));
        int y;
    } point;
    int coordinates[2];
};

/* Forward declaration with GTY annotation containing parentheses */
struct forward_decl GTY((user));
struct forward_decl {
    int value;
    struct forward_decl *next;
};

/* Final test: all three delimiters in one type */
struct comprehensive GTY((user)) {
    /* Parentheses in length expression */
    int *array GTY((length("(count * (sizeof(int) / 2))")));
    
    /* Brackets in array dimension */
    int matrix[ (2 + (3 * 1)) ][ (4) ];
    
    /* Braces in nested struct */
    struct {
        int a;
        int b;
    } GTY((tag("NESTED"))) nested;
    
    int count;
};

#endif /* TEST_PARSE_H */
