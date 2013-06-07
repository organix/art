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

OOP
object_call(OOP obj, OOP msg)
{
	return (obj->kind)(obj, msg);
}

struct object nil_object = { object_kind };

/*
pair:
	A combination of 2 object pointers.
*/

struct pair {
	struct object	o;
	OOP				h;
	OOP				t;
};

KIND(pair_kind);

struct pair *
pair_new(OOP h, OOP t)
{
	struct pair * oop = (struct pair *)object_new(pair_kind, sizeof(struct pair));
	oop->h = h;
	oop->t = t;
	return oop;
}

struct symbol name_symbol;
struct symbol args_symbol;

KIND(pair_kind)
{
	if (pair_kind == self->kind) {
		struct pair * oop = (struct pair *)self;
		/* no message protocol for pair */
	}
	return &undef_object;
}

/*
finger:
	Small collections of 1 to 4 items.
*/

struct finger {
	struct object	o;
	OOP				item_1;
	OOP				item_2;
	OOP				item_3;
	OOP				item_4;
};

KIND(finger_1_kind);
KIND(finger_2_kind);
KIND(finger_3_kind);
KIND(finger_4_kind);

struct finger *
finger_1_new(OOP item_1) {
	struct finger * oop = (struct finger *)object_new(finger_1_kind, sizeof(struct finger));
	oop->item_1 = item_1;
	return oop;
}

struct finger *
finger_2_new(OOP item_1, OOP item_2) {
	struct finger * oop = (struct finger *)object_new(finger_2_kind, sizeof(struct finger));
	oop->item_1 = item_1;
	oop->item_2 = item_2;
	return oop;
}

struct finger *
finger_3_new(OOP item_1, OOP item_2, OOP item_3) {
	struct finger * oop = (struct finger *)object_new(finger_3_kind, sizeof(struct finger));
	oop->item_1 = item_1;
	oop->item_2 = item_2;
	oop->item_3 = item_3;
	return oop;
}

struct finger *
finger_4_new(OOP item_1, OOP item_2, OOP item_3, OOP item_4) {
	struct finger * oop = (struct finger *)object_new(finger_4_kind, sizeof(struct finger));
	oop->item_1 = item_1;
	oop->item_2 = item_2;
	oop->item_3 = item_3;
	oop->item_4 = item_4;
	return oop;
}

struct symbol put_symbol;
struct symbol pop_symbol;
struct symbol push_symbol;
struct symbol pull_symbol;

KIND(finger_1_kind)
{
	if (finger_1_kind == self->kind) {
		struct finger * oop = (struct finger *)self;
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)finger_2_new(oop->item_1, mfp->item_2);
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)finger_2_new(mfp->item_2, oop->item_1);
			}
		}
	}
	return &undef_object;
}

KIND(finger_2_kind)
{
	if (finger_2_kind == self->kind) {
		struct finger * oop = (struct finger *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				return (OOP)pair_new(oop->item_1, 
						(OOP)finger_1_new(oop->item_2));
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return (OOP)pair_new(oop->item_2, 
						(OOP)finger_1_new(oop->item_1));
			}
		}
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)finger_3_new(oop->item_1, oop->item_2, mfp->item_2);
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)finger_3_new(mfp->item_2, oop->item_1, oop->item_2);
			}
		}
	}
	return &undef_object;
}

KIND(finger_3_kind)
{
	if (finger_3_kind == self->kind) {
		struct finger * oop = (struct finger *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				return (OOP)pair_new(oop->item_1, 
						(OOP)finger_2_new(oop->item_2, oop->item_3));
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return (OOP)pair_new(oop->item_3, 
						(OOP)finger_2_new(oop->item_1, oop->item_2));
			}
		}
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)finger_4_new(oop->item_1, oop->item_2, oop->item_3, mfp->item_2);
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)finger_4_new(mfp->item_2, oop->item_1, oop->item_2, oop->item_3);
			}
		}
	}
	return &undef_object;
}

