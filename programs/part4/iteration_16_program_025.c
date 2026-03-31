## Key Points:
1. **Memory Access**: Each iteration accesses `data + i*element_size`
2. **Increment Timing**: The increment happens **after** the memory access in this loop structure
3. **Cache Behavior**: Sequential access is cache-friendly (spatial locality)
4. **Potential Optimization**: Compilers often optimize this to pointer arithmetic:
