# Beere

Beere is a simple, fast, and low-level programming language, designed for systems programming with modern syntax and manual memory control.

> ⚙️ Inspired by C, Java, Go and C++, but with better readability, more safety, easier to develop projects, and optional object-oriented programming.

## Features

- Fast native compilation to x86_64 Assembly  
- Manual memory management with safety checks  
- Simple and clean syntax 
- Optional object-oriented features (classes, vtables)  
- Easy C interoperability (`extern "C" fn`)  
- Lightweight module system and package manager (WIP)

## Hello World

```rs
fn main() {
    println("Hello, World!")
}
```

## Object-oriented
Beere is has OOP support, and will be improved with more features in other releases.
```ts
class Parent {
    public field_2: int = 10

    public Parent(n1: int) {
        this.field_2 = n1
    }
}

class Child extends Parent {
    public n1: int = 0

    public Child(n1: int) {
        super(n1)
        this.n1 = n1
    }
}

fn main() {
    let child: Child = new Child(10)
}
```

## Building
To compile a Beere file:

```
beerec path/to/entry/point path/to/dotmod/file
```
(Compiler will generate raw Assembly and link it automatically.)

## Status

> ⚠️ Beere is under active development.
