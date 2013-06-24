/*

object.c -- Actor Run-Time

"MIT License"

Copyright (c) 2013 Dale Schumacher, Tristan Slominski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

//#include <stdio.h>  /* for TRACE */
#include "object.h"

/*
object:
	Object is the root kind, containing just a polymorphic dispatch procedure.
*/

OOP
object_call(OOP obj, ...)
{
	va_list args;

	va_start(args, obj);
	OOP result = (obj->kind)(obj, args);
	va_end(args);
	return result;
}

OOP
object_new(DISP kind, size_t size)
{
	OOP self = (OOP)ALLOC(size);
	self->kind = kind;
	return self;
}

KIND(object_kind)
{
	return o_undef;
}

struct object undef_object = { object_kind };

/*
symbol:
	Symbols are constants with a string representation 's'.
*/

OOP
symbol_new(char * name)
{
	struct symbol * this = object_alloc(struct symbol, symbol_kind);
	this->s = name;
	return (OOP)this;
}

KIND(symbol_kind)
{
	if (symbol_kind == self->kind) {
		struct symbol * this = as_symbol(self);
		/* no object protocol for symbol */
	}
	return o_undef;
}

struct symbol _t_symbol = { { symbol_kind }, "#t" };
struct symbol _f_symbol = { { symbol_kind }, "#f" };
