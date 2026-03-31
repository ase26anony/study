/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef unsigned int size_t;

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define COMPLEX_SIZE (10 + (5 * (2 - 1)))
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define CALLBACK_CAST (void (*)(int (*)(int [ (4) ])))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* More parentheses - multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array with complex dimension expression */
struct s3 GTY((user)) {
    int arr GTY((length("N")))[ (2 * 3) + (4 / 2) ];
    size_t N;
};

/* Pointer to array with nested brackets */
typedef int (*array_ptr_t GTY((tag("ARRAY_PTR"))))[ (5) ][ (3 + 2) ];

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y[2];
    } nested;
    int value;
};

/* Union with array containing brace initializer in GTY context */
union u1 GTY((desc("((%1.type == 0) ? 0 : 1)"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        int x;
        int y;
    } point;
};

/* Function pointer with complex signature containing parentheses and brackets */
typedef void (*complex_fn_ptr GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ]),
    void (*)(struct s1 *)
);

/* GTY with skip option containing parenthesized expression */
struct skip_example GTY((skip(("(char **)") "ptr"))) {
    void *ptr;
    int count;
};

/* Nested GTY annotations with multiple delimiter types */
struct deeply_nested GTY((chain_next("next"), chain_prev("prev"))) {
    struct deeply_nested *next;
    struct deeply_nested *prev;
    
    /* Array with parenthesized size expression */
    int matrix GTY((length("rows * cols")))[ (10) ][ (20) ];
    int rows;
    int cols;
    
    /* Nested structure with braces */
    struct {
        int id;
        char name[ (32) ];
    } info GTY((tag("INFO")));
};

/* Template-like macro usage with GTY */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name GTY((length(#size))) { \
        int data[size]; \
        int (*processor)(int [ (size) ]); \
    }

/* Instantiate the macro with parenthesized expression */
DEFINE_GTY_STRUCT(macro_struct, (5 + 3));

/* GTY with if_marked option containing complex expression */
struct marked_struct GTY((if_marked("((struct marked_struct *)ptr)->marked"))) {
    int marked;
    void *data;
    struct marked_struct *link;
};

/* Array of function pointers with GTY */
typedef int (*func_array_t GTY((length("count")))[ (5) ])(
    char *str,
    int options[ (MAX_OPTIONS) ]
);
#define MAX_OPTIONS (10 + (2 * 3))

/* Union with GTY and nested switch expression */
union switch_union GTY((desc("(%1.type == UNION_TYPE_A) ? 0 : "
                            "(%1.type == UNION_TYPE_B) ? 1 : 2"))) {
    int type;
    struct {
        int a;
        int b[ (8) ];
    } type_a;
    struct {
        char *name;
        int values[ (16) ];
    } type_b;
};

/* Forward declaration with GTY containing parenthesized expression */
struct forward_decl GTY((user, tag("(struct forward_decl *)"))) ;

/* Complete definition */
struct forward_decl {
    int id;
    struct forward_decl *next;
    char data[ (256) ];
};

/* GTY with nested structure and array */
struct container GTY((user)) {
    /* This nested struct definition contains braces */
    struct element GTY((tag("ELEMENT"))) {
        int key;
        char value[ (64) ];
        struct element *next;
    } *elements;
    
    int count;
    int sizes[ (10) ][ (20) ];
};

/* Test case specifically for consume_balanced with all delimiters in one */
struct all_delimiters GTY((user)) {
    /* Parentheses in array size */
    int a[ (5 + (3 * 2)) ];
    
    /* Brackets in type declaration */
    int (*b[ (3) ])(int [ (10) ]);
    
    /* Nested structure with braces */
    struct {
        int x;
        struct {
            int y;
            int z[ (2) ];
        } inner;
    } nested;
};

#endif /* TEST_PARSE_H */
