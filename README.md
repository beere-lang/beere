# Beere Programming Language
Beere is a compiled programming language designed with a more modern syntax. The Beere compiler is implemented in C and generates assembly code.

## 🚀 Features
Modern syntax inspired by classic and modern languages

- Compiled to assembly.
- Compiler written in C.
- Package manager.

## 🔧 Status
This project is in early development.

## 📄 Syntax
```rust
class SuperClass
{
    pub let name: string
}

class Clazz extends SuperClass
{
    pub let age: int = 0
    
    pub Clazz(name: string, age: int)
    {
        super(name)
        this->age = age
    }
}

fn main()
{
    let clazz: Clazz* = new Clazz("Name", 20)
}
```
