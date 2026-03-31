/* Test header for gengtype-parse.cc coverage testing
 * Covers: default case (non-delimiter chars) and all delimiter cases
 */

/* ========== DEFAULT CASE COVERAGE ========== */
/* Keywords, identifiers, operators, punctuation */
static const volatile int global_counter = 42;
extern unsigned long *pointer_var;
typedef char byte_type;
#define MAX_SIZE 100

/* ========== PARENTHESES CASE ========== */
/* Function prototypes */
void simple_func(int arg);
int *allocate_memory(size_t size);
char *process_string(const char *input, int flags);

/* Function pointers */
typedef int (*comparator_t)(const void *, const void *);
void (*signal_handler)(int signum);

/* Complex function pointer */
int (*(*get_callback_array(void))[5])(char *str);

/* Function returning function pointer */
void (*setup(void))(int);

/* ========== BRACKETS CASE ========== */
/* Array declarations */
extern int numbers[10];
float matrix[3][4];
char *string_array[] = {"hello", "world"};

/* Array of pointers */
int *ptr_array[8];

/* Multi-dimensional arrays */
double tensor[2][3][4];

/* Array in struct */
struct ArrayHolder {
    int data[20];
    char buffer[256];
};

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
    struct {
        unsigned count;
        double values[5];
    } anonymous;
};

/* ========== MIXED NESTED PATTERNS ========== */
/* Array of function pointers */
int (*operation[4])(int, int);

/* Function returning array pointer */
int (*get_lookup_table(void))[10];

/* Structure containing function pointer array */
struct CallbackManager {
    void (*callbacks[8])(void *data);
    int priority;
};

/* Complex nested type */
struct ComplexType {
    union {
        struct {
            int (*handlers[3])(char *);
            void (*cleanup)(void);
        } ops;
        struct {
            char *(*formatters[2])(int);
            int counters[4];
        } fmt;
    } u;
    int state;
};

/* Function pointer with complex return type */
struct Result (*processor)(int param, void *context);

/* Array of function pointers returning structs */
struct Data (*transformers[5])(const struct Data *input);

/* ========== EDGE CASES ========== */
/* Empty parentheses */
int (*signal(int sig, void (*handler)(int)))(int);

/* Function with no parameters */
void initialize(void);

/* Zero-length array (GCC extension) */
struct Flexible {
    int length;
    char data[];
};

/* String containing delimiter characters */
const char *message = "Text with (parentheses) and [brackets] and {braces}";

/* Comment with delimiters - should be ignored by parser */
/* This (comment) has [all] {delimiters} */

/* Preprocessor with parentheses */
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Typedef with function pointer */
typedef void (*event_callback_t)(int event_id, void *user_data);

/* Const pointer to array of const pointers to functions */
int (* const (* const complex_array[3])[2])(void);

/* ========== FINAL COMPLEX EXAMPLE ========== */
/* Ultimate test of nested delimiter consumption */
struct UltimateTest {
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*(*func_table[4])(int))[5])(char);
    
    /* Nested anonymous struct with union */
    struct {
        union {
            struct {
                void (*start)(void);
                int (*process[2])(double);
            } phase1;
            struct {
                char (*get_name)(int);
                void (*cleanup[3])(void);
            } phase2;
        } u;
        int step;
    } state_machine;
    
    /* Multi-dimensional array of various types */
    union Data *data_grid[2][3][4];
};
