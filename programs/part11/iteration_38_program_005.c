/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define ARRAY_SIZE ( (2) * (3) + (4) )
#define COMPLEX_BRACES { struct inner { int x; }; }
#define CALLBACK_FUNC ((void (*)(int))0)

/* Helper macro for nested parentheses */
#define GTY_LENGTH_EXPR "sizeof(int) * (2 + (3 * (4)))"

/* Test case 1: Struct with nested parentheses in GTY length option */
struct s1 GTY((length(GTY_LENGTH_EXPR))) {
    int data[];
};

/* Test case 2: Struct with multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Test case 3: Array with complex dimension expression inside brackets */
struct s3 GTY((user)) {
    int arr GTY((length("N")))[ (2 * 3) + (4 - 1) ];
    my_int matrix[ (1 << 2) ][ (3 + 2) ];
};

/* Test case 4: Pointer to array with nested brackets */
typedef int (*array_ptr_t GTY((tag("ARRAY_PTR"))))[ (sizeof(int) > 4) ? 8 : 16 ];

/* Test case 5: Function pointer with complex argument list */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (4) + (2) ], char *),
    struct s1 *,
    ptr_t
);

/* Test case 6: Union with array size using macro expansion */
union u1 GTY((desc("%0.type"))) {
    int type;
    char arr[ ARRAY_SIZE + (5) ];
    double dbl;
};

/* Test case 7: Nested structure with braces in definition */
struct outer GTY((user)) {
    struct inner GTY((tag("INNER"))) {
        int x;
        int y;
        char data[ (10) ];
    } nested;
    int count;
};

/* Test case 8: Forward declared struct with complex GTY annotation */
struct forward_decl GTY((user, maybe_undef));

/* Test case 9: Complete forward declared struct with nested parentheses */
struct forward_decl {
    int value GTY((length("(1 << 5) + (sizeof(int) * 2)")));
    char *name;
};

/* Test case 10: Typedef with GTY and array declarator */
typedef struct s4 GTY((chain_next("nxt"))) {
    int id;
    struct s4 *nxt;
    float values[ (NESTED_EXPR) / 2 ];
} s4_t;

/* Test case 11: Complex conditional in array dimension */
struct s5 GTY((user)) {
    unsigned char flags[ (sizeof(void*) == 8) ? 64 : 32 ];
    int *ptr_array[ (10 + (5 * 2)) ];
};

/* Test case 12: Multiple levels of nested parentheses in expression */
#define COMPLEX_CALC ((a) + ((b) * ((c) + (d))) - ((e) / ((f) + 1)))
struct s6 GTY((length("COMPLEX_CALC"))) {
    long data[];
};

/* Test case 13: GTY with skip option containing parentheses */
struct skip_example GTY((skip(("skip_field")))) {
    int skip_field;
    int keep_field GTY((user));
};

/* Test case 14: Embedded initializer-like syntax (rare but possible) */
struct with_init GTY((user)) {
    int x;
    /* This comment contains braces { } for testing */
    int y;
};

/* Test case 15: Macro that expands to brace-enclosed code */
#ifdef TEST_BRACES
#define BRACE_MACRO { int temp = 0; }
struct brace_test GTY((user)) {
    int value;
    /* BRACE_MACRO would go here if allowed */
};
#endif

/* Test case 16: Array of function pointers */
typedef int (*func_array_t[ (3) + (2) ])(int, char *);

/* Test case 17: GTY on typedef with complex type */
typedef struct complex_type GTY((user)) {
    union {
        int i;
        double d;
    } u;
    func_array_t funcs;
} complex_type_t;

/* Test case 18: Multiple nested structures */
struct level1 GTY((user)) {
    struct level2 {
        struct level3 GTY((tag("LEVEL3"))) {
            int deep_value;
            char name[ (20) + (10) ];
        } deepest;
        int mid_value;
    } middle;
    int top_value;
};

/* Test case 19: Variable length array in struct */
struct vla_example GTY((length("len"))) {
    size_t len;
    int items[];
};

/* Test case 20: Final comprehensive test with all delimiters */
struct final_test GTY((chain_next("next"), chain_prev("prev"), 
                       length("(count * sizeof(int)) + (padding)"))) {
    struct final_test *next;
    struct final_test *prev;
    int count;
    int padding;
    int data[ ( (count) > 0 ? (count) : 1 ) ];
    struct {
        int x;
        int y;
    } point;
};

#endif /* TEST_PARSE_H */
