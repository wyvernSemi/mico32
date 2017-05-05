/*
  Copyright 2001 Georges Menie
  https://www.menie.org/georges/embedded/small_printf_source_code.html

  Modified by Simon Southwell: Copyright (c) 2017

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _SMALL_PRINTF_
#define _SMALL_PRINTF_

int outbyte (int c);
int sml_printf(const char *format, ...);
int sml_sprintf(char *out, const char *format, ...);

#endif

