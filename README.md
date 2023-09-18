# LilyChorus

Chorus effect! For making singular things sound like they are in plural. Or making them sound like bees? Maybe.

Attempts to implement things learned from [this article](https://www.soundonsound.com/techniques/more-creative-synthesis-delays).

## Status

This project mostly just exists for me learning Juce and plugin development.

Currently it's in an early state. It works, on my computer, but:

- There are no tests
- The UI is ugly
- It has too many params to be user friendly
- It's not particularly CPU efficient
- It's only been tested in Reaper on my computer
- There are no binaries
- There is no CI
- The default settings of the params are not very good
- ...

## Obtaining

Binaries might be available later, sorry!

## Building

It uses cmake so should hopefully be easy to build. I've been building it on windows with Clang that came from VS2022 with these commands:

```
cmake -B Builds -G "Visual Studio 17 2022" -T "ClangCL"
cmake --build Builds --config Release
```