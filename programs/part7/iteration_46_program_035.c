/* Test Header for gengtype-parse.cc coverage
 * Covers all delimiter cases and default character handling
 */

/* DEFAULT CASE: Keywords, identifiers, operators */
#define DEFAULT_CASE 1
static const volatile int global_counter = 0;
typedef unsigned long size_t;
extern char* string_literal;

/* CASE '(': Function prototypes and parameters */
void simple_func(int arg);
int process_data(char *input, size_t len, void *context);
int (*signal_handler(int sig, void (*handler)(int)))(int);
void empty_params(void);

/* CASE '[': Array declarations */
int simple_array[10];
char *string_array[] = {"hello", "world"};
int (*array_of_func_ptrs[5])(void);
int (*complex_array[3])(char *str, int len);

/* CASE '{': Structure and union definitions */
struct SimpleStruct {
    int member1;
    char member2;
};

struct NestedStruct {
    struct Inner {
        int x;
        int y;
    } inner;
    int outer_member;
};

union TestUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

/* MIXED NESTED DELIMITERS */
struct Container {
    int (*operations[3])(struct Container *self, int param);
    void (*cleanup)(void);
    char buffer[256];
};

typedef int (*Comparator)(const void *, const void *);
Comparator sort_funcs[5];

/* FUNCTION POINTERS WITH COMPLEX SIGNATURES */
char *(*name_generator)(int id, char buffer[]);
struct Result *(*result_processor)(int (*input)(void), char data[]);

/* ARRAY OF STRUCTS CONTAINING FUNCTION POINTERS */
struct Handler {
    int (*handle)(char *data, int len);
    void (*cleanup)(void);
    char name[32];
} handlers[10];

/* DEEP NESTING */
int (*(*deep_nested[2])[3])(void);
struct Outer {
    struct Middle {
        struct Inner {
            int matrix[3][3];
            void (*callback)(int, char *);
        } inners[5];
    } middle;
};

/* STRINGS AND COMMENTS WITH DELIMITERS */
char *message = "Text with (parentheses) and [brackets]";
char *path = "/usr/local/include/file.h";
/* Comment with {braces} and (parens) */

/* TYPEDEF WITH FUNCTION POINTER */
typedef int (*BinaryOp)(int a, int b);
BinaryOp operations[] = {NULL, NULL, NULL};

/* CONST ARRAY OF POINTERS TO FUNCTIONS RETURNING POINTERS */
const char *(*const api_table[4])(int, const char**) = {NULL};

/* VARIADIC FUNCTION */
int variadic_func(const char *fmt, ...);

/* BITFIELD STRUCTURE */
struct BitFields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int : 3;  /* Unnamed bitfield */
    unsigned int flag3 : 4;
};

/* ANONYMOUS STRUCT/UNION */
struct Anonymous {
    union {
        int x;
        float y;
    };
    struct {
        char a;
        char b;
    };
};

/* FINAL COMPLEX EXAMPLE COMBINING EVERYTHING */
struct UltimateTest {
    int (*(*get_processor(void))[5])(char *);
    struct {
        int (*handlers[3])(struct UltimateTest *, int (*)(void));
        char data[(*message == 'T') ? 10 : 20];
    } nested;
};