KIND(finger_4_kind)
{
	if (finger_4_kind == self->kind) {
		struct finger * oop = (struct finger *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				return (OOP)pair_new(oop->item_1, 
						(OOP)finger_3_new(oop->item_2, oop->item_3, oop->item_4));
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return (OOP)pair_new(oop->item_4, 
						(OOP)finger_3_new(oop->item_1, oop->item_2, oop->item_3));
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
struct symbol put_symbol = { { symbol_kind }, "put" };
struct symbol pop_symbol = { { symbol_kind }, "pop" };
struct symbol push_symbol = { { symbol_kind }, "push" };
struct symbol pull_symbol = { { symbol_kind }, "pull" };
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
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&eq_symbol) {
				if (mfp->item_2 == self) {
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
	
	In this implementation, each node holds a single 'name'/'value' pair.
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
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&lookup_symbol) {
				if (mfp->item_2 == oop->name) {  // [FIXME] consider using "eq" message here
					return oop->value;
				}
				return object_call(oop->next, msg);
			}
		}
		if (finger_3_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&bind_symbol) {
				return (OOP)dict_new(mfp->item_2, mfp->item_3, self);
			}
		}
	}
	return &undef_object;
}

KIND(empty_dict_kind)
{
	if (empty_dict_kind == self->kind) {
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&lookup_symbol) {
				return &undef_object;
			}
		}
		if (finger_3_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&bind_symbol) {
				return (OOP)dict_new(mfp->item_2, mfp->item_3, self);
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

	TRACE(fprintf(stderr, "finger_1_kind = %p\n", (void*)finger_1_kind));
	TRACE(fprintf(stderr, "finger_2_kind = %p\n", (void*)finger_2_kind));
	TRACE(fprintf(stderr, "finger_3_kind = %p\n", (void*)finger_3_kind));
	TRACE(fprintf(stderr, "finger_4_kind = %p\n", (void*)finger_4_kind));

	struct finger * fp = finger_2_new((OOP)&eq_symbol, (OOP)&_f_symbol);
	TRACE(fprintf(stderr, "fp = %p\n", fp));
	TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
		fp->item_1, fp->item_2, fp->item_3, fp->item_4));
	OOP result = object_call((OOP)&_f_symbol, (OOP)fp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert((OOP)&_t_symbol == result);
	
	struct symbol * x_symbol_ptr = symbol_new("x");
	TRACE(fprintf(stderr, "x_symbol_ptr = %p\n", x_symbol_ptr));
	TRACE(fprintf(stderr, "x_symbol_ptr->s = \"%s\"\n", x_symbol_ptr->s));
	fp = finger_2_new((OOP)&lookup_symbol, (OOP)x_symbol_ptr);
	TRACE(fprintf(stderr, "fp = %p\n", fp));
	TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
		fp->item_1, fp->item_2, fp->item_3, fp->item_4));
	result = object_call(&empty_dict, (OOP)fp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(&undef_object == result);
	
	struct symbol * _42_symbol_ptr = symbol_new("42");
	TRACE(fprintf(stderr, "_42_symbol_ptr = %p\n", _42_symbol_ptr));
	TRACE(fprintf(stderr, "_42_symbol_ptr->s = \"%s\"\n", _42_symbol_ptr->s));
	fp = finger_3_new((OOP)&bind_symbol, (OOP)x_symbol_ptr, (OOP)_42_symbol_ptr);
	TRACE(fprintf(stderr, "fp = %p\n", fp));
	TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
		fp->item_1, fp->item_2, fp->item_3, fp->item_4));
	OOP env_ptr = object_call(&empty_dict, (OOP)fp);
	TRACE(fprintf(stderr, "env_ptr = %p\n", env_ptr));
	fp = finger_2_new((OOP)&lookup_symbol, (OOP)x_symbol_ptr);
	result = object_call(env_ptr, (OOP)fp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert((OOP)_42_symbol_ptr == result);

	struct symbol * y_symbol_ptr = symbol_new("y");
	TRACE(fprintf(stderr, "y_symbol_ptr = %p\n", y_symbol_ptr));
	TRACE(fprintf(stderr, "y_symbol_ptr->s = \"%s\"\n", y_symbol_ptr->s));
	fp = finger_2_new((OOP)&lookup_symbol, (OOP)y_symbol_ptr);
	result = object_call(env_ptr, (OOP)fp);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(&undef_object == result);

	return 0;
}
