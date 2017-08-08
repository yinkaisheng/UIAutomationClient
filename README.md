# UIAutomationClient
The C++ source code of Python UIAutomation for Windows

C Python 3.5+ was built by VS2015(C:\Python36\vcruntime140.dll).
C Python 3.3+ was built by VS2010(C:\Python34\msvcr100.dll).
C Python below 3.3 was built by VS2008(C:\Python27\msvcr90.dll).

I built the dlls with different Visual Studio to fit different Python versions.

So you don't have to install Visual C++ Redistributable. 

If your Python was not built by VS2008, VS2010 or VS2015, you may need to install to Visual C++ Redistributable.

Build VS2008\UIAutomationClient.sln, VS2010\UIAutomationClient.sln and VS2015\UIAutomationClient.sln.

You'll get

UIAutomationClient_VC90_X64.dll

UIAutomationClient_VC90_X86.dll

UIAutomationClient_VC100_X64.dll

UIAutomationClient_VC100_X86.dll

UIAutomationClient_VC140_X64.dll

UIAutomationClient_VC140_X86.dll

in bin folder.

