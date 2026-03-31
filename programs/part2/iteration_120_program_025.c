#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into different states
namespace AsmTest {
    template<typename T>
    class Processor {
    public:
        void process() {
            // Almost correct inline assembly - missing template string
            asm volatile ( : : : "memory" );
        }
        
        // Another invalid asm context
        int x = asm;  // 'asm' used as value
    };
}

// Global scope invalid asm
asm ("mov eax, ebx" : : );  // Missing output/input operands

// Macro that expands to incomplete asm
#define BAD_ASM_STMT asm volatile (
void testBadMacro() {
    BAD_ASM_STMT : : : "memory");
}

#endif
