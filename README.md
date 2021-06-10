# GenLibFromDll

A simple C++17 program to generate lib file from dll.

## Usage

Need to run in Visual Studio command line. (Require build tools: `DUMPBIN` and `LIB`)

Usage:

```
GenLibFromDll.exe some.dll
```

It will generate `some.def` and `some.lib` in the same floder.

## How it works

Step:

1. Use `DUMPBIN` to generate def of dll file
   - Internal command: `DUMPBIN some.dll /EXPORTS /OUT:some.def`
2. Modify def file for `LIB`
3. Generate lib file from def file
   - Internal command: `LIB /DEF:some.def /machine:X64 /OUT:some.lib`
