/* Basic delimiter cases to trigger all switch branches */
#ifndef SIMPLE_H
#define SIMPLE_H

/* Trigger '(' case - function prototype */
void simple_function(int param);

/* Trigger '[' case - array declaration */
extern int simple_array[10];

/* Trigger '{' case - structure definition */
struct SimpleStruct {
    int member1;
    char member2;
};

/* Trigger default case - keywords and identifiers */
static const volatile unsigned long counter = 0UL;

/* Mixed delimiters in one declaration */
struct Mixed {
    int (*func_ptr)(int);
    char buffer[256];
};

#endif /* SIMPLE_H */
