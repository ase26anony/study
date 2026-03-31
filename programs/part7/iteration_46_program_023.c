/* simple.h - Basic delimiter patterns to trigger switch cases */

/* Default case triggers: keywords, identifiers, operators */
#define DEFAULT_CASE 1
typedef int my_int;
static volatile const unsigned long counter;

/* Case '(' - Function prototypes */
extern void simple_func(int arg);
double calculate_sum(double a, double b);
char* process_string(const char* input);

/* Case '[' - Array declarations */
int numbers[100];
extern char buffer[256];
float matrix[10][20];

/* Case '{' - Structure definitions */
struct Point {
    int x;
    int y;
};

union Data {
    int i;
    float f;
    char str[20];
};
