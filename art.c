/*

art.c -- Actor Run-Time

To compile:
	cc -o art art.c

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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define	TRACE(x)	x		/* enable tracing */

/*
object:
	Object is the root kind, containing just a polymorphic dispatch procedure.
*/

typedef struct object * OOP;
typedef OOP (*FUNCP)(OOP self, OOP msg);
#define KIND(kind) OOP kind(OOP self, OOP msg)

struct object {
	FUNCP			kind;
};

KIND(object_kind);

OOP
object_new(FUNCP kind, size_t size)
{
	OOP oop = (OOP)calloc(1, size);
	oop->kind = kind;
	return oop;
}

KIND(object_kind)
{
	return self;
}

struct object undef_object = { object_kind };
struct object nil_object = { object_kind };

OOP
object_call(OOP obj, OOP msg)
{
	return (obj->kind)(obj, msg);
}

/*
message:
	Messages consists of a selector 'name' and related 'args'.
*/

struct message {
	struct object	o;
	OOP				name;
	OOP				args;
};

KIND(message_kind);

struct message *
message_new(OOP name, OOP args)
{
	struct message * oop = (struct message *)object_new(message_kind, sizeof(struct message));
	oop->name = name;
	oop->args = args;
	return oop;
}

struct symbol name_symbol;
struct symbol args_symbol;

KIND(message_kind)
{
	if (message_kind == self->kind) {
		struct message * oop = (struct message *)self;		
		if (message_kind == msg->kind) {
			struct message * mp = (struct message *)msg;		
			if (mp->name == (OOP)&name_symbol) {
				return oop->name;
			}
			if (mp->name == (OOP)&args_symbol) {
				return oop->args;
			}
		}
	}
	return &undef_object;
}

/*
symbol:
	Symbols are constants with a string representation 's'.
*/

struct symbol {
	struct object	o;
	char *			s;
};

KIND(symbol_kind);

struct symbol _t_symbol = { { symbol_kind }, "#t" };
struct symbol _f_symbol = { { symbol_kind }, "#f" };
struct symbol eq_symbol = { { symbol_kind }, "eq" };
struct symbol name_symbol = { { symbol_kind }, "name" };
struct symbol args_symbol = { { symbol_kind }, "args" };
struct symbol bind_symbol = { { symbol_kind }, "bind" };
struct symbol lookup_symbol = { { symbol_kind }, "lookup" };

struct symbol *
symbol_new(char * name)
{
	struct symbol * oop = (struct symbol *)object_new(symbol_kind, sizeof(struct symbol));
	oop->s = name;
	return oop;
}

KIND(symbol_kind)
{
	if (symbol_kind == self->kind) {
		struct symbol * oop = (struct symbol *)self;
		if (message_kind == msg->kind) {
			struct message * mp = (struct message *)msg;		
			if (mp->name == (OOP)&eq_symbol) {
				if (self == mp->args) {
					return (OOP)&_t_symbol;
				}
				return (OOP)&_f_symbol;
			}
		}
	}
	return &undef_object;
}

/*
dict:
	Dictionaries define mappings from names to values.
	
	In this implementation, each node holds a single name/value pair.
	The 'next' pointer delegates to a linear chain of dictionaries.
*/

struct dict {
	struct object	o;
	OOP				name;
	OOP				value;
	OOP				next;
};

KIND(dict_kind);

struct dict *
dict_new(OOP name, OOP value, OOP next)
{
	struct dict * oop = (struct dict *)object_new(dict_kind, sizeof(struct dict));
	oop->name = name;
	oop->value = value;
	oop->next = next;
	return oop;
}

KIND(dict_kind)
{
	if (dict_kind == self->kind) {
		struct dict * oop = (struct dict *)self;
		if (message_kind == msg->kind) {
			struct message * mp = (struct message *)msg;		
			if (mp->name == (OOP)&bind_symbol) {
				if (message_kind == mp->args->kind) {
					mp = (struct message *)mp->args;
					return (OOP)dict_new(mp->name, mp->args, self);
				}
			}
			if (mp->name == (OOP)&lookup_symbol) {
				if (mp->args == oop->name) {  // [FIXME] consider using "eq" message here
					return oop->value;
				}
				return object_call(oop->next, msg);
			}
		}
	}
	return &undef_object;
}

KIND(empty_dict_kind)
{
	if (empty_dict_kind == self->kind) {
		if (message_kind == msg->kind) {
			struct message * mp = (struct message *)msg;		
			if (mp->name == (OOP)&bind_symbol) {
				if (message_kind == mp->args->kind) {
					mp = (struct message *)mp->args;
					return (OOP)dict_new(mp->name, mp->args, self);
				}
			}
			if (mp->name == (OOP)&lookup_symbol) {
				return &undef_object;
			}
		}
	}
	return &undef_object;
}

struct object empty_dict = { empty_dict_kind };

/*
	Main entry-point
*/
int
main()
{
	TRACE(fprintf(stderr, "&undef_object = %p\n", &undef_object));
	TRACE(fprintf(stderr, "&nil_object = %p\n", &nil_object));

	TRACE(fprintf(stderr, "&_t_symbol = %p\n", &_t_symbol));
	TRACE(fprintf(stderr, "&_f_symbol = %p\n", &_f_symbol));
	TRACE(fprintf(stderr, "&eq_symbol = %p\n", &eq_symbol));
	TRACE(fprintf(stderr, "symbol_kind = %p\n", (void*)symbol_kind));
	TRACE(fprintf(stderr, "eq_symbol.o.kind = %p\n", (void*)eq_symbol.o.kind));
	TRACE(fprintf(stderr, "eq_symbol.s = \"%s\"\n", eq_symbol.s));

	struct message * mp = message_new((OOP)&eq_symbol, (OOP)&_f_symbol);
	TRACE(fprintf(stderr, "mp = %p\n", mp));
	TRACE(fprintf(stderr, "message_kind = %p\n", (void*)message_kind));
	TRACE(fprintf(stderr, "mp->o.kind = %p\n", (void*)mp->o.kind));
	TRACE(fprintf(stderr, "mp->name = %p\n", mp->name));
	TRACE(fprintf(stderr, "mp->args = %p\n", mp->args));

	OOP result = object_call((OOP)&_f_symbol, (OOP)mp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert((OOP)&_t_symbol == result);
	
	struct symbol * x_symbol_ptr = symbol_new("x");
	TRACE(fprintf(stderr, "x_symbol_ptr = %p\n", x_symbol_ptr));
	TRACE(fprintf(stderr, "x_symbol_ptr->s = \"%s\"\n", x_symbol_ptr->s));
	mp = message_new((OOP)&lookup_symbol, (OOP)x_symbol_ptr);
	TRACE(fprintf(stderr, "mp = %p\n", mp));
	TRACE(fprintf(stderr, "mp->o.kind = %p\n", (void*)mp->o.kind));
	TRACE(fprintf(stderr, "mp->name = %p\n", mp->name));
	TRACE(fprintf(stderr, "mp->args = %p\n", mp->args));
	result = object_call(&empty_dict, (OOP)mp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(&undef_object == result);
	
	struct symbol * _42_symbol_ptr = symbol_new("42");
	TRACE(fprintf(stderr, "_42_symbol_ptr = %p\n", _42_symbol_ptr));
	TRACE(fprintf(stderr, "_42_symbol_ptr->s = \"%s\"\n", _42_symbol_ptr->s));
	mp = message_new((OOP)x_symbol_ptr, (OOP)_42_symbol_ptr);
	mp = message_new((OOP)&bind_symbol, (OOP)mp);
	OOP env_ptr = object_call(&empty_dict, (OOP)mp);
	TRACE(fprintf(stderr, "env_ptr = %p\n", env_ptr));
	mp = message_new((OOP)&lookup_symbol, (OOP)x_symbol_ptr);
	result = object_call(env_ptr, (OOP)mp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert((OOP)_42_symbol_ptr == result);

	struct symbol * y_symbol_ptr = symbol_new("y");
	TRACE(fprintf(stderr, "y_symbol_ptr = %p\n", y_symbol_ptr));
	TRACE(fprintf(stderr, "y_symbol_ptr->s = \"%s\"\n", y_symbol_ptr->s));
	mp = message_new((OOP)&lookup_symbol, (OOP)y_symbol_ptr);
	result = object_call(env_ptr, (OOP)mp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(&undef_object == result);

	return 0;
}
