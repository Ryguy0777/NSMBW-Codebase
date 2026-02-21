# Contributing

## Code Guidelines

### General

- Lines should not exceed `100` characters, these can be split into multiple lines.
- Use `nullptr` instead of `0` when assigning / comparing a pointer (unless strictly necessary).
- Use the `.cpp`/`.hpp` extension for C++ files, the `.c`/`.h` extension for C files and the `.s` extension for ASM files.
- The preferred indentation style is [1TBS/OTBS](https://en.wikipedia.org/wiki/Indent_style#Variant:_1TBS_(OTBS)).
- Empty code blocks should only take one line.

### Headers

- Headers that are new to PropelParts and not from the original game must go inside of `include/new/`.
- Use forward-declared types when possible.
- At the top of every header place:

    ```cpp
    #pragma once
    ```

### Includes

- For includes use:

    ```cpp
    #include <...>
    ```

    (These are relative to the `include` folder.)

### Names

- Names for known symbols should match **exactly**, including typos.
- Member variables must be prefixed with `m` (and `p` if they're a pointer).
- Static member variables must be prefixed with `s` (and `p` if they're a pointer).
- The above rule can be ignored if the existing symbol differs.
- Functions with no known symbol must use **camelCase**.

### Types

- Pointer/reference types must have the asterisk/ampersand near the variable name.
- Use C style casts to cast a type to another.

### Constants

- Constants with a heavy impact on the game must be declared and used properly. If such a constant appears in more than one compilation unit, it must be placed in `constants/game_constants.h`. Else, place it in the correct header file.
- Japanese text strings **must** be placed in `constants/sjis_constants.h`.

### Enumerations

- Unless a cracked symbol says otherwise, the following conventions apply:
  - The enumeration tag should be all uppercase with the suffix `_e`.
  - Enumeration identifiers should be all uppercase.
  - Global enum identifiers must be prefixed, while for class enum identifiers it's optional (do it, for example, if it increases readability or prevents confusion).

### Comments

- Code comments should begin with an uppercase letter and usually not end in a period.

### Classes

- When referencing a class member, do **not** use `this->` unless required for compilation.
- Try to use constructor initializers as much as possible.
- Class members must be placed in the following order:
  - Nested Classes/Structures/Enumerations
  - Functions (place static ones last)
  - Variables (place static ones last)
  - Friends

- Functions for classes must be placed in the following order:
  - Constructor
  - Destructor (unless virtual)
  - Operators (unless virtual)
  - Virtual Functions
  - Member Functions (place static ones last)

- Set appropriate access modifiers for each member. Within each category listed above, place the entries in the following order:
  - Public
  - Protected
  - Private

- If the virtual functions do not follow the ordering conventions, the above rules can be ignored.

## Documentation
- If no official symbol has been cracked for a class or a function, report this information using the `@unofficial` command. For unknown class names, adding the note to each member function is not necessary.
- Do **not** document inline getters/setters unless their logic is complex, document the corresponding variables instead.
