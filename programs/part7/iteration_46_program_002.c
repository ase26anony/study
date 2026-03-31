/* simple.h - Basic delimiter cases for coverage */

/* Default case triggers: keywords, identifiers, operators */
typedef unsigned long int size_t;
extern volatile int global_counter;
static const char *default_message = "test";

/* Parentheses case: function prototypes */
void simple_func(int param);
int calculate_sum(int a, int b, int c);
char *allocate_memory(size_t size);

/* Brackets case: array declarations */
extern int numbers[100];
float matrix[3][4];
char *string_array[] = {"hello", "world", "test"};

/* Braces case: structure definitions */
struct Point {
    int x;
    int y;
    int z;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Mixed: all delimiters in one line */
struct Container {
    int (*compare)(const void *, const void *);
    void *items[50];
};
