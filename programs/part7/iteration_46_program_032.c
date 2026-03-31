/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: keywords, identifiers, operators */
extern int global_counter;
static const volatile long persistent_value;

/* '(' case: Function prototypes */
void simple_function(int param);
double calculate_average(float *data, int count);
char *allocate_string(size_t length);

/* '[' case: Array declarations */
int numbers[100];
extern char buffer[256];
float matrix[10][20];

/* '{' case: Structure definitions */
struct Point {
    int x;
    int y;
    double z;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Mixed: All delimiters in one */
struct Container {
    void (*callback)(int event);
    int items[50];
};
