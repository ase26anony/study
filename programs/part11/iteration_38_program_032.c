/* test_parse.h - Complex GTY annotations to test delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = (1 + 2), .y = {3, 4} }
#define COMPLEX_MACRO(x) ((x) + ( (x) * (2) ))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Array with length containing nested parentheses */
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
};

/* More parentheses: chain_next/chain_prev with multiple parenthesized groups */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node *next;
    struct list_node *prev;
};

/* Even more nested parentheses in skip expression */
struct tree_node GTY((skip(("tree_node")"))) {
    int data;
    struct tree_node *left;
    struct tree_node *right;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s2 GTY((user)) {
    /* Array with dimension containing parentheses */
    int arr1 GTY((length("N")))[ (2 * 3) + (4 / 2) ];
    
    /* Pointer to array with nested brackets */
    int (*arr2 GTY((length("M"))))[ (sizeof(int) > 4) ? 10 : 20 ];
    
    /* Multi-dimensional array */
    int matrix GTY((length("ROWS"), length("COLS")))[ (1 << 3) ][ (8 / 2) ];
};

/* typedef with array in function pointer */
typedef int (*array_func_ptr GTY((callback)))(int arr[ (4) + (2) ]);

/* Case 3: Braces - structure with nested struct definition */
struct outer GTY((user)) {
    int id;
    /* Nested structure definition with braces */
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    
    /* Union with initializer-like syntax in GTY */
    union {
        int i;
        float f;
    } value GTY((desc("(%s ? 0 : 1)")));
};

/* Union containing array with bracketed size expression */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5) ];
    long long big[ (sizeof(long) == 8) ? 2 : 4 ];
};

/* Forward-declared struct with GTY annotation containing nested expression */
struct forward_decl GTY((user, if("(defined(FLAG) && (FLAG > 0))")));

/* Function pointer type with complex argument list containing parentheses */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ], 
            struct s1 * GTY((skip))),
    void (*)(void)
);

/* Structure with conditional GTY options */
struct conditional GTY((
    maybe_undef,
    if("(defined(USE_GC) && (USE_GC == 1))"),
    skip(("(void*)0"))
)) {
    void *data;
};

/* Array of pointers with nested GTY options */
struct ptr_array GTY((user)) {
    void * GTY((skip)) items[ (16) ];
    int count;
};

/* Nested structure with array of structures */
struct container GTY((user)) {
    struct element GTY((tag("1"))) {
        int id;
        char name[ (32) + (8) ];
    } elements[ (100) ];
    
    /* Embedded anonymous struct */
    struct GTY((tag("2"))) {
        int x;
        int y;
    } point;
};

/* Macro expansion within GTY arguments */
#define ARRAY_LEN_EXPR ( (10) + (20) )
struct macro_expansion GTY((user)) {
    int data GTY((length( "ARRAY_LEN_EXPR" )))[];
};

/* Even more complex: GTY options with string literals containing delimiters */
struct string_delimiters GTY((user)) {
    /* String containing parentheses */
    const char *msg1 GTY((length("strlen(\"(test)\") + 1")));
    
    /* String containing brackets */
    const char *msg2 GTY((length("sizeof(\"[test]\")")));
    
    /* String containing braces */
    const char *msg3 GTY((length("sizeof(\"{test}\")")));
};

/* Template-like macro usage */
#define DECLARE_GTY_STRUCT(name, size) \
    struct name##_t GTY((user)) { \
        int buffer[size]; \
        int count; \
    }

/* Instantiate with parentheses */
DECLARE_GTY_STRUCT(my_buffer, (16 + 4));

/* Function type with array parameters */
typedef int (array_processor GTY((callback)))(
    int input[],
    int output[ (10) ],
    int (*callback)(int, int)
);

/* Structure with bitfield and array */
struct bitfield_array GTY((user)) {
    unsigned int flags : (3);
    unsigned char bytes[ (sizeof(int)) ];
};

/* Union with nested anonymous struct */
union complex_union GTY((user)) {
    struct {
        int type;
        union {
            int i;
            float f;
        } value;
    } data;
    
    long long raw[ (2) ];
};

/* Pointer to function returning pointer to array */
typedef int (*(*complex_func_ptr GTY((callback)))(void))[ (8) ];

/* Final structure combining all delimiter types */
struct all_delimiters GTY((
    user,
    if("(defined(COMPLEX) && (COMPLEX == 1))"),
    skip(("skip_ptr"))
)) {
    /* Parentheses in array size */
    int paren_array[ (2 + 3) * (4 - 1) ];
    
    /* Brackets in type declaration */
    int (*bracket_ptr)[ (5) ];
    
    /* Braces in nested structure */
    struct {
        int x;
        int y;
    } point;
    
    /* Mixed in function pointer */
    void (*callback)(int [ (10) ], struct { int a; int b; });
};

#endif /* TEST_PARSE_H */
