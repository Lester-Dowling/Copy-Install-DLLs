# Copy-Installation-DLLs
A command line tool to copy corresponding DLLs of listed lib files into an installation folder.

The typical use case of this tool is to copy Boost DLLs next to newly built apps.  For example:

```PowerShell
  > Copy-Installation-DLLs C:/Projects/my-new-app C:/boost_MSVC_2019_x64/lib/boost_system-vc142-mt-x64-1_71.lib
```

will copy 

> boost_system-vc142-mt-x64-1_71.dll

into 

> C:/Projects/my-new-app
