> [Русская версия этого файла](README_ru.md)
# Turbostroi V2 with cross-compile support
- [x] Linux compile
- [x] Use Source Engine think instead of our thread
- [x] Code refactoring 
- [ ] Optimization

# Manual for Windows MSVC compile:
1. Install Visual Studio 2015 or newer
2. [Get](https://premake.github.io/download) `premake5.exe` for Windows
3. Place and run `premake5.exe` in this folder:
```
premake5.exe vs2022
```
- `vs2015` for Visual Studio 2015
- `vs2017` for Visual Studio 2017
- `vs2019` for Visual Studio 2019
- `vs2022` for Visual Studio 2022
4. Open `x86 Native Tools Command Prompt for VS` command prompt
5. Go to `external\luajit\src` folder
6. Run command:
```
msvcbuild.bat static
```
7. Copy `lua51.lib` to `external\luajit\x86`
8. Open `projects\windows\<vs20xx>\turbostroi.sln`
9. Compile with `Release/Win32` configuration
10. Copy `projects\windows\vs2022\x86\Release\gmsv_turbostroi_win32.dll` to `GarrysModDS\garrysmod\lua\bin` (create `bin` folder if it doesn't exist) 

# Manual for Linux GCC compile:
1. [Get](https://premake.github.io/download) `premake5.exe` for Linux
2. Place and run `premake5` in this folder:
```
chmod +x ./premake5
./premake5 gmake --gmcommon=external/garrysmod_common
```

3. To build LuaJIT in `external/luajit/src/Makefile` find
```
CC= $(DEFAULT_CC)
```
and add `-m32` parameter:
```
CC= $(DEFAULT_CC) -m32
```
4. Run `make` in `external/luajit` directory
5. Copy `external/luajit/src/libluajit.a` to `external/luajit/x86`
7. To build Boost library run `bootstrap.sh` in `external/boost/`
8. Run `b2` in `external/boost/` with next recommended args: 
```
--build-dir=build/x86 address-model=32 threading=multi --stagedir=./bin/x86 --toolset=gcc -j 16 link=static runtime-link=static --variant=release
```
9. Open terminal in `projects/linux/gmake`
10. Run compilation
```
make config=release_x86
```
11. Copy `projects/linux/gmake/x86/Release/gmsv_turbostroi_linux.dll` to `GarrysModDS\garrysmod\lua\bin` (create `bin` folder if it doesn't exist) 

# IMPORTANT: DO NOT RUN THE SERVER ON GMOD x64 (TROUBLES WITH GMOD PHYSICS)
