But the clobber list includes 64-bit registers (`rax`, `rbx`, etc.). If you're compiling for x86-64, you should use `movq` for 64-bit or keep `movl` but use 32-bit registers.

### 2. **Incorrect clobber list**
