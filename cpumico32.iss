
[Setup]
#define DocDir          "C:\Users\simon\Documents\web\homepage\wyvernsemi\lm32"

AppName                 = CPUMICO32
AppVerName              = CPUMICO32_3_20
DefaultDirName          = {pf}\mico32
DisableProgramGroupPage = yes
OutputBaseFilename      = setup_cpumico32_3_20

[Dirs]
Name: "{app}\obj"
Name: "{app}\obj_lnx"

[Files]
; This inno file
Source: "cpumico32.iss";                               DestDir: "{app}"

; Linux/cygwin make files
Source: "makefile";                                    DestDir: "{app}"
Source: "makefile.lnx";                                DestDir: "{app}"

; MSVC setup and build batch file
Source: "runmsbuild.bat";                              DestDir: "{app}"

; Example .ini file
Source: "lm32cpu.ini";                                 DestDir: "{app}"

; Documentation
Source: "doc\README.pdf";                              DestDir: "{app}\doc"; Flags: isreadme
;Source: "{#DocDir}\index.html";                        DestDir: "{app}\doc"; DestName: "readme.htm"; Flags: isreadme
;Source: "{#DocDir}\lm32_files\*.jpg";                  DestDir: "{app}\doc\lm32_files"
;Source: "{#DocDir}\lm32_files\*.png";                  DestDir: "{app}\doc\lm32_files" 
;Source: "{#DocDir}\..\wyvernsemi_files\wy.css";        DestDir: "{app}\wyvernsemi_files"
;Source: "{#DocDir}\..\wyvernsemi_files\*.png";         DestDir: "{app}\wyvernsemi_files"    
;Source: "man\cpumico32.1";                             DestDir: "{app}\man"

; Visual studio files for 2010 express
Source: "msvc\cpumico32.sln";                          DestDir: "{app}\msvc"
Source: "msvc\cpumico32\cpumico32.vcxproj";            DestDir: "{app}\msvc\cpumico32"
Source: "msvc\cpumico32\cpumico32.vcxproj.filters";    DestDir: "{app}\msvc\cpumico32"
Source: "msvc\cpumico32\cpumico32.vcxproj.user";       DestDir: "{app}\msvc\cpumico32"
Source: "msvc\libmico32\libmico32.vcxproj";            DestDir: "{app}\msvc\libmico32"
Source: "msvc\libmico32\libmico32.vcxproj.filters";    DestDir: "{app}\msvc\libmico32"
Source: "msvc\libmico32\libmico32.vcxproj.user";       DestDir: "{app}\msvc\libmico32"
Source: "msvc\lnxmico32\lnxmico32.vcxproj";            DestDir: "{app}\msvc\lnxmico32"
Source: "msvc\lnxmico32\lnxmico32.vcxproj.filters";    DestDir: "{app}\msvc\lnxmico32"
Source: "msvc\lnxmico32\lnxmico32.vcxproj.user";       DestDir: "{app}\msvc\lnxmico32"

; Pre-compiled executables and library for windows
Source: "msvc\Release\cpumico32.exe";                  DestDir: "{app}"
Source: "msvc\Release\lnxmico32.exe";                  DestDir: "{app}"
Source: "msvc\Release\libmico32.dll";                  DestDir: "{app}"

; Source code
Source: "src\cpumico32.cpp";                           DestDir: "{app}\src"
Source: "src\cpumico32_c.c";                           DestDir: "{app}\src"
Source: "src\getopt.c";                                DestDir: "{app}\src"
Source: "src\lm32_cache.cpp";                          DestDir: "{app}\src"
Source: "src\lm32_cache.h";                            DestDir: "{app}\src"
Source: "src\lm32_cpu.cpp";                            DestDir: "{app}\src"
Source: "src\lm32_cpu.h";                              DestDir: "{app}\src"
Source: "src\lm32_cpu_c.cpp";                          DestDir: "{app}\src"
Source: "src\lm32_cpu_c.h";                            DestDir: "{app}\src"
Source: "src\lm32_cpu_disassembler.cpp";               DestDir: "{app}\src"
Source: "src\lm32_cpu_elf.cpp";                        DestDir: "{app}\src"
Source: "src\lm32_cpu_elf.h";                          DestDir: "{app}\src"
Source: "src\lm32_cpu_hdr.h";                          DestDir: "{app}\src"
Source: "src\lm32_cpu_inst.cpp";                       DestDir: "{app}\src"
Source: "src\lm32_cpu_mico32.h";                       DestDir: "{app}\src"
Source: "src\lm32_get_config.cpp";                     DestDir: "{app}\src"
Source: "src\lm32_get_config.h";                       DestDir: "{app}\src"
Source: "src\lm32_gdb.cpp";                            DestDir: "{app}\src"
Source: "src\lm32_gdb.h";                              DestDir: "{app}\src"
Source: "src\lm32_tlb.cpp";                            DestDir: "{app}\src"
Source: "src\lm32_tlb.h";                              DestDir: "{app}\src"
Source: "src\lnxmico32.cpp";                           DestDir: "{app}\src"
Source: "src\lnxmico32.h";                             DestDir: "{app}\src"
Source: "src\lnxtimer.cpp";                            DestDir: "{app}\src"
Source: "src\lnxtimer.h";                              DestDir: "{app}\src"
Source: "src\lnxuart.cpp";                             DestDir: "{app}\src"
Source: "src\lnxuart.h";                               DestDir: "{app}\src"

