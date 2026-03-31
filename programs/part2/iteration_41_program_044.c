Device (e.g., GPU)
├── Thread Teams (created by `teams`)
│   ├── Team 0: processes j = 0..k
│   ├── Team 1: processes j = k+1..l
│   └── ...
└── Within each team:
    ├── Threads execute SIMD operations
    └── Vector length ≤ 16 (due to safelen)
