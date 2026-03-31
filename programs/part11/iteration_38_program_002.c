/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ((sizeof(int) * (2 + 3)))
#define BRACKET_EXPR [ (10) + (5) ]
#define BRACE_INIT { .x = 1, .y = 2 }
#define COMPLEX_MACRO(x) ((x) * (2 + (3)))

/* Forward declarations */
struct forward_decl GTY((user));

/* Test case 1: Parentheses in GTY length option */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* Test case 2: Multiple parenthesized groups in GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Test case 3: Nested parentheses in callback declaration */
typedef void (*callback_fn GTY((callback)))(int (*)(int [ (4) ]));

/* Test case 4: Array with complex dimension expression in brackets */
struct s3 GTY((length("N"))) {
    int arr[ (2 * 3) + (4 / 2) ];
};

/* Test case 5: Pointer to array with nested brackets */
typedef int (*array_ptr GTY((user)))[ (5) ][ (3) ];

/* Test case 6: Union with array containing parenthesized size */
union u1 GTY(()) {
    int i;
    char arr[ 10 + (5) ];
    long long big[ (sizeof(long) == 8) ? 2 : 1 ];
};

/* Test case 7: Structure with nested structure definition (braces) */
struct outer GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    int count;
};

/* Test case 8: GTY with skip option containing parenthesized expression */
struct s4 GTY((skip(("skip_func")))) {
    int data;
    void *ptr;
};

/* Test case 9: Complex array dimensions with multiple bracket levels */
struct s5 {
    int (*complex_arr GTY((length("(dim1 * dim2)"))))[ (2) ][ (3) ];
};

/* Test case 10: Function pointer type with nested parentheses */
typedef int (*complex_fp GTY((callback)))(
    int (*)(char *[], int),
    void (*)(struct s1 *)
);

/* Test case 11: Use of macro expansion with parentheses */
struct s6 GTY((length("NESTED_PAREN_EXPR"))) {
    double values[];
};

/* Test case 12: Conditional in array dimension */
struct s7 GTY((user)) {
    unsigned char flags[ (sizeof(void*) == 8) ? 8 : 4 ];
};

/* Test case 13: Nested structure with array of structures */
struct container GTY((user)) {
    struct element GTY((chain_next("next"))) {
        struct element *next;
        int id;
        float data[ (10) ];
    } *first;
    int count;
};

/* Test case 14: Union with anonymous struct containing braces */
union u2 GTY((user)) {
    struct {
        int a;
        int b;
    };
    long long both;
};

/* Test case 15: Template-like macro usage (GCC extension) */
#define DECLARE_VECTOR(type, size) type data[size]

struct s8 GTY((user)) {
    DECLARE_VECTOR(int, (16));
};

/* Test case 16: GTY options with string containing delimiters */
struct s9 GTY((desc("%s"), param_is(struct forward_decl))) {
    char *name;
    struct forward_decl *fd;
};

/* Test case 17: Multiple nested parentheses in expression */
struct s10 GTY((length("((1 << 5) + (sizeof(int) * 2))"))) {
    unsigned char buffer[];
};

/* Test case 18: Array of function pointers */
struct s11 GTY((user)) {
    int (*funcs[ (3) ])(int, char *);
};

/* Test case 19: Bitfield with complex expression */
struct s12 GTY((user)) {
    unsigned int flags : (sizeof(int) * 8 - 1);
    unsigned int last : 1;
};

/* Test case 20: Complete test with all delimiters */
struct comprehensive GTY((
    chain_next("next"),
    chain_prev("prev"),
    length("count"),
    skip(("skip_comp")),
    user
)) {
    struct comprehensive *next;
    struct comprehensive *prev;
    int count;
    union {
        int ival;
        char cval[ (16) ];
    } data;
    struct {
        int x;
        int y;
    } point;
};

#endif /* TEST_PARSE_H */
