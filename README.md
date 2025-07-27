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

```beere
fn main() {
    println("Hello, World!")
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
