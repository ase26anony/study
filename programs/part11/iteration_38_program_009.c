/* test_parse.h - Complex GTY annotations to exercise delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_fnptr)(void);

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define NESTED_MACRO(x) ((x) * ((x) + 1))

/* Case 1: Parentheses - struct with GTY((length("(complex expression)"))) */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with nested parentheses in length expression */
    int data GTY((length("(NESTED_PAREN_EXPR) / sizeof(int)")))[];
};

/* Case 2: Parentheses - GTY with multiple parenthesized groups */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node *next;
    struct list_node *prev;
};

/* Case 3: Parentheses - typedef with function pointer and complex cast */
typedef int (*complex_fnptr GTY((callback)))(
    int (*)(int GTY((skip)) [ (sizeof(int) * 2) ]),
    void * GTY((user))
);

/* Case 4: Brackets - array with complex dimension expression */
struct s2 GTY(()) {
    int arr GTY((length("N"))) [ (2 * 3) + (4 / 2) ];
    char nested_arr GTY((length("M"))) [ 5 ][ (10) ];
};

/* Case 5: Brackets - pointer to array with nested brackets */
typedef int (*array_ptr GTY((user))) [ (sizeof(int) == 4) ? 10 : 20 ];

/* Case 6: Brackets - multi-dimensional array with macro expansion */
struct s3 GTY(()) {
    int matrix GTY((length("ROWS"), param_is("COLS"))) 
        [ NESTED_MACRO(5) ][ COMPLEX_BRACKET_EXPR[0] ];
};

/* Case 7: Braces - union with GTY and nested structure definition */
union u1 GTY((desc("0"))) {
    int i;
    struct inner GTY((tag("1"))) {
        int x;
        int y GTY((length("2")))[2];
    } nested;
    char str GTY((length("(10 + 5)")))[];
};

/* Case 8: Braces - struct with GTY marker containing nested anonymous struct */
struct s4 GTY((user)) {
    struct GTY((tag("LANG"))) {
        int lang_specific;
        void *data GTY((length("(lang_size)")));
    } lang_data;
    
    /* Array with initializer-like syntax in GTY */
    int values GTY((length("COUNT"), 
                   param_is("COUNT ? COUNT : (DEFAULT_COUNT)"))) [];
};

/* Case 9: Complex nested case - all delimiters combined */
struct complex_type GTY((chain_next("next"))) {
    /* Parentheses in function pointer type */
    void (*callback GTY((callback)))(
        int, 
        struct complex_type * GTY((skip))
    );
    
    /* Brackets in array with parenthesized expression */
    int matrix GTY((length("(rows * cols)"))) [ (10) ][ (20) ];
    
    /* Nested struct with braces */
    struct {
        int x;
        union {
            int i;
            char c GTY((length("(sizeof(char))")))[];
        } u GTY((tag("2")));
    } nested GTY((user));
    
    struct complex_type *next;
};

/* Case 10: Forward declaration with GTY containing parenthesized expression */
struct forward_decl GTY((user, length("(is_valid ? size : 0)")));

/* Case 11: Enum with GTY marker (uses parentheses) */
enum my_enum GTY((tag("ENUM_TYPE"))) {
    VALUE1,
    VALUE2 GTY((skip)),
    VALUE3
};

/* Case 12: Typedef with GTY and function type containing brackets */
typedef void (*signal_handler GTY((callback)))(
    int signum,
    void *context GTY((user)),
    int extra_data GTY((length("(signum * 2)"))) []
);

/* Case 13: Structure with conditional GTY options using parentheses */
struct conditional_gt GTY((
    maybe_undef,
    param_is("USE_CHAIN ? chain_next(\"next\") : skip")
)) {
    int data;
    struct conditional_gt *next;
};

/* Case 14: Array of pointers with nested GTY markers */
typedef struct ptr_array GTY((length("(count + 1)"))) {
    void * GTY((user)) items[];
} ptr_array_t;

/* Case 15: Union containing array with size from macro containing parentheses */
union final_union GTY((desc("3"))) {
    int as_int;
    char as_chars GTY((length("(sizeof(int))")))[ sizeof(int) ];
    struct s1 *as_ptr GTY((tag("PTR")));
};

#endif /* TEST_PARSE_H */
