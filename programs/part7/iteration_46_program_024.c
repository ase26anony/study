/* Test Header for gengtype-parse.cc coverage
 * Covers: default case (advance()) and all delimiter cases
 */

/* ========== DEFAULT CASE TRIGGERS ========== */
/* Keywords, identifiers, operators, punctuation */
static const volatile int global_counter = 42;
extern unsigned long *pointer;
typedef char byte;
#define MAX_SIZE 100

/* ========== PARENTHESES CASE ========== */
/* Function prototypes */
void simple_func(void);
int calculate(int a, int b);
char *allocate_memory(size_t size);

/* Function pointers */
int (*func_ptr)(int, char);
void (*signal_handler)(int signum);

/* Complex function pointer */
int (*(*complex_fp[5])(char *))(double);

/* K&R style function declaration */
int old_style_func();

/* ========== BRACKETS CASE ========== */
/* Array declarations */
int simple_array[10];
char *string_array[20];
const float matrix[3][4];

/* Array of pointers */
int *ptr_array[5];

/* Complex array declarations */
int (*array_of_func_ptrs[3])(void);
struct Node *graph[100];

/* ========== BRACES CASE ========== */
/* Structure definitions */
struct Simple {
    int id;
    char name[50];
};

/* Union definition */
union Data {
    int i;
    float f;
    char str[20];
};

/* Nested structures */
struct Outer {
    struct Inner {
        int x;
        int y;
    } inner;
    int outer_val;
};

/* ========== MIXED NESTED PATTERNS ========== */
/* Array of function pointers returning structs */
struct Result (*operations[3])(int param);

/* Function returning pointer to array */
int (*get_matrix(void))[4][4];

/* Structure containing function pointer array */
struct Callbacks {
    void (*handlers[10])(void *data);
    int priorities[10];
};

/* Complex type with all delimiters */
struct Container {
    int (*comparator)(const void *, const void *);
    void *items[100];
    struct {
        int count;
        int capacity;
    } metadata;
};

/* Function with complex return type */
struct Result *(*factory_method(int type))(void);

/* ========== EDGE CASES ========== */
/* Empty parameter list */
int (*no_args_func(void))(void);

/* String containing delimiters */
char *message = "Test (with [nested] {delimiters}) in string";

/* Comment with delimiters - should be ignored */
/* This (comment [has] {delimiters}) but they don't count */

/* Pointer to array of function pointers */
int (*(*ptr_to_array)[5])(char);

/* Typedef with complex type */
typedef int (*Comparator)(const void *, const void *);

/* Anonymous struct in union */
union Value {
    struct {
        int type;
        void *data;
    };
    long long as_int;
};

/* ========== FINAL COMPLEX EXAMPLE ========== */
/* Everything combined */
struct MasterType {
    /* Default case elements */
    static const unsigned int flags;
    
    /* Parentheses */
    void (*init)(struct MasterType *self, int options[5]);
    
    /* Brackets */
    void *buffer_pool[20];
    
    /* Braces */
    struct {
        int ref_count;
        pthread_mutex_t lock;
    } control;
    
    /* Mixed */
    int (*(*dispatch_table[10])(struct MasterType *))(int, ...);
};

/* Function using all delimiters */
struct MasterType *create_master(void (*callback)(int result[3])) {
    /* Function body would be in .c file */
    return 0;
}
