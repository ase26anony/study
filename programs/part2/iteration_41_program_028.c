Device (GPU/Accelerator)
├── Team 0 → j = 0..k
│   └── SIMD lanes → i = 0..M (vectorized)
├── Team 1 → j = k+1..l
│   └── SIMD lanes → i = 0..M (vectorized)
└── ... more teams
