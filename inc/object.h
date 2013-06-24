/*

object.h -- Actor Run-Time

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
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "art.h"
#include <stdarg.h>

typedef struct object * OOP;
#define KIND(kind) OOP kind(OOP self, va_list args)
typedef OOP (*DISP)(OOP self, va_list args);

/*
 * object
 */

struct object {
	DISP			kind;
};

#define	take_arg()	va_arg(args, OOP)

extern OOP		object_call(OOP obj, ...);
extern OOP		object_new(DISP kind, size_t size);
#define	object_alloc(structure, kind)	((structure *)object_new((kind), sizeof(structure)))

extern KIND(object_kind);

extern struct object undef_object;
#define	o_undef	((OOP)&undef_object)

/*
 * symbol
 */

struct symbol {
	struct object	o;
	char *			s;
};
#define	as_symbol(oop)	((struct symbol *)(oop))
extern OOP		symbol_new(char * name);
extern KIND(symbol_kind);

extern struct symbol _t_symbol;
#define	o_true	((OOP)&_t_symbol)
extern struct symbol _f_symbol;
#define	o_false	((OOP)&_f_symbol)

#endif /* _OBJECT_H_ */
