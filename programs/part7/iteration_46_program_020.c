/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: identifiers, keywords, operators */
int simple_variable;
static const volatile char *string_literal = "test";

/* '(' case: function prototypes */
void simple_function(int param);
int calculate_sum(int a, int b, int c);

/* '[' case: array declarations */
extern int numbers[10];
float matrix[3][4];

/* '{' case: structure definitions */
struct Point {
    int x;
    int y;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Mixed simple cases */
enum Color { RED, GREEN, BLUE };
typedef unsigned long ulong;
