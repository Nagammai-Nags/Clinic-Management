# Clinic Management App

## Setup

1. Install Python 3.
2. Install a C compiler and make sure it is available on `PATH`.
   - Windows: install MinGW-w64 or LLVM/Clang.
   - macOS: install Xcode Command Line Tools.
   - Linux: install `gcc` or `clang` from your package manager.
3. Install Python dependencies:

```bash
pip install -r requirements.txt
```

4. Start the Flask app:

```bash
python app.py
```

The C backend is compiled automatically from `backend/clinic_backend.c` when the app starts. If your compiler has a custom location or name, set `CLINIC_BACKEND_CC` to that compiler executable before running the app.
