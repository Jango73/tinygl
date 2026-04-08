# AGENTS.md

## Project Overview
This is a compact CPU implementation of OpenGL.

## Main rule
**If the guidelines below are not followed, all modifications will be rejected.**

## Communication Guidelines
- NEVER use emojis in responses.
- If a demand DOES NOT make sense (for instance, breaks architecture), SAY IT and ask for confirmation BEFORE DOING ANYTHING.
- NEVER commit unless explicitly asked for in the current conversation.

## Coding Conventions
- **Naming**: camelCase for functions, PascalCase for variables/struct members, SCREAMING_SNAKE_CASE for structs/defines.
- **Declaration order**: Group declarations by type. 1: macros / 2: type definitions / 3: inline functions / 4: external functions / 5: other
- **Function order**: DO NOT OVERUSE forward declarations. Define functions before they are used.
- **I18n**: Write comments, console output and technical doc in english.
- **Naming clarity**: In addition to using full words, every name must express its intent clearly and without ambiguity.
- **Comments**: For single-line comments, use `//`, not `/*`.
- **Style**: 4-space indentation, follow `.clang-format` rules.
- **Numbers**: Hexadecimal for constant numbers, except for sizes, vectors, points and time.
- **Number suffixes**: Do not add numeric suffixes like `u` to constants; they are not wanted here.
- **Clean code**: No duplicate code. Create intermediate functions to avoid it. This also applies to data: create intermediate structures to avoid duplicating data.
- **No globals**: Before adding a global variable, **ALWAYS ASK** if permitted.
- **Functions**: Add a doxygen header to functions and separate all functions with a 75 character line such as : /************************************************************************/
- **File size**: Keep source files under 1000 lines; split by responsibility before crossing this limit.
