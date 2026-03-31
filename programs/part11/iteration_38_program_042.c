/* test_parse.h - Complex GTY annotations to test delimiter handling */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define NESTED_BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ( (x) + ((x) * 2) )

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
    struct s1 *next GTY((skip));
};

/* More complex parentheses case with multiple nested groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct s2 *next;
    struct s2 *prev;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY(()) {
    int arr1 GTY((length("N")))[ (2 * 3) ];
    int arr2 GTY((length("M")))[ sizeof(int) * 2 ];
    int (*ptr_arr GTY((length("P"))))[ (4 + 1) ];
};

/* Nested brackets in typedef */
typedef int matrix_type GTY((length("(rows * cols)")))[10][ (5 * 2) ];

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y[2];
    } nested;
    int count;
};

/* Union with brace-enclosed initializer in macro expansion */
union u1 GTY(()) {
    int i;
    char arr[ 10 + (5) ];
    struct point {
        int x, y;
    } p;
};

/* Function pointer type with complex argument list containing brackets */
typedef void (*complex_fn_ptr GTY((callback)))(
    int (*)(int arr[ (sizeof(int) > 4) ? 8 : 4 ]),
    void (*)(struct s1 *list[ (2 << 3) ])
);

/* GTY with skip containing nested parentheses in expression */
struct s4 GTY((skip("(obj) && ((obj)->value > 0)"))) {
    int value;
    struct s4 *link;
};

/* Array of pointers with nested dimension calculations */
struct s5 GTY(()) {
    int *(*ptr_array GTY((length("(width * height)"))))[ (16 + 8) ];
    int width;
    int height;
};

/* Nested structure with GTY on inner member containing all delimiters */
struct s6 GTY((user)) {
    struct {
        int *data GTY((length("(size + 1)")))[];
        struct {
            int x;
            int y;
        } point;
    } container GTY((tag("NESTED")));
    int size;
};

/* Macro expansion within GTY arguments */
#define LENGTH_EXPR ( (10) + (20) )
#define TAG_VALUE "STRUCT_TAG"

struct s7 GTY((length( "LENGTH_EXPR" ), tag(TAG_VALUE))) {
    char buffer[ LENGTH_EXPR ];
};

/* Complex conditional in array dimension */
struct s8 GTY(()) {
    int dynamic_array GTY((length("count")))[ 
        (sizeof(void*) == 8) ? 64 : 32 
    ];
    int count;
};

/* Multiple nested parentheses in callback specification */
typedef int (*nested_callback GTY((callback)))(
    int (*inner)(int (*)(int [ (4) ])),
    void *context
);

/* Union with GTY and array containing expression with parentheses */
union u2 GTY((desc("tag"))) {
    int tag;
    struct {
        int items GTY((length("len")))[ (MAX_ITEMS) ];
        int len;
    } list;
};

#ifndef MAX_ITEMS
#define MAX_ITEMS ( (16) + (8) )
#endif

/* Structure with embedded anonymous union containing braces */
struct s9 GTY((user)) {
    int type;
    union {
        int num;
        struct {
            char *str;
            int len;
        } text;
    } value;
};

/* Final complex case mixing all delimiters */
struct s10 GTY((chain_next("nxt"), chain_prev("prv"))) {
    int id;
    char *name GTY((length("(strlen(name) + 1)")));
    struct s10 *nxt;
    struct s10 *prv;
    int scores GTY((length("(num_scores)")))[ 
        (MAX_SCORES > 0) ? MAX_SCORES : 10 
    ];
    int num_scores;
};

#ifndef MAX_SCORES
#define MAX_SCORES ( (20) + (5) )
#endif

#endif /* TEST_PARSE_H */
