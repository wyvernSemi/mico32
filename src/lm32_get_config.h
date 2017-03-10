//=============================================================
// 
// Copyright (c) 2016 Simon Southwell. All rights reserved.
//
// Date: 27th August 2016  
//
// Header for cpumico32 configuration function 
//
// This file is part of the cpumico32 ISS linux system model.
//
// cpumico32 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// cpumico32 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with cpumico32. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: lm32_get_config.h,v 3.0 2016/09/07 13:15:38 simon Exp $
// $Source: /home/simon/CVS/src/cpu/mico32/src/lm32_get_config.h,v $
//
//=============================================================

#ifndef _LM32_GET_CONFIG_H_
#define _LM32_GET_CONFIG_H_

// Prototype for configuration parser
extern "C" lm32_config_t* lm32_get_config(int argc, char** argv, const char* default_ini_fname = (char*)"");

#endif
