But the clobber list includes 64-bit registers (`rax`, `rbx`, etc.). If you're on x86-64, you should use `movq` for 64-bit or `movl` for 32-bit consistently.

### 2. **Incorrect clobber list**
