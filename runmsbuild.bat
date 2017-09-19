@echo off
REM #########################################################
REM  
REM  Copyright (c) 2012 Simon Southwell. All rights reserved.
REM 
REM  Date: 19th July 2017 
REM  
REM  This file is part of the cpumico32 instruction set simulator.
REM 
REM  cpumico32 is free software: you can redistribute it and/or modify
REM  it under the terms of the GNU General Public License as published by
REM  the Free Software Foundation, either version 3 of the License, or
REM  (at your option) any later version.
REM 
REM  cpumico32 is distributed in the hope that it will be useful,
REM  but WITHOUT ANY WARRANTY; without even the implied warranty of
REM  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM  GNU General Public License for more details.
REM 
REM  You should have received a copy of the GNU General Public License
REM  along with cpumico32. If not, see <http://www.gnu.org/licenses/>.
REM 
REM  $Id: runmsbuild.bat,v 3.2 2017/07/11 07:13:19 simon Exp $
REM  $Source: /home/simon/CVS/src/cpu/mico32/runmsbuild.bat,v $
REM  
REM #########################################################

if "%VCINSTALLDIR%"=="" (
  call "c:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\vcvarsall.bat"
)
msbuild /m /noconlog /p:Configuration=Release msvc\%1.sln