This program uses multiple techniques to stress GCC's reload pass:

1. **Complex constraints**: Uses alternative constraints (`"=r,m"`, `"r,m,i"`) that force the compiler to consider multiple possibilities
2. **Hard register clobbers**: Explicitly clobbers specific registers to increase register pressure
3. **Nested addresses**: Takes addresses of volatile array elements with complex index expressions
4. **Register variables**: Binds variables to specific registers, then uses them in complex expressions
5. **Multi-operand asm**: Uses 10+ operands to maximize the chance of conflicts
6. **`__builtin_constant_p`**: Creates conditional address expressions
7. **Mixed types**: Uses `__builtin_bit_cast` to treat floats as integers in assembly

**Compilation recommendations**:
