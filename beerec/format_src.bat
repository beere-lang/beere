@echo off
for /r . %%f in (*.h *.c *.cpp *.hpp *.cc *.cxx) do (
    echo %%f | findstr /v /i "build" >nul && (
        echo Formatting %%f
        clang-format -style=file -i "%%f"
    )
)
echo Formatted all source files successfully...
pause