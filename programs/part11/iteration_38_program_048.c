/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to introduce nested delimiter layers */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = (5), .y = {1, 2, 3} }
#define COMPLEX_MACRO(x) ((x) * ((x) + 1))

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

/* Even more nested parentheses in callback */
typedef void (*complex_callback GTY((callback)))(int (*)(int, int), 
                                                 void *);

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY((user)) {
    int arr GTY((length("N")))[ (2 * 3) + sizeof(int) ];
    char multi_dim GTY((tag("ARRAY")))[5][ (10) ];
};

/* Pointer to array with brackets */
typedef int (*array_ptr GTY((skip)))[ (sizeof(int) > 4) ? 10 : 20 ];

/* Case 3: Braces - structure with nested struct definition */
struct outer GTY((user)) {
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y;
        struct deeper {
            int z;
        } deep;
    } nested;
    
    union inner_union GTY((desc("$1.type"))) {
        int i;
        float f;
        struct { char a; char b; } chars;
    } u;
};

/* Union with array and initializer-like syntax in GTY */
union u1 GTY((desc("((int)$1) & 1"))) {
    int i;
    char arr[ 10 + (5 * sizeof(int)) ];
    long long big;
};

/* Function pointer with complex argument list containing brackets */
typedef int (*func_ptr_t GTY((callback)))(
    int matrix[][ (3 * 4) ], 
    void (*callback)(int, char *[])
);

/* Struct with flexible array member and nested expressions */
struct flexible GTY((length("(1 << 5) + sizeof(long)"))) {
    int count;
    double items[];
};

/* Nested structure with multiple GTY annotations */
struct container GTY((user)) {
    struct element GTY((chain_next("next_element"))) {
        struct element *next_element;
        int data[ (sizeof(void*) * 2) ];
    } *first;
    
    /* GTY with skip and param_is option containing parentheses */
    void *ptr GTY((skip("(struct container *)$1"), 
                   param_is("struct container *")));
};

/* Template-like macro usage with GTY */
#define DECLARE_GTY_STRUCT(name, size) \
    struct name##_t GTY((length(#size))) { \
        unsigned char data[size]; \
    }

/* Instantiate with complex size expression */
DECLARE_GTY_STRUCT(buffer, (256 * sizeof(int)));

/* Another complex case: GTY with condition containing parentheses */
struct conditional GTY((if("(flags & 1) != 0"))) {
    int flags;
    void *data GTY((length("(flags & 2) ? 10 : 20")));
};

/* Array of function pointers */
typedef void (*func_array_t GTY((length("5"))))[
    (sizeof(void*) == 8) ? 10 : 20
];

/* Struct with anonymous union containing array */
struct with_anon_union GTY((user)) {
    int type;
    union {
        int ints[ (4 + 1) ];
        struct {
            char *str;
            int len;
        } string;
    } data;
};

/* Multi-level nested parentheses in GTY options */
struct deeply_nested GTY((
    maybe_undef,
    tag("DEPTH_((3))"),
    reorder("(a), (b), ((c) + (d))")
)) {
    int a, b, c, d;
};

/* Final test: all delimiters combined */
struct all_delimiters GTY((
    user,
    desc("(($1)->type == 1) ? 'A' : 'B'"),
    length("[sizeof(struct all_delimiters)]")
)) {
    int type;
    union {
        struct { int x[(2+3)]; } s;
        char *p;
    } u GTY((tag("($1)->type")));
};

#endif /* TEST_PARSE_H */
