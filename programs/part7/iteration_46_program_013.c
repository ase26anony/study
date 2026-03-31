/* test_suite.h - Comprehensive test for gengtype parser delimiter handling */

/* DEFAULT CASE: Keywords, identifiers, operators */
static const volatile int global_counter = 42;
extern unsigned long *pointer_var;
typedef char byte_type;

/* CASE '(': Function prototypes */
void simple_func(void);
int calculate_sum(int a, int b);
char *allocate_memory(size_t size);
double complex_math(double x, double (*transform)(double));

/* CASE '[': Array declarations */
int simple_array[10];
float matrix[3][4];
extern char *string_array[];
const int lookup_table[256] = {0};

/* CASE '{': Structure/union definitions */
struct SimpleStruct {
    int id;
    char name[32];
};

union DataUnion {
    int int_val;
    float float_val;
    char str_val[16];
};

/* NESTED DELIMITERS: Complex patterns */
struct ComplexType {
    /* Nested struct with array */
    struct Inner {
        int values[5];
        void (*callback)(int);
    } inner;
    
    /* Array of function pointers */
    int (*operations[10])(int, int);
    
    /* Function returning pointer to array */
    char (*get_buffer(void))[256];
};

/* Mixed nested delimiters */
typedef int (*CallbackArray[5])(char *str, int options);
struct Container {
    CallbackArray callbacks;
    struct {
        int (*compare)(const void *, const void *);
    } sorter;
};

/* Function pointer with complex signature */
void (*signal_handler(int sig, void (*handler)(int)))(int);

/* Array of structs containing function pointers */
struct Operation {
    const char *name;
    int (*execute)(void *context);
    void (*cleanup)(void *);
} operations[] = {
    {"init", NULL, NULL},
    {"run", NULL, NULL},
    {"exit", NULL, NULL}
};

/* Deeply nested example */
int (*(*complex_nested[3])(void))[5];

/* Edge case: Empty parameter list */
int (*get_default_func(void))(void);

/* String containing delimiter characters (should be skipped) */
const char *message = "Text with (parentheses) and [brackets] and {braces}";

/* Comment with delimiters - should be ignored */
/* This is a (test) comment [with] {delimiters} */

/* Macro definitions with delimiters */
#define MAX_ITEMS(x) ((x) > 100 ? 100 : (x))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Final complex declaration mixing all delimiters */
struct Result {
    int status;
    union {
        int error_code;
        char *message;
    } details;
} (*api_functions[10])(const char *input, int (*validator)(const char *));
