@echo off

echo Baixando raylib...

curl -L -o raylib.zip https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win64_mingw-w64.zip

echo Extraindo...

powershell -command "Expand-Archive raylib.zip libs/"

move libs\raylib-5.0_win64_mingw-w64 libs\raylib

del raylib.zip

echo Pronto!