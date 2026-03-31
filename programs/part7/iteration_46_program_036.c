/* simple.h - Basic delimiter cases for gengtype parser coverage */

#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case with function prototypes */
void simple_function(int param);
int calculate_sum(int a, int b, int c);
char* get_message(void);

/* Trigger '[' case with array declarations */
extern int numbers[10];
float matrix[3][4];
const char* days_of_week[7];

/* Trigger '{' case with structure definitions */
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

/* Default case triggers - identifiers, keywords, operators */
static const volatile unsigned long counter = 0;
typedef struct Point* PointPtr;
enum Color { RED, GREEN, BLUE };

#endif /* SIMPLE_H */
