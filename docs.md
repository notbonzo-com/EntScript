# EntS Code Documentation

EntS (EntScript or Enlightened Script) is a C inspired language, with a few syntax changed that I have implemented to make it more nice for me personally.

## EntS Syntax

### Comments and Whitespace

EntS is a whitespace insignificant language, meaning that whitespaces are only used to separate identifiers and keywords. They will generally be ignored.
Different whitespace include:

- Tabs `\t`
- Newlines `\n`
- Spaces ` `
As well as comments.
There are two types of comments, just like in C:

- Line comments `//`
- Block comments `/* */`

Line comments will make all text after them ignored up until a new line character `\n`.
Block comments will make all text after them ignored until the closing block comment `*/`

### EntS Functions

As EntS is functional, not object oriented, all executable code must be placed inside of a function.
A function is defined by the `function` keyword followed by the name of the function, its arguements in brackets `()` and its body in a block `{}`.
The return type of the function is specified after the `->` identifier before the block and can be any valid identifier (NOT a struct).

Arguements of a function are declared in format `type` `identifier` and are separated by `,`.

```ent
function exampleFunctionName(int32 arg1, int32 arg2) -> void {
    // One line comment, everything is ignored int32 i = 5; this is also ignored
    /*
    Block comment everything is ignored until the * / sequence without the space between
    */
    // Some code here...
};
```

A function may return `void` or any type. Even if a function returns `void` the void keyword must be specified! If a function returns non-void, it must include the
return keyword in itself followed by a value in the following syntax: `return` `value/expression` `;`.
A void returing function may return preemptively by using `return;`

## Variables

Variables can have two types. Global and local. Local variable's declarations are located inside of a function and will be forgotten once execution leaves the function.
They are stored on the stack, unless optimized. Global variables are declared outside of functions and have space allocated directly in the executable.

The general variable declaring syntax is:
- `type` `identifier` `=` `value/expression` `;`

Examples of both global and local variables are:

```ent

int32 globalVariableWithValueFive = 5;

function someFunctions(void) -> void
{
    int32 localVariableWithValueFive = 5;

};
```

## The memory adress operator `[]`

EntS uses a interesting approach to pointers, trying to be expressive just like assembly. There is nothing as a pointer and yet everything is a pointer.
When declaring a variable, it can be declared by value, allocated on the stack: `type` `identifier` `;` or by adress `type` `[` `identifier` `]`. Such identifier
doesnt exist until an adress is assigned to it. This can be done, just like to an ordinary variable by chaning its adress using the `[]` operator:
`[` `existingVariable` `]` `=` `newAdress` `;`
Where adress can be an expression which is parsable as a number. It can be a variable, an variable adress (`[` `existingVariable` `]`), or an expression.
The `[]` operator can be negated by itself.
`[[variable]]` will be the variable's value, not the adress.

```ent
int32 [pointer]; // Defines a global variable of an uninitilised `int32`

//int32 [pointerErr] = 5; This is illegal as you can not assign defualt values to global variables

function someFunctions(void) -> void
{
    int32 [pointer2];
    //pointer2 = 5; // Pointer2 now has the value 5, but its not initilised, so the program will crash
    [pointer2] = 5; // Pointer2 now is initlised to the adress `0x5`. It has the value of whatever is there

    int32 [pointer3] = 5; // Pointer3 now is initilised to the adress `0x5`. It has the value of whatever is there
    
    [pointer] = [pointer3] // The global variable pointer now is initilised to the same adress as pointer3, which is `0x5`

    int32 number = 10; // Somewhere, we dont know where (probably on the stack) number resides with the value 10;
    intt64 adressOfNumber = [number]; // AdressOfNumber now has the value of wherever number was initilised (most likely on the stack so dont mess with this)

    [pointer] = number; // Now pointer is initilised to the VALUE of number, which is 10, so pointer is initilised to `0xa`
};

```
Yes its very complicated so make sure you understand it. It can also be stacked, so `[[variable]]` is the same as `variable`.

## Indexation

We can use C like indexation, I am sure you understand, but it can be done with anything as indexation just gives you adress of the original var + index*sizeof(original_var). So if you index an uint32 by 2, you will get the value 64 bits after the end of the original varibale.

## Typedef

The typedef functions allows for creation of new types. Its generic strcture is:
`typedef` `oldType` `newType` `;`
A typedef may also include special operators such as the memory adress operator `[]`.
```ent
typedef [int32] int32Pointer;
```
Such variable is always accessed as the adress and in order the get its value, the `[]` operator must be used to negate itself.

Another complication of typedef are struct. Structs may only be defined as new types, not as structs themselfs.

```ent
typedef struct
{
    int32 value;
} int32Struct;
```
The snippet above defined a new struct called `int32Struct` with a single field called `value` of type `int32`.
Now the int32Struct is a valid type.

## C Like control flow

Ent has the classic C like control flow with the same syntax as C with the slight difference of having an `;` at the end.
Example:

```ent

function main(void) -> int32
{

    while(/* Expression */ 1==1) {

        // Do something
        continue; // Continue will skip to the next iteration

        break; // Break will exit the loop
    };

    int32 someValue = 2;

    switch(someValue) {
        case 1:
            // Do something
            break; // Break will exit the switch statement

        case 2:
            // Do something
            continue; // Continue will skip to the next case
        
        default:
            // Do something
            break;
    };
    if (someValue == 1) {
        // Do something
    } else if (someValue == 2) {
        // Do something
    } else {
        // Do something
    };

};
```

## header

The header block is a EntS specific feature. In case a .e file is includable by the preprocessor, it should contain a #header
The header syntax is:
```ent
header {
    // Function prototypes
    function functionName() -> void;
    function functionName(int32 arg1, int32 arg2) -> void;
};
```
These are the public function prototypes that can be used in other files. 
The header block can also include global variables and typedefs.
```ent
header {
    int32 globalVariable;
    typedef int32 int32Pointer;
    typedef struct
    {
        int32 value;
    } int32Struct;
    function functionName(int32 arg1, int32 arg2) -> void;
};
```
## Preprocessor directives.

The header is the first and most important preprocessor directive.
The others follow:

- #include `filename`

The include has two types:
```ent
#include "filename"
// And
#include <filename>
```
The difference being that with `"`, the preprocessor will look for the filename/path relative to the directory the current file is in. While the `<>` will look for the file relative to the `root` directory (Can be multiple) passed to the compiler with the -I flag. Only the functions variables and types from the header block will be included and visible to the linker!

- #asmstart
- #asmend

When the preprocessor encounters an `#asmstart` everything until the `#asmend` will be passed directly into the assembler. Keep in mind that `#asmend` must be on its own line
just like all preprocessor directives.

If you wish to use variables, you can specify them in the `#asmstart` directive using the following format:
#asmstart(variablename1, variablename2)
Then, these variables will be moved into the system v abi registers in order (Adress of `variablename1` will be in rdi, `variablename2` in rsi)

- #define `name` `value`
- #undef `name`

Will define a macro that will be expanded to the value when name is encountered. Value can be multiple words and is whitespace significant. It will be terminated with with a newline. It can be expanded to multiple lines using `\` as the last symbol before a newline.

## Default types

By default the default types are as follows:
- int8
- int16
- int32
- int64
- uint8
- uint16
- uint32
- uint64
- float
- char
- bool
- void