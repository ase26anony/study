/* Basic cases for each delimiter type */
#ifndef TEST_SIMPLE_H
#define TEST_SIMPLE_H

/* Trigger '(' case - function prototype */
void simple_function(int arg);

/* Trigger '[' case - array declaration */
extern int simple_array[10];

/* Trigger '{' case - structure definition */
struct SimpleStruct {
    int member1;
    char member2;
};

/* Trigger default case - keywords and identifiers */
static const volatile unsigned long counter = 0;

#endif /* TEST_SIMPLE_H */
