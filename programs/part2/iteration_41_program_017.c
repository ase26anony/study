Device (e.g., GPU)
├── Team 0
│   ├── Threads execute j = 0..k
│   │   └── SIMD vectorized i-loop (vector length ≤ 16)
├── Team 1
│   ├── Threads execute j = k+1..m
│   │   └── SIMD vectorized i-loop
└── ...