; Python GUI for cpumico32
Source: "python\lm32.py";                              DestDir: "{app}\python"
Source: "python\icon.gif";                             DestDir: "{app}\python"
Source: "python\favicon.ico";                          DestDir: "{app}\python"
Source: "python\tbimages\*.gif";                       DestDir: "{app}\python\tbimages"

; Linux binaries for lnxmico32 case study
Source: "test\romfs.ext2";                             DestDir: "{app}\test"
Source: "test\vmlinux.bin";                            DestDir: "{app}\test"

; Test .ini files
Source: "test\lnx.ini";                                DestDir: "{app}\test"
Source: "test\test.ini";                               DestDir: "{app}\test"

; Python script to run model test code
Source: "test\runtest.py";                             DestDir: "{app}\test"

; Model test source code
Source: "test\api\num_instr\test.s";                   DestDir: "{app}\test\api\num_instr"
Source: "test\exceptions\dbus_errors\test.s";          DestDir: "{app}\test\exceptions\dbus_errors"
Source: "test\exceptions\external\test.s";             DestDir: "{app}\test\exceptions\external"
Source: "test\exceptions\hw_debug\test.s";             DestDir: "{app}\test\exceptions\hw_debug"
Source: "test\exceptions\ibus_errors\test.s";          DestDir: "{app}\test\exceptions\ibus_errors"
Source: "test\exceptions\instruction\test.s";          DestDir: "{app}\test\exceptions\instruction"
Source: "test\instructions\add\test.s";                DestDir: "{app}\test\instructions\add"
Source: "test\instructions\and\test.s";                DestDir: "{app}\test\instructions\and"
Source: "test\instructions\branch_cond\test.s";        DestDir: "{app}\test\instructions\branch_cond"
Source: "test\instructions\branch_uncond\test.s";      DestDir: "{app}\test\instructions\branch_uncond"
Source: "test\instructions\cmp_e_ne\test.s";           DestDir: "{app}\test\instructions\cmp_e_ne"
Source: "test\instructions\cmpg\test.s";               DestDir: "{app}\test\instructions\cmpg"
Source: "test\instructions\cmpge\test.s";              DestDir: "{app}\test\instructions\cmpge"
Source: "test\instructions\csr\test.s";                DestDir: "{app}\test\instructions\csr"
Source: "test\instructions\div\test.s";                DestDir: "{app}\test\instructions\div"
Source: "test\instructions\load\test.s";               DestDir: "{app}\test\instructions\load"
Source: "test\instructions\mul\test.s";                DestDir: "{app}\test\instructions\mul"
Source: "test\instructions\or\test.s";                 DestDir: "{app}\test\instructions\or"
Source: "test\instructions\sext\test.s";               DestDir: "{app}\test\instructions\sext"
Source: "test\instructions\sl\test.s";                 DestDir: "{app}\test\instructions\sl"
Source: "test\instructions\sr\test.s";                 DestDir: "{app}\test\instructions\sr"
Source: "test\instructions\store\test.s";              DestDir: "{app}\test\instructions\store"
Source: "test\instructions\sub\test.s";                DestDir: "{app}\test\instructions\sub"
Source: "test\instructions\template\test.s";           DestDir: "{app}\test\instructions\template"
Source: "test\instructions\xor\test.s";                DestDir: "{app}\test\instructions\xor"
Source: "test\mmu\tlb\test.s";                         DestDir: "{app}\test\mmu\tlb"
Source: "test\speed\test.s";                           DestDir: "{app}\test\speed"

; Driver code
Source: "drivers\crt0.s";                              DestDir: "{app}\drivers"
Source: "drivers\lm32_uart_driver.c";                  DestDir: "{app}\drivers"
Source: "drivers\small_printf.c";                      DestDir: "{app}\drivers"
Source: "drivers\small_printf.h";                      DestDir: "{app}\drivers"
Source: "drivers\lnxram.ld";                           DestDir: "{app}\drivers"
Source: "drivers\cpuram.ld";                           DestDir: "{app}\drivers"
Source: "drivers\liblm32drivers.a";                    DestDir: "{app}\drivers"

; Example code
Source: "examples\simple_c\makefile";                  DestDir: "{app}\examples\simple_c"
Source: "examples\simple_c\simple_c.c";                DestDir: "{app}\examples\simple_c"
Source: "examples\simple_c\simple_c.elf";              DestDir: "{app}\examples\simple_c"
