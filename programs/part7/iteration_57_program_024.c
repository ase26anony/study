- `movl` is for 32-bit operations
- But you're clobbering 64-bit registers (`rax`, `rbx`, etc.)

### 2. **Incorrect clobber list**
