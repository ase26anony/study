/* Test header for gengtype-parse.cc coverage */
/* This file triggers all delimiter cases and the default case */

/* ========== DEFAULT CASE TRIGGERS ========== */
/* Keywords, identifiers, operators, punctuation */
static const volatile int global_counter = 42;
extern unsigned long *pointer_var;
typedef char byte;
#define MAX_SIZE 100
#include <stddef.h>

/* ========== PARENTHESES CASE ========== */
/* Function prototypes */
void simple_func(void);
int calculate_sum(int a, int b);
char *allocate_memory(size_t size);
double (*get_math_func(int op))(double);

/* Function pointer declarations */
typedef int (*comparator_t)(const void *, const void *);
void (*signal_handler)(int sig);
int (*complex_func_array[5])(char *str, int len);

/* Nested parentheses */
int (*(*get_func_ptr(void))(int))(char);
void (*((*get_handler_table(void))[3]))(void);

/* ========== BRACKETS CASE ========== */
/* Array declarations */
extern int simple_array[10];
float matrix[3][4];
char *string_array[] = {"hello", "world", "test"};

/* Complex array declarations */
int (*array_of_func_ptrs[5])(void);
struct Node *linked_list_nodes[100];
void (*signal_handlers[NSIG])(int);

/* Multi-dimensional with pointers */
char *(*(*complex_array[2])[3])[4];

/* ========== BRACES CASE ========== */
/* Structure definitions */
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

/* Union definition */
union DataUnion {
    int i;
    float f;
    char str[20];
};

/* Nested structures */
struct Outer {
    struct Inner {
        int depth;
        char label[16];
    } inner;
    struct Inner *inner_ptr;
    int outer_value;
};

/* Structure with function pointer member */
struct Operations {
    int (*add)(int, int);
    void (*print)(const char *);
    struct Operations *next;
};

/* ========== MIXED DELIMITERS ========== */
/* Deeply nested combinations */
struct Container {
    /* Array of function pointers returning structs */
    struct Result (*operations[3])(void);
    
    /* Function pointer with array parameter */
    void (*processor)(int data[], int size);
    
    /* Nested structure with array */
    struct {
        int (*comparators[5])(int, int);
        char buffer[256];
    } helper;
};

/* Complex type declaration mixing all delimiters */
typedef int (*(*(*complex_type)[5])(struct Params {int x; int y;}))[10];

/* Function with struct parameter returning function pointer */
void (*(*register_callback(struct Callback {void (*func)(void);} cb))(int))();

/* ========== EDGE CASES ========== */
/* Empty delimiters */
int (*signal(int sig, void (*handler)(int)))(int);

/* String containing delimiter characters */
const char *message = "Text with (parentheses) and [brackets] and {braces}";

/* Comment with delimiters - should be ignored by parser */
/* This (comment) has [all] {delimiters} */

/* Function-like macro with parentheses */
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* Pointer to array */
int (*ptr_to_array)[10];

/* Array of pointers to functions returning pointers to arrays */
int (*(*(*func_table[3])(void))[5])(void);

/* Final complex example combining everything */
struct MasterType {
    /* Member function pointer */
    int (*(*member_func)(struct Args {int x; int arr[5];} args))[10];
    
    /* Array of structures containing function pointers */
    struct {
        void (*start)(void);
        int (*process)(int data[], int len);
        void (*end)(char *result);
    } handlers[4];
    
    /* Union with array */
    union {
        int numbers[20];
        struct {
            char *(*get_name)(void);
            int id;
        } info;
    } data;
};
