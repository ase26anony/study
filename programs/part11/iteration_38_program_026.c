/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define NESTED_MACRO(x) ((x) * ((x) + 1))

/* ===== Test Case 1: Parentheses in GTY options ===== */

/* Struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Array with complex length expression in parentheses */
    int* data GTY((length("(sizeof(int) * (NESTED_PAREN_EXPR))")));
};

/* Function pointer type with nested parentheses in arguments */
typedef int (*complex_func_ptr GTY((callback)))(
    int (*)(int, char*),
    void* GTY((skip))
);

/* Union with chain_next/chain_prev containing parenthesized expressions */
union u1 GTY((chain_next("((next))"), chain_prev("((prev))"))) {
    int value;
    char* str GTY((length("(strlen(str) + (1))")));
};

/* ===== Test Case 2: Brackets in GTY annotations ===== */

/* Array type with complex dimension expression in brackets */
struct s2 GTY(()) {
    /* Multi-dimensional array with nested brackets */
    int matrix GTY((length("N * M")))[ (2 * 3) ][ (4 + 1) ];
    
    /* Pointer to array with bracketed size expression */
    int (*ptr_array GTY((length("size"))))[ (sizeof(int) * 2) ];
};

/* Typedef for array of function pointers with brackets */
typedef void (*func_array GTY((length("10")))[ (5) ])(int, char);

/* Struct with flexible array member using bracketed expression */
struct s3 GTY((length("(count * (2))"))) {
    int count;
    /* Flexible array with complex size expression */
    double items[];
};

/* ===== Test Case 3: Braces in GTY context ===== */

/* Nested structure definition within GTY annotation */
struct outer GTY((user)) {
    /* Inner struct definition with braces */
    struct inner GTY((tag("SPECIAL"))) {
        int x;
        int y GTY((length("(x * 2)")));
    } nested;
    
    /* Union with initializer-like syntax in comment/string */
    union {
        int a;
        char b[10];
    } u GTY((desc("$1.u.a ? 1 : 0")));
};

/* Forward declared struct with GTY and later definition with braces */
struct forward_decl GTY((user));

struct forward_decl {
    int id;
    /* Array with size from macro containing parentheses */
    char name[ NESTED_PAREN_EXPR ];
};

/* ===== Test Case 4: Mixed delimiters ===== */

/* Complex type mixing all delimiter types */
struct mixed_delimiters GTY((user)) {
    /* Parentheses in function pointer type */
    void (*callback GTY((callback)))(int (*handler)(int[ (10) ]));
    
    /* Array with complex GTY options containing parentheses */
    struct s1* items GTY(
        (length("count")),
        (skip("((void*)0)")),
        (tag("ITEM_LIST"))
    );
    
    /* Nested anonymous struct with braces */
    struct {
        int flags;
        /* Bitfield with parenthesized expression */
        unsigned int bits: (sizeof(int) * 8 - 1);
    } GTY((desc("$1.bits"))) state;
};

/* ===== Test Case 5: Template-like patterns ===== */

/* Macro that expands to include braces (for parser testing) */
#define GTY_WITH_BRACES(...) GTY(__VA_ARGS__)

struct macro_test GTY_WITH_BRACES((user)) {
    /* Using macro that might confuse delimiter balancing */
    int* array GTY((length("(NESTED_MACRO(5))")));
};

/* ===== Test Case 6: Edge cases ===== */

/* Empty braces in struct definition */
struct empty_braces GTY((user)) {
    /* Nothing inside, but braces are present */
};

/* GTY on typedef with function type containing nested parentheses */
typedef int (complex_type GTY((callback))[10])(
    int (*)(int, int),
    char*[]
);

/* Union with array containing bracket expressions */
union final_test GTY((user)) {
    int scalar;
    /* Array dimension with parenthesized expression inside brackets */
    char buffer[ (sizeof(struct s1) + (16)) ];
    
    /* Pointer to array with nested brackets in type */
    int (*multi_array)[ (2) ][ (3) ];
};

#endif /* TEST_PARSE_H */
