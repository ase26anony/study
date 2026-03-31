/* test_suite.h - Comprehensive test for gengtype-parse.cc delimiter handling */

/* DEFAULT CASE: Keywords, identifiers, operators, punctuation */
static const volatile int global_counter = 42;
extern unsigned long *pointer_var;
typedef char byte_t;

/* CASE '(': Function prototypes with parameters */
void simple_func(int arg);
double calculate(double x, double y, int precision);
char *allocate_memory(size_t bytes);
int (*signal_handler(int sig, void (*handler)(int)))(int);

/* CASE '[': Array declarations */
int simple_array[10];
extern char *string_array[5];
float multi_dim[3][4][5];
int (*func_ptr_array[8])(void);

/* CASE '{': Structure/union definitions */
struct SimpleStruct {
    int member1;
    char member2;
    float member3;
};

union DataUnion {
    int i;
    float f;
    char str[20];
};

/* NESTED DELIMITERS: Complex patterns */
struct ComplexType {
    /* Nested '{' inside '{' */
    struct Inner {
        int x;
        struct Deeper {
            char c;
        } deeper;
    } inner;
    
    /* '(' and '[' inside '{' */
    int (*callback)(int (*)(char *), void *);
    void *data_ptrs[10];
};

/* Mixed nested delimiters */
typedef int (*(*complex_array[5])(char *str, int len))[10];

/* Function returning pointer to array */
int (*get_matrix(void))[10][20];

/* Array of function pointers returning struct pointers */
struct Result *(*operations[3])(struct Input *);

/* Edge cases with empty delimiters */
void empty_params(void);
int empty_array[];

/* Strings containing delimiter characters */
const char *message1 = "Text with (parentheses) inside";
const char *message2 = "Array[0] = {value}";
const char *message3 = "Mixed: func(arg[0]) { return; }";

/* Complex declaration with all delimiters */
struct Container {
    int (*(*func_ptrs[3])(struct Data *d, int idx))(char *);
    union {
        struct {
            int count;
            char *names[10];
        } s;
        void (*handlers[5])(int, char *);
    } u;
};

/* Preprocessor directives (if processed) */
#define MAX_SIZE 100
#define CREATE_PTR(type) (type *)malloc(sizeof(type))

/* More default case coverage */
enum Color { RED, GREEN, BLUE };
static inline int increment(int *x) { return ++(*x); }

/* Final complex type mixing everything */
typedef struct {
    int (*compare)(const void *, const void *);
    void (*sort)(void *, size_t, size_t, 
                 int (*)(const void *, const void *));
    void *data_array[50];
} SortOperations;
