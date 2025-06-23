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
import "std"

class Sum
{
    priv n1: int = 0
    n2: int = 0 // private by default...

    constructor(n1: int, n2: int)
    {
        this->n1 = n1
        this->n2 = n2
    }

    pub fn sum(): int
    {
        return this->n1 + this->n2
    }
}

fn sum(n1: int, n2: int): int
{
    const scoped sum_object: Sum* = new Sum(n1, n2) // scoped: automatically frees when it goes out of scope...
    const sum = sum_object->sum()

    return sum
}

fn ptr_test(ptr: int*, value: int) // void by default...
{
    *ptr = value
}

fn main(): int
{
    let sum: int = sum(10, 20)

    if (sum != 10 + 20)
    {
        return 1
    }

    let i: int = 10

    ptr_test(&i, 20)

    if (i != 20)
    {
        print("Failed to test pointers...")
        return 1
    }

    return 0
}
```
