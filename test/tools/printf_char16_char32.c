/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * printf_char16_char32.cpp - printf handler for char16_t and char32_t
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
 *
 * The COJSON Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License v2
 * as published by the Free Software Foundation;
 *
 * The COJSON Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <printf.h>
#include <uchar.h>

static int print_char16(FILE* stream, const struct printf_info* info,
		const void* const* args) {
    const char16_t* str = *((const char16_t**)(args[0]));
    int r, l, n = 0;
	char mb[MB_CUR_MAX];
    while(*str) {
    	mbstate_t ps = { 0, {0}} ;
    	n += (l=c16rtomb(mb, *str, &ps));
    	if( l == (size_t)-1 ) {
    		fprintf(stream, "\\u%4.4X",*str);
    	} else
    		if( (r=fwrite(mb,l,1,stream)) < 0 ) return r;
    	++str;
    }
    return n;
}

static int print_char16_arginfo(const struct printf_info* info, size_t n,
		int* argtypes, int* size) {
    if (n > 0) {
        argtypes[0] = PA_POINTER;
        size[0] = sizeof (char16_t *);
    }
    return 1;
}

static int print_char32(FILE* stream, const struct printf_info* info,
		const void* const* args) {
    const char32_t* str = *((const char32_t**)(args[0]));
    int r, l, n = 0;
	char mb[MB_CUR_MAX];
    while(*str) {
    	mbstate_t ps = { 0, {0}};
    	n += (l=c32rtomb(mb, *str, &ps));
    	if( l == (size_t)-1 ) {
    		if( (*str)>>16 )
        		fprintf(stream, "\\u%4.4X",(*str)>>16);
    		fprintf(stream, "\\u%4.4X",(*str)&0xFFFF);
    	} else
    	if( (r=fwrite(mb,l,1,stream)) < 0 ) return r;
    	++str;
    }
    return n;
}

static int print_char32_arginfo(const struct printf_info* info, size_t n,
		int* argtypes, int* size) {
    if (n > 0) {
        argtypes[0] = PA_POINTER;
        size[0] = sizeof (char32_t *);
    }
    return 1;
}

int register_char16_specifier(void) {
    return register_printf_specifier('w', print_char16, print_char16_arginfo);
}

int register_char32_specifier(void) {
    return register_printf_specifier('W', print_char32, print_char32_arginfo);
}
