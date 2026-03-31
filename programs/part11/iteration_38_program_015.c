/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef unsigned int size_t;

/* Macros to introduce nested delimiter layers */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Forward declarations */
struct forward_decl GTY((user));

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((length("(2 + 3) * sizeof(int)"))) {
    int data[];
};

/* More complex parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Even deeper parentheses nesting */
struct s3 GTY((length("NESTED_PAREN_EXPR"))) {
    char buffer[];
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s4 GTY((user)) {
    int arr GTY((length("N")))[ (2 * 3) + 4 ];
    int *ptr GTY((length("M")));
};

/* Pointer to array with brackets */
typedef int (*array_ptr GTY((user)))[ (sizeof(int) > 4) ? 8 : 16 ];

/* Multi-dimensional array with brackets */
struct s5 GTY((user)) {
    int matrix GTY((length("ROWS"), param_is("COLS")))[ (3 + 2) ][ (4 * 2) ];
};

/* Case 3: Braces - structure with nested struct definition */
struct s6 GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    
    union {
        int i;
        float f;
    } value GTY((skip));
};

/* Union with array and braces in initializer position (in macro expansion) */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        short a;
        short b;
    } pair;
};

/* Function pointer type with complex signature containing parentheses and brackets */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ]),
    struct s1 * GTY((skip))
);

/* Struct with conditional in array dimension using parentheses */
struct s7 GTY((user)) {
    int items GTY((length("count")))[ (sizeof(void*) == 8) ? 64 : 32 ];
};

/* Nested structures with GTY markers at different levels */
struct outer GTY((user)) {
    struct middle GTY((tag("MIDDLE"))) {
        struct inner GTY((tag("INNER"))) {
            int depth;
        } innermost;
        int level;
    } mid;
    int id;
};

/* Array of function pointers - complex parentheses nesting */
typedef int (*func_array GTY((user))[ (4) + (2) ])(int, int);

/* Struct with skip annotation containing parenthesized expression */
struct s8 GTY((skip("(unsigned long)"))) {
    void *data;
    size_t size;
};

/* Union with nested anonymous struct containing array with brackets */
union u2 GTY((user)) {
    struct {
        int flags GTY((length("flag_count")))[ (1 << 3) ];
        char name[ (32) + 1 ];
    } header;
    char raw[256];
};

/* Forward-declared struct with GTY annotation containing parenthesized conditional */
struct forward_decl GTY((user, desc("(tag == 1) ? 0 : 1"))) {
    int tag;
    void *data;
};

/* Typedef with GTY and nested parentheses in attribute */
typedef struct {
    int counter;
    void *buffer GTY((length("(alloc_size / sizeof(int))")));
} buffer_container GTY((user));

/* Macro expansion that creates complex delimiter sequences */
#define GTY_WITH_NESTING(member) GTY((length("ARRAY_SIZE(" #member ")")))

struct s9 GTY_WITH_NESTING(items) {
    int items[ (5) + (3) ];
    int count;
};

/* Final test: All three delimiters in one type */
struct comprehensive GTY((user)) {
    /* Parentheses in length expression */
    int *array GTY((length("(capacity + 1)")));
    
    /* Brackets in array declaration */
    char name[ (32) ];
    
    /* Braces in nested struct */
    struct {
        int x;
        int y;
    } point;
    
    /* Function pointer with parentheses */
    void (*handler)(int, char * GTY((skip)));
};

#endif /* TEST_PARSE_H */
