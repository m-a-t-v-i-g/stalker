@echo off
REM Убиваем процесс UnrealEditor, чтобы избежать блокировки файлов
echo Завершаем процесс UnrealEditor...
taskkill /f /im UnrealEditor.exe
taskkill /f /im UnrealLightmass.exe
taskkill /f /im UnrealPak.exe

REM Подождите пару секунд для завершения процессов
timeout /t 5 /nobreak >nul

REM Устанавливаем переменную для папки проекта Unreal Engine
set PROJECT_DIR=%~dp0

REM Удаление папок Build, Binaries, DerivedDataCache, Intermediate, Saved и .vs
echo Удаляем папки Build, Binaries, DerivedDataCache, Intermediate, Saved и .vs...
rmdir /s /q "%PROJECT_DIR%Build"
rmdir /s /q "%PROJECT_DIR%Binaries"
rmdir /s /q "%PROJECT_DIR%DerivedDataCache"
rmdir /s /q "%PROJECT_DIR%Intermediate"
rmdir /s /q "%PROJECT_DIR%Saved"
rmdir /s /q "%PROJECT_DIR%.vs"

REM Удаление .sln файла
echo Удаляем .sln файл...
del /q "%PROJECT_DIR%*.sln"

REM Уведомляем, что все завершено
echo Очистка завершена!
pause