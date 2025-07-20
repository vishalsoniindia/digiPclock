@echo off
setlocal enabledelayedexpansion

:START
:: Fetch all available COM ports (connected or not)
set "COMPORTS="
for /f "tokens=*" %%i in ('powershell -command "[System.IO.Ports.SerialPort]::getportnames()"') do (
    set "port=%%i"
    set "isDuplicate=0"
    
    :: Check for duplicates
    for %%j in (!COMPORTS!) do (
        if "!port!"=="%%j" (
            set "isDuplicate=1"
            goto :continue
        )
    )

    :continue
    if "!isDuplicate!"=="0" (
        set "COMPORTS=!COMPORTS! !port!"
    )
)

:: Check if COMPORTS is empty
if "%COMPORTS%"=="" (
    echo No COM ports found.
    pause
    exit /b
)

:: Display available COM ports with numbered options
echo Available COM Ports:
set /a count=1
for %%i in (%COMPORTS%) do (
    echo   !count!. %%i
    set "COM_!count!=%%i"
    set /a count+=1
)

:: Prompt user for selection
set /p choice="Select COM port list number: "

:: Validate selection and set the chosen COM port
set "selectedCOM=!COM_%choice%!"
if "%selectedCOM%"=="" (
    echo Invalid selection. Exiting...
    exit /b
)

:: Show selected COM port
echo Selected COM Port: %selectedCOM%

:: Flashing process

esptool.exe --chip esp32 -p %selectedCOM% -b 460800 erase_flash

esptool.exe --chip esp32 --port %selectedCOM% --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 ino.bin

echo Flashing complete on %selectedCOM%.

:: Prompt to flash again
echo.
echo Press any key to flash again, or close the window to exit.
pause >nul
goto START