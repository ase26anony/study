/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define COMPLEX_BRACE { struct inner { int x; }; }

/* Forward declarations */
struct forward_decl GTY((user));

/* Case 1: Parentheses - struct with GTY((length("(complex expression)"))) */
struct s1 GTY((length("(1 << 5) + (sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* Case 2: Multiple parenthesized groups in GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 3: Function pointer with nested parentheses in argument */
typedef void (*complex_fn_ptr GTY((callback)))(
    int (*)(int [ (4) + (2) ], 
            void (*)(char (*(*)[5])()))
);

/* Case 4: Array with complex dimension expression inside brackets */
struct s3 GTY((length("N"))) {
    int arr[ (2 * 3) + sizeof(int) ];
    int N;
};

/* Case 5: Pointer to array with nested brackets */
typedef int (*array_ptr GTY((user)))[ (10) ][ (20) ];

/* Case 6: Union with array containing parenthesized size */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5 * sizeof(int)) ];
    long long ll;
};

/* Case 7: Nested structure definition with braces inside GTY context */
struct outer GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    int outer_val;
};

/* Case 8: GTY with skip/param options containing parentheses */
struct s4 GTY((skip(("skip_me")), param(("user_data")))) {
    void *data;
    int (*skip_me)(void *);
    void *user_data;
};

/* Case 9: Conditional expression in GTY length */
struct s5 GTY((length("(cond) ? (10) : (20)"))) {
    char buffer[];
    int cond;
};

/* Case 10: Multiple levels of nested parentheses */
struct s6 GTY((length("((((5) + (3)) * ((2) - (1))) / (sizeof(int)))"))) {
    double values[];
};

/* Case 11: Array of function pointers with complex signatures */
typedef int (*(*complex_array GTY((user)))[5])(
    int, 
    char (*)[ (2) + (3) ],
    void (*)(struct s1 *)
);

/* Case 12: Structure with embedded union containing array */
struct s7 GTY((user)) {
    union {
        int a;
        struct {
            char c[ (sizeof(int) + 1) ];
            short s;
        } GTY((tag("EMBEDDED"))) inner;
    } u;
};

/* Case 13: Forward declared struct with GTY containing parentheses */
struct forward_decl GTY((chain_next("next"))) {
    struct forward_decl *next;
    int data;
};

/* Case 14: Variable length array in parameter */
typedef struct s8 GTY((user)) {
    int len;
    /* This will trigger bracket parsing in type declaration */
    int items[0];
} s8_t;

/* Case 15: Macro expansion with parentheses in GTY */
#define ARRAY_LEN_EXPR ( (16) + (8) )
struct s9 GTY((length("ARRAY_LEN_EXPR"))) {
    unsigned char bytes[];
};

/* Case 16: Multiple attribute GTY with deeply nested expressions */
struct s10 GTY((length("(int)((double)(10) * (3.14159))"),
                skip(("(void*)0")),
                param(("(void*)((char*)0 + (100))")))) {
    void *ptr;
    int count;
};

/* Case 17: Union with GTY and bitfields (uses braces) */
union u2 GTY((user)) {
    struct {
        unsigned int a : (3) + (2);
        unsigned int b : (8) - (3);
    } bits;
    unsigned int full;
};

/* Case 18: Structure containing anonymous union (braces) */
struct s11 GTY((user)) {
    int type;
    union {
        int i;
        float f;
        char str[ (10) ];
    } data;
};

/* Case 19: Nested array dimensions with parentheses */
typedef int multi_array GTY((user))[ (2) ][ (3) ][ (4) ];

/* Case 20: Final complex case combining all delimiters */
struct s12 GTY((length("(int)({ int x = (5); x *= (2); x; })"),
                user)) {
    /* This comment contains examples of all delimiters for parser:
       { brace }, [ bracket ], ( paren ) */
    struct nested {
        int (*func_ptr)(int [ (2) ]);
        union {
            char c;
            int i;
        } choice;
    } GTY((tag("COMPLEX"))) inner;
    int values[ (sizeof(struct nested)) / (sizeof(int)) ];
};

#endif /* TEST_PARSE_H */
