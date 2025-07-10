# Beere Programming Language

**Beere** is a lightweight, low-level programming language focused on simplicity, performance, and full control over memory and execution.  
It’s designed for developers who want the power of C with a more modern and clean syntax, without sacrificing control or transparency.

---

## ✨ Features

- ⚙️ **Compiled to Assembly**
- 🔒 **Manual memory management**, with optional **Garbage Collector** (planned)
- 💡 **Static typing**, no dynamic typing or inference by assignment
- 🧱 **Module system** with strict encapsulation and no circular access
- 🧠 **Object-oriented support**
- 📦 **Dynamic** and heap allocated arrays
- ⚡ Compile-time **optimizations**
- 🧩 Clean and **modern** syntax
- 🧃 **Package Manager** system
- 🌐 Native C interop:
  ```beere
  extern "C" from "libc.h"
  {
    fn foo(bar: int)
  }
  ```
