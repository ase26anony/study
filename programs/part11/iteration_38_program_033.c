/* test_parse.h - Complex GTY annotations to exercise balanced delimiter parsing */

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
#define NESTED_CONDITIONAL ( (1) ? (2) : ((3) + (4)) )

/* Forward declarations */
struct forward_decl;

/* ========== PARENTHESES TEST CASES ========== */

/* Case 1: GTY with nested parentheses in length attribute */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* Case 2: Multiple parenthesized groups in GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 3: Function pointer with complex parenthesized argument list */
typedef void (*complex_fn_ptr GTY((callback)))(
    int (*)(int, char),
    void (*)(struct s1 *),
    int
);

/* Case 4: GTY with deeply nested parentheses */
struct s3 GTY((length("((((1) + (2)) * ((3) - (4))) / (5))"))) {
    double values[];
};

/* ========== BRACKETS TEST CASES ========== */

/* Case 5: Array with complex dimension expression in GTY */
struct s4 GTY((length("N"))) {
    int arr[ (2 * 3) + sizeof(int) ];
};

/* Case 6: Pointer to array with nested brackets */
typedef int (*array_ptr GTY(()))[ (4) + (5) ];

/* Case 7: Multi-dimensional array with complex expressions */
struct s5 GTY(()) {
    int matrix[ (1 << 2) ][ (3 * 4) ][ sizeof(double) / sizeof(int) ];
};

/* Case 8: GTY on type containing array declarator with macro expansion */
struct s6 GTY((user)) {
    char buffer[ NESTED_PAREN_EXPR ];
};

/* ========== BRACES TEST CASES ========== */

/* Case 9: Structure with nested structure definition (contains braces) */
struct s7 GTY((tag("LANG"))) {
    struct inner GTY((user)) {
        int x;
        int y;
        struct innermost {
            char c;
        } z;
    } nested;
    
    union {
        int a;
        char b;
    } u;
};

/* Case 10: Union with GTY and initializer-like syntax in comments */
union u1 GTY((desc("1"))) {
    int i;
    char arr[ 10 + (5) ];
    /* Simulating complex initializer in comment: { .i = 1, .arr = {1,2,3} } */
};

/* Case 11: Structure with anonymous union containing braces */
struct s8 GTY((user)) {
    int tag;
    union {
        struct { int x; int y; } point;
        struct { char *name; int id; } info;
    } data;
};

/* ========== COMBINED DELIMITER TESTS ========== */

/* Case 12: All delimiters combined in one complex type */
struct s9 GTY((length("(sizeof(struct { int x; })"), user)) {
    /* Array with parenthesized size */
    int complex_array[ (sizeof(int) * (2 + 3)) ];
    
    /* Nested structure with braces */
    struct {
        int nested_value;
        /* Pointer to function with parenthesized args */
        void (*nested_func)(int (*)(int [ (4) ]));
    } inner;
    
    /* Union with array */
    union {
        char chars[ (10) ];
        int ints[ (5) ];
    } variant;
};

/* Case 13: Typedef with function pointer containing all delimiters */
typedef int (*ultimate_callback GTY((callback)))(
    struct s9 *,
    int (*)(char [ (sizeof(int)) ], struct { int x; } *),
    void (*)(void)
);

/* Case 14: GTY with string containing delimiter characters */
struct s10 GTY((variable_size)) {
    /* The string literal contains delimiter chars which should be skipped */
    const char *description; /* Could be "array[10] = {1,2,3}" */
    int count;
};

/* Case 15: Macro expansion within GTY arguments */
#define ARRAY_SIZE ( (10) * (20) )
struct s11 GTY((length("ARRAY_SIZE"))) {
    unsigned char data[];
};

/* Case 16: Conditional compilation with GTY */
#ifdef SPECIAL_FEATURE
struct s12 GTY((special)) {
    int special_data[ (1 << 4) ];
};
#else
struct s12 GTY((normal)) {
    int normal_data[ (2 * 8) ];
};
#endif

/* ========== EDGE CASES ========== */

/* Case 17: Empty braces in nested structure */
struct s13 GTY((user)) {
    struct empty {};
    int value;
};

/* Case 18: GTY on enum with complex expressions */
enum e1 GTY((user)) {
    VALUE_A = (1 << 0),
    VALUE_B = (1 << 1),
    VALUE_C = (1 << 2) | (1 << 3)
};

/* Case 19: Pointer to array of function pointers */
typedef int (*(*complex_array_ptr GTY(()))[ (3) + (2) ])(int, char);

/* Case 20: Final test with deeply nested all delimiters */
struct s14 GTY((length("(sizeof(int [ (2) ][ (3) ]))"))) {
    struct {
        union {
            int x;
            struct {
                char c[ (4) ];
            } s;
        } u;
    } nested[ (1) + (1) ];
};

#endif /* TEST_PARSE_H */
