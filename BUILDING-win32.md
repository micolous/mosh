# Building the Win32 port #

Install required dependencies:

- Visual Studio 2010 (with C++ compiler tools installed)
- OpenSSL for Windows distribution ([such as this one](http://slproweb.com/products/Win32OpenSSL.html) -- select the non-light version)
- [Download source for Protobuf-2.5.0](https://code.google.com/p/protobuf/downloads/list) and extract it to the root of the mosh folder.
- [Download source for zlib](http://zlib.net) and extract the folder into `win/mosh` and rename the `zlib-X.X.X` folder to `zlib`.
- [CMake](http://www.cmake.org/cmake/resources/software.html), installed with it in your `PATH`.

## Build protobuf ##

Go into the `protobuf-2.5.0/vsprojects` folder, and open up `protobuf.sln` in Visual Studio.

Then build the solution (F6).

Note: The configuration for mosh is set up such that it will use the Debug target of protobuf when in Debug mode, and the Release target of protobuf when in Release mode.

## Build zlib ##

Open the Visual Studio Command Prompt (2010), and change to the directory where you extracted zlib.

At the command prompt, type:

    > mkdir build
	> cd build
	> cmake .. -G"Visual Studio 2010" -DCMAKE_INSTALL_PREFIX="install"
	> msbuild /P:Configuration=Release INSTALL.vcxproj

You should now have built libraries in your `zlib/build/install` directory.  These will be referenced by the `mosh` Visual Studio project.

## Build mosh ##

Open `mosh.sln` in Visual Studio.

Select the "Release" or "Debug" configuration as appropriate, and select build.

TODO: Make the project file automatically copy `zlib.dll`.  For now, copy `zlib.dll` into the same folder as `mosh-client.exe`.

Note: the `mosh` wrapper for establishing the initial SSH connection does not work properly on Windows at this time.  You need to do this manually.