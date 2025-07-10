# 🧃 Beere Programming Language

**Beere** is a lightweight, low-level programming language focused on simplicity, performance, and full control over memory and execution.  
It’s designed for developers who want the power of C with a more modern and clean syntax, without sacrificing control or transparency.

---

## ✨ Features

- ⚙️ **Compiled to x86_64 Assembly**
- 🔒 **Manual memory management**, with optional **Garbage Collector** (planned)
- 💡 **Static typing**, no dynamic typing or inference by assignment
- 🧱 **Module system** with strict encapsulation and no circular access
- 🧮 Built-in types:
  - `int`, `float`, `double`, `char`, `bool`
  - `ptr`, `any ptr`
- 🔧 **No pointer offset/indexing** (only allowed in arrays)
- 🧠 **Object-oriented support** (in development):
  - Classes, inheritance, virtual functions
  - `static` fields and methods
  - `super` keyword
- 📦 All arrays are **dynamic** and heap-allocated
- ⚡ Compile-time optimizations
- 🧩 Clean and modern syntax
- 🌐 Native C interop:
  ```beere
  extern "C" from "libc.h"
  {
    fn foo(bar: int)
  }
  ```
