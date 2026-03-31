/* Test Header 1: Basic delimiter cases */
#ifndef TEST_SUITE_H
#define TEST_SUITE_H

/* ============================================
   DEFAULT CASE COVERAGE (lines 341-342)
   ============================================ */

/* Keywords, identifiers, operators, punctuation */
static const volatile int global_counter = 42;
extern unsigned long *pointer;
typedef char byte;
#define MAX_SIZE 100
enum Color { RED, GREEN, BLUE };

/* ============================================
   '(' CASE COVERAGE (lines 343-345)
   ============================================ */

/* Simple function prototypes */
void simple_func(int arg);
double calculate_sum(float a, float b, float c);
char* allocate_string(size_t length);

/* Function with empty parameter list */
int get_value(void);

/* Complex function declarations */
int (*signal_handler(int sig, void (*handler)(int)))(int);
void (*array_of_funcs[5])(int, char*);

/* ============================================
   '[' CASE COVERAGE (lines 346-348)
   ============================================ */

/* Simple array declarations */
extern int numbers[10];
float matrix[3][4];
char message_buffer[256];

/* Array with function pointers */
int (*operation_handlers[8])(void*, size_t);

/* Multi-dimensional arrays */
double tensor[2][3][4][5];

/* ============================================
   '{' CASE COVERAGE (lines 349-351)
   ============================================ */

/* Simple structure */
struct Point {
    int x;
    int y;
    int z;
};

/* Nested structures */
struct Employee {
    char name[50];
    struct Address {
        char street[100];
        char city[50];
        int zipcode;
    } address;
    struct Department* dept;
};

/* Union */
union Data {
    int i;
    float f;
    char str[20];
};

/* ============================================
   NESTED DELIMITER COMBINATIONS
   ============================================ */

/* Function pointer returning pointer to array */
int (*(*complex_func)(int))[10];

/* Array of function pointers returning structs */
struct Result {
    int status;
    char* message;
};

struct Result (*api_functions[3])(int param, void* context);

/* Deeply nested structure with arrays and function pointers */
struct Outer {
    struct Middle {
        struct Inner {
            int (*compare)(const char*, const char*);
            void* data[5];
        } inner_array[3];
        union Choice {
            int (*int_func)(int);
            float (*float_func)(float);
        } choice;
    } middle;
    char (*string_ops[2])(char*, int);
};

/* Function with complex parameter */
void register_callback(
    int (*callback)(struct Outer*, int[][5]),
    const char* name
);

/* ============================================
   EDGE CASES AND STRESS TESTS
   ============================================ */

/* Empty delimiters */
typedef void (*EmptyFunc)();
struct EmptyStruct {};

/* Strings containing delimiter characters */
const char* example_strings[] = {
    "Text with (parentheses) inside",
    "Array[index] notation",
    "Struct{member} style",
    "Mixed: func(arr[i]) { return ptr->field; }"
};

/* Function returning function pointer */
int (*(*get_operation(int opcode))(int, int))(void);

/* Type definition with all delimiters */
typedef struct {
    int (*methods[5])(char* (*formatter)(int), int);
    union {
        int* array_ptr[10];
        void (*func_ptr)(void);
    } variant;
} ComplexType;

/* Macro with delimiters */
#define CREATE_POINTER(type) ((type*)malloc(sizeof(type)))
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

/* Inline function-like macro */
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* Conditional compilation with delimiters */
#ifdef DEBUG
    #define LOG(msg) fprintf(stderr, "[DEBUG] %s (line %d)\n", msg, __LINE__)
#else
    #define LOG(msg) ((void)0)
#endif

/* ============================================
   FINAL COMPREHENSIVE EXAMPLE
   ============================================ */

/* This declaration uses all delimiter types in one statement */
struct Container* (*factory_methods[5])(
    int count,
    struct Item items[],
    void (*cleanup)(struct Container*)
) = {
    create_int_container,
    create_string_container,
    create_array_container
};

#endif /* TEST_SUITE_H */
