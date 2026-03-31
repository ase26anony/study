/* test_suite.h - Comprehensive test for gengtype-parse.cc delimiter handling */

/* DEFAULT CASE COVERAGE: Keywords, identifiers, operators */
#define DEFAULT_TEST 1
static const volatile int global_counter = 0;
extern unsigned long long big_number;
typedef char byte;

/* CASE '(': Function prototypes and parameters */
void simple_func(void);
int calculate_sum(int a, int b, int c);
char* process_string(const char* input, size_t length);
double (*get_calculator(void))(double, double);
int (*signal_handler(int sig, void (*handler)(int)))(int);

/* CASE '[': Array declarations */
int simple_array[10];
extern char* string_array[];
const float matrix[3][4];
int (*function_ptr_array[5])(void);
struct Node* adjacency_list[100];

/* CASE '{': Structure/union definitions */
struct Empty {};
struct Point { int x; int y; };
union Data { int i; float f; char str[20]; };

/* NESTED DELIMITERS - Complex patterns */
struct Complex {
    int (*comparator)(const void*, const void*);
    void (*handlers[10])(struct Complex*);
    struct {
        int depth;
        struct Nested* next;
    } inner;
};

/* Mixed nested delimiters */
typedef int (*Callback)(char buffer[256], int (*validator)(char));
struct Container {
    Callback callbacks[5];
    struct {
        int (*transform)(int matrix[3][3]);
    } operations;
};

/* Deep nesting stress test */
int (*(*deep_nested[2])(void))[3];
struct Outer {
    struct Middle {
        struct Inner {
            int values[5];
            void (*method)(struct Inner*);
        } item;
        struct Inner* array[10];
    } middle;
};

/* Function pointers with complex signatures */
void (*event_handlers[])(int, char* []) = { NULL };
int (*(*get_router(void))(int))[5];

/* Edge cases with empty delimiters */
void empty_params(void);
int empty_array[];
struct EmptyStruct {};

/* Strings and comments containing delimiters */
char* message = "Text (with parentheses) [and brackets] {and braces}";
/* Comment with delimiters: int test_array[(10+5)*2] */

/* More default case coverage */
enum Color { RED, GREEN, BLUE };
static inline int min(int a, int b) { return a < b ? a : b; }

/* Preprocessor (if processed) */
#ifdef DEFAULT_TEST
    #define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

/* Final complex declaration mixing all delimiters */
struct FinalTest {
    int (*(*complex[3])(struct { int x; int y; }))[2];
    void (*initialize)(int params[2], void (*callback)(void));
    union {
        int (*as_int)(int);
        float (*as_float)(float);
    } converter;
};
