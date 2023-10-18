# Parser

- It can parse literals and block statements, includint nested blocks

## High Priority Problems
- Hard to debug - it's hard to explore this recursive AST in debugger, and there's absolutely no way to observe it in any other way (printing to stdout in some form, or even export as dot)
- Hard to test - it's hard to construct AST for even simple block statement like "{ 'asd' }", the code is unreadable

## Lower priority problems
- No correspondence between the line of code and the token, which makes error reporting poor
- All dynamic parts of the AST are allocated with general purpose heap allocator. It's not bad while prototyping, but eventually custom memory allocators should come. For AST and symbols it's just simple memory arena, because this data is static and permanent, but anyway it's better to allocate stuff according to the usage patterns
