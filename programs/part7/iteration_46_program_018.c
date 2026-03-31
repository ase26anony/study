/* Test suite for gengtype-parse.cc delimiter coverage */
#ifndef TEST_SUITE_H
#define TEST_SUITE_H

/* ========== SIMPLE CASES (Basic delimiter triggers) ========== */

/* Triggers '(' case - function prototype */
void simple_func(int arg);

/* Triggers '[' case - array declaration */
extern int simple_array[10];

/* Triggers '{' case - structure definition */
struct SimpleStruct {
    int member;
    char *name;
};

/* ========== NESTED DELIMITERS ========== */

/* Nested parentheses in function pointer */
int (*func_ptr)(int (*callback)(void), char *args[]);

/* Array of function pointers with nested brackets */
void (*signal_handlers[5])(int sig, void *context);

/* Nested structures */
struct Outer {
    struct Middle {
        struct Inner {
            int deepest;
        } inner;
        float middle_val;
    } mid;
    char outer_tag[20];
};

/* Complex nested combination */
struct ComplexType {
    int (*comparator)(const void *, const void *);
    void *data_pool[100];
    struct {
        unsigned int flags;
        char id[32];
    } metadata;
};

/* ========== MIXED DELIMITERS ========== */

/* Function returning pointer to array */
int (*get_matrix(void))[10][10];

/* Array of pointers to functions returning structs */
struct Result (*operations[8])(int param, void *ctx);

/* Deeply nested with all delimiter types */
void (*(*complex_array[3])(char *))[5](int, float);

/* ========== STRESS DEFAULT CASE ========== */

/* Keywords, identifiers, operators, punctuation */
static const volatile unsigned long counter = 0UL;
extern char *global_string;
typedef struct Node *NodePtr;
enum Color { RED, GREEN, BLUE = 255 };

/* Multiple operators and punctuation */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Complex declaration with many non-delimiter chars */
static const struct Config {
    int verbose_level;
    char *log_file;
    unsigned timeout_ms;
} default_config = { 1, "output.log", 5000 };

/* ========== EDGE CASES ========== */

/* Empty parentheses */
int (*signal_handler(int sig, void (*handler)(int)))(int);

/* String containing delimiter characters */
char *message = "Text (with parentheses) [and brackets] {and braces}";

/* Function with no parameters */
void no_params(void);

/* Zero-length array (GNU extension) */
struct Header {
    int length;
    char data[0];
};

/* Multi-dimensional arrays */
int matrix[3][4][5];

/* Pointer to array of pointers to functions */
char *(*(*string_processor)[10])(const char *);

/* ========== FINAL COMPREHENSIVE TEST ========== */

/* Ultimate test combining everything */
typedef union UltimateUnion {
    struct {
        int (*methods[10])(union UltimateUnion *self);
        char name[50];
    } vtable;
    struct {
        void *data;
        size_t size;
        unsigned ref_count;
    } buffer;
    struct {
        long double complex_num;
        _Complex double arr[4];
    } math;
} UltimateUnion;

#endif /* TEST_SUITE_H */
