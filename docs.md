# EntS Language Documentation

## Table of Contents
1. [Introduction](#introduction)
2. [Syntax Overview](#syntax-overview)
   - [Comments and Whitespace](#comments-and-whitespace)
   - [Functions](#functions)
3. [Variables](#variables)
4. [Memory Address Operator `[]`](#memory-address-operator-)
5. [Indexing](#indexing)
6. [Typedef](#typedef)
7. [Control Flow](#control-flow)
8. [Headers](#headers)
9. [Preprocessor Directives](#preprocessor-directives)
10. [Default Types](#default-types)

---

## Introduction

EntS (short for Enlightened Script) is a programming language inspired by C, with several syntactic changes tailored for a more intuitive development experience. This documentation provides an overview of EntS syntax, functions, variables, memory handling, control flow, and preprocessor directives.

---

## Syntax Overview

### Comments and Whitespace

EntS is a whitespace-insensitive language, meaning that spaces, tabs, and newlines serve only to separate identifiers and keywords, with no impact on the execution. Whitespace characters include:

- **Tabs**: `\t`
- **Newlines**: `\n`
- **Spaces**: ` `

In addition, comments in EntS function similarly to those in C:

- **Single-line comments**: `//`
  - Everything after `//` is ignored until the next newline.
  
- **Block comments**: `/* ... */`
  - All text within `/*` and `*/` is ignored.

### Functions

EntS is a functional language, requiring all executable code to reside within functions. A function is declared using the `function` keyword, followed by the function name, parameters in parentheses `()`, and a block `{}` for the function's body. The return type is specified after the `->` symbol.

#### Syntax:
```ent
function functionName(type1 param1, type2 param2) -> returnType {
    // Function body
};
```
#### Example:
```ent
function exampleFunction(int32 arg1, int32 arg2) -> void {
    // Function code...
};
```
- **Return Type**: Functions must specify a return type, including `void`.
    - If the return type is not `void`, the function must include a return statement with a value.
    - A `void` function can exit early using `return;`.

### Variables

Variables in EntS can be either global or local:

 - **Global Variables**: Declared outside of functions, with space allocated directly in the executable.
 - **Local Variables**: Declared within functions and exist only during function execution, typically stored on the stack.

#### Syntax:
```ent
type variableName = value/expression;
```

#### Example:
```ent
int32 globalVar = 5;

function sampleFunction(void) -> void {
    int32 localVar = 5;
};
```

### Memory Address Operator `[]`

In EntS, the memory adress operator, the square brackets can be used in two ways.

- **For indexation of variables**: `variable[index]`
- **For switching variable adressing mode**: `[variable]`

When a variable is enclosed is `[]` it can be used for both assigning to change its address or as an expression to evaluate its address.

### Control Flow

EntS supports traditional C-style control flow, with minor syntactical differences such as mandatory semicolons after control structures.

**Example**:
```ent
function main(void) -> int32 {
    int32 someValue = 2;

    while (someValue < 5) {
        someValue++;
        continue;
    };

    switch(someValue) {
        case 1:
            // Action for case 1
            break;
        case 2:
            // Action for case 2
            continue;
        default:
            // Default action
            break;
    };

    if (someValue == 1) {
        // Action for someValue == 1
    } else if (someValue == 2) {
        // Action for someValue == 2
    } else {
        // Default action
    };

    return someValue;
};
```

### Headers

EntS features a specific header block for files intended to be included by the preprocessor. The `header` block contains function prototypes, global variables, and typedefs.

#### Syntax:
```ent
header {
    // Declarations
};
```
#### Example:
```ent
header {
    int32 globalVariable;
    typedef int32 int32Pointer;
    function functionName(int32 arg1, int32 arg2) -> void;
};
```

## Preprocessor Directives

EntS provides several preprocessor directives:

- `#header`: Indicates that the file contains header information.
- `#include "filename"` and `#include <filename>`:
  - The first searches relative to the current directory.
  - The second searches in root directories specified with the `-I` flag.
- `#define` name value: Defines a macro.
- `#undef` name: Undefines a macro.

## Default Types

EntS includes the following default types:

 - Signed Integers: `int8`, `int16`, `int32`, `int64`
 - Unsigned Integers: `uint8`, `uint16`, `uint32`, `uint64`
 - Floating Point: `float`
 - Character: `char`
 - Boolean: `bool`
 - Void: `void`