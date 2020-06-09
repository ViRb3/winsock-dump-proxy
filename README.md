# WinSock Dump Proxy
> A proxy DLL for Windows to dump Windows Socket connections.

## Limitations
- x64 only
- Only works if your target imports `version.dll`

## Usage
Download the latest [release](https://github.com/ViRb3/winsock-dump-proxy/releases). Place `version.dll` next to your target executable. On your desktop, create a new directory called `SOCKDUMP`.

When you run your target, you will see a message box with the hooking result. Once you press `OK`, all Windows Socket connections will be saved under the directory on your desktop, in a separate file based on the port.

## FAQ
- Q: I am getting `Error 2` \
A: `SOCKDUMP` is not accessible on your desktop

## Compilation
- mingw-w64 7.0.0+
- CMake 3.16+

## References
- Based on [jvm-dump-proxy](https://github.com/ViRb3/jvm-dump-proxy/releases), check it out for a detailed technical explanation
