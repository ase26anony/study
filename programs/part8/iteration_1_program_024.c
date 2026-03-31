/* gengtype-coverage-test.c - Comprehensive test for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Simplified parser stub for testing if actual gengtype is not available */
#ifdef TEST_STUB
void advance(void) { /* Simulated token advance */ }
void consume_balanced(char open, char close) {
    static int depth = 0;
    depth++;
    if (depth > 100) depth = 0; /* Prevent infinite recursion */
}
#else
/* These would be linked from actual gengtype in real test environment */
#endif

/* Complex .gt content targeting all three consume_balanced cases */
const char* generate_complex_gt_content(void) {
    return 
    "/* Test file for gengtype parser coverage - targeting lines 341-352 */\n"
    "/* Balanced parentheses in various contexts */\n"
    "\n"
    "/* 1. Function pointer types - triggers '(' case */\n"
    "%typedef int (*comparator_t)(const void*, const void*);\n"
    "%typedef void (*complex_func)(int (*nested)(double), char);\n"
    "%typedef ( *deep_nested_funcptr )( int (*(*array_of_funcs[5]))(float) );\n"
    "\n"
    "/* 2. Array types with nested dimensions - triggers '[' case */\n"
    "%struct Matrix {\n"
    "    double data[10][20][30];  /* Triple nested arrays */\n"
    "    int* ptr_array[5][(2+3)*4];  /* Expression in dimension */\n"
    "};\n"
    "\n"
    "/* 3. Complex struct/union definitions - triggers '{' case */\n"
    "%union DeepNestedUnion {\n"
    "    struct {\n"
    "        int x;\n"
    "        struct { char a; double b; } inner;\n"
    "    } s;\n"
    "    int arr[5][(2+3)];\n"
    "    void* ptr;\n"
    "};\n"
    "\n"
    "/* 4. Interleaving comments with balanced delimiters */\n"
    "%struct CommentTest {\n"
    "    int normal;  /* Simple field */\n"
    "    /* Nested comment with { braces } inside */\n"
    "    char array[10];  /* Array with // line comment */\n"
    "    // Line comment with (parentheses)\n"
    "    void (*func)(int);\n"
    "};\n"
    "\n"
    "/* 5. Macro expansions with balanced delimiters */\n"
    "#define ARRAY_TYPE(T) struct { T data[10]; }\n"
    "#define FUNCTION_PTR(RET, ARGS) RET (*)(ARGS)\n"
    "#define NESTED_STRUCT(T) struct { union { T val; }; }\n"
    "\n"
    "ARRAY_TYPE(int) int_array_struct;\n"
    "%typedef FUNCTION_PTR(int, (int, char*)) complex_funcptr_t;\n"
    "NESTED_STRUCT(double) nested_double;\n"
    "\n"
    "/* 6. GC root declarations with complex types */\n"
    "GC roots {\n"
    "    struct Matrix *matrix_root;\n"
    "    %union DeepNestedUnion *union_root[(2+3)*2];\n"
    "    void (**func_root_array[5])(int, int);\n"
    "}\n"
    "\n"
    "/* 7. Enum with complex initializers */\n"
    "%enum ComplexEnum {\n"
    "    VAL1 = (1 << 0),\n"
    "    VAL2 = (1 << 1) | (1 << 2),\n"
    "    VAL3 = sizeof(struct { int x; double y; })\n"
    "};\n"
    "\n"
    "/* 8. Multiple levels of nesting */\n"
    "%struct UltimateTest {\n"
    "    /* Parentheses in function pointer array */\n"
    "    int (*(*func_table[10]))(double);\n"
    "    \n"
    "    /* Array of structs with nested union */\n"
    "    struct {\n"
    "        union {\n"
    "            int i;\n"
    "            char str[20];\n"
    "        } data;\n"
    "    } entries[100];\n"
    "    \n"
    "    /* Pointer to array of pointers to functions */\n"
    "    void (*(**dynamic_funcs)[5])(void);\n"
    "};\n"
    "\n"
    "/* 9. Template-like patterns (not real templates, but similar syntax) */\n"
    "%struct Container {\n"
    "    void* data;\n"
    "    size_t (*get_size)(struct Container*);\n"
    "    void (*free_data)(void*);\n"
    "};\n"
    "\n"
    "/* 10. Edge case: Almost balanced but not quite (for error handling) */\n"
    "%struct UnbalancedTest {\n"
    "    int ok_field;\n"
    "    char bad_array[10;  /* Missing closing bracket */\n"
    "    void (*bad_funcptr(int);  /* Missing closing paren */\n"
    "    struct { int x; ;  /* Missing closing brace */\n"
    "};\n"
    "\n"
    "/* 11. Recovery test: Balanced after unbalanced */\n"
    "%struct RecoveryTest {\n"
    "    int good_array[5][10];  /* This should parse correctly */\n"
    "    void (*good_func)(int, char);\n"
    "};\n"
    "\n"
    "/* 12. Mixed whitespace and newlines in balanced constructs */\n"
    "%typedef int (\n"
    "    *multiline_funcptr\n"
    ") (\n"
    "    int arg1,\n"
    "    char *arg2\n"
    ");\n"
    "\n"
    "/* 13. Empty balanced constructs */\n"
    "%struct EmptyConstructs {\n"
    "    int empty_array[0];\n"
    "    void (*empty_func)(void);\n"
    "    struct {} empty_struct;\n"
    "};\n"
    "\n"
    "/* 14. Preprocessor conditional with balanced delimiters */\n"
    "#ifdef SPECIAL_FEATURE\n"
    "%struct Special {\n"
    "    int special_array[(SPECIAL_SIZE)];\n"
    "    void (*special_func)(struct { int x; });\n"
    "};\n"
    "#endif\n"
    "\n"
    "/* 15. Final complex type combining everything */\n"
    "%typedef struct {\n"
    "    /* All three delimiter types in one type */\n"
    "    int (*(*function_matrix[5][5]))(double matrix[3][3]);\n"
    "    \n"
    "    union {\n"
    "        struct {\n"
    "            char *strings[10];\n"
    "            void (*handlers[5])(int);\n"
    "        } data;\n"
    "        long long raw[20];\n"
    "    } storage;\n"
    "    \n"
    "    /* Expression with parentheses in array dimension */\n"
    "    float computed[(sizeof(struct { int x; double y; }) + 7) / 8];\n"
    "} UltimateType_t;\n";
}

/* Additional test with specific edge cases */
const char* generate_edge_case_gt_content(void) {
    return
    "/* Edge case test file */\n"
    "\n"
    "/* Test 1: Nested parentheses in macro arguments */\n"
    "#define APPLY(func, arg) func(arg)\n"
    "#define NESTED_CALL(a,b) a(b)\n"
    "%typedef int (*callback)(APPLY(NESTED_CALL, (int (*)(double))));\n"
    "\n"
    "/* Test 2: Array dimensions with complex expressions */\n"
    "%struct ExprArray {\n"
    "    int a[ (1 + 2) * 3 ];\n"
    "    char b[ sizeof(int[5]) ];\n"
    "    void* c[ (int)(3.14 * 10) ];\n"
    "};\n"
    "\n"
    "/* Test 3: Function returning pointer to array */\n"
    "%typedef int (*func_ret_array(void))[10];\n"
    "\n"
    "/* Test 4: Pointer to array of function pointers */\n"
    "%typedef void (*(*ptr_to_func_array)[5])(int);\n"
    "\n"
    "/* Test 5: Anonymous struct/union in typedef */\n"
    "%typedef struct {\n"
    "    union {\n"
    "        int i;\n"
    "        float f;\n"
    "    };\n"
    "    struct {\n"
    "        char c;\n"
    "        double d;\n"
    "    };\n"
    "} AnonymousStruct;\n"
    "\n"
    "/* Test 6: Multiple pointer stars with parentheses */\n"
    "%typedef int (*(***(*complex_ptr))(void))(double);\n"
    "\n"
    "/* Test 7: GCC attributes with balanced parentheses */\n"
    "%struct Attributed {\n"
    "    int x __attribute__((aligned(16)));\n"
    "    char y[10] __attribute__((packed));\n"
    "};\n"
    "\n"
    "/* Test 8: Bitfields (use braces in expressions) */\n"
    "%struct BitfieldTest {\n"
    "    unsigned int flags : 3;\n"
    "    signed int value : 5;\n"
    "};\n"
    "\n"
    "/* Test 9: String literals with delimiters (should be ignored) */\n"
    "%struct StringTest {\n"
    "    char* msg1;  /* Contains { but not a delimiter */\n"
    "    char* msg2;  /* Contains ( parentheses ) */\n"
    "};\n"
    "\n"
    "/* Test 10: Character constants with delimiters */\n"
    "%struct CharTest {\n"
    "    char bracket = '}';\n"
    "    char paren = ')';\n"
    "    char square = ']';\n"
    "};\n";
}

/* Write content to temporary file */
char* write_temp_file(const char* content, const char* prefix) {
    char template[256];
    snprintf(template, sizeof(template), "/tmp/%s_XXXXXX.gt", prefix);
    
    int fd = mkstemps(template, 3);  /* .gt is 3 chars */
    if (fd == -1) {
        perror("mkstemps failed");
        return NULL;
    }
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        perror("fdopen failed");
        close(fd);
        return NULL;
    }
    
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return strdup(template);
}

/* Simulate parser execution for coverage testing */
void simulate_parser(const char* content) {
    /* This simulates the tokenization loop that would call consume_balanced */
    const char* p = content;
    int in_comment = 0;
    int in_line_comment = 0;
    int in_string = 0;
    int in_char = 0;
    char prev = '\0';
    
    while (*p) {
        if (!in_comment && !in_line_comment && !in_string && !in_char) {
            switch (*p) {
                case '(':
                    /* In real gengtype, this would call consume_balanced('(', ')') */
                    printf("Found '(' at position %ld\n", (long)(p - content));
                    break;
                case '[':
                    /* Would call consume_balanced('[', ']') */
                    printf("Found '[' at position %ld\n", (long)(p - content));
                    break;
                case '{':
                    /* Would call consume_balanced('{', '}') */
                    printf("Found '{' at position %ld\n", (long)(p - content));
                    break;
                case '/':
                    if (p[1] == '*') {
                        in_comment = 1;
                        p++;
                    } else if (p[1] == '/') {
                        in_line_comment = 1;
                        p++;
                    }
                    break;
                case '"':
                    in_string = !in_string;
                    break;
                case '\'':
                    in_char = !in_char;
                    break;
            }
        } else if (in_comment) {
            if (prev == '*' && *p == '/') {
                in_comment = 0;
            }
        } else if (in_line_comment) {
            if (*p == '\n') {
                in_line_comment = 0;
            }
        } else if (in_string) {
            if (*p == '"' && prev != '\\') {
                in_string = 0;
            }
        } else if (in_char) {
            if (*p == '\'' && prev != '\\') {
                in_char = 0;
            }
        }
        
        prev = *p;
        p++;
    }
}

int main(void) {
    printf("=== GCC gengtype Parser Coverage Test ===\n\n");
    
    /* Generate and write test files */
    const char* complex_content = generate_complex_gt_content();
    const char* edge_content = generate_edge_case_gt_content();
    
    char* complex_file = write_temp_file(complex_content, "gengtype_complex");
    char* edge_file = write_temp_file(edge_content, "gengtype_edge");
    
    if (!complex_file || !edge_file) {
        fprintf(stderr, "Failed to create temporary files\n");
        return 1;
    }
    
    printf("Generated test files:\n");
    printf("1. %s (complex balanced constructs)\n", complex_file);
    printf("2. %s (edge cases)\n\n", edge_file);
    
    /* Simulate parser execution (for stub testing) */
    printf("Simulating parser on complex content...\n");
    simulate_parser(complex_content);
    
    printf("\nSimulating parser on edge content...\n");
    simulate_parser(edge_content);
    
    /* In a real test environment, we would invoke gengtype like:
     *   system("./gengtype -p %s", complex_file);
     *   system("./gengtype -p %s", edge_file);
     */
    
    printf("\n=== Test Complete ===\n");
    printf("To run actual gengtype coverage test:\n");
    printf("  g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("      -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("  ./gengtype-instr -p %s\n", complex_file);
    printf("  gcov gengtype-parse.cc\n");
    
    /* Cleanup */
    unlink(complex_file);
    unlink(edge_file);
    free(complex_file);
    free(edge_file);
    
    return 0;
}
