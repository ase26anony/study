/* simple.h - Basic delimiter cases for coverage */

/* Default case triggers: identifiers, keywords, operators */
int global_counter;
static const volatile float PI = 3.14159;

/* '(' case: function prototypes */
void simple_function(int arg);
char* process_string(const char* input, size_t length);
int calculate(int x, int y, int z);

/* '[' case: array declarations */
extern int numbers[100];
float matrix[3][4];
char message_buffer[256];

/* '{' case: structure definitions */
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

/* Mixed: struct with array member */
struct Collection {
    int ids[50];
    struct Point points[10];
};
