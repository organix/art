/*

pair.h -- Actor Run-Time

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
#ifndef _PAIR_H_
#define _PAIR_H_

#include "art.h"
#include "object.h"
#include "symbol.h"

/*
 * pair / stack
 */

extern struct symbol empty_p_symbol;
#define	s_empty_p	((OOP)&empty_p_symbol)
extern struct symbol push_symbol;
#define	s_push	((OOP)&push_symbol)
extern struct symbol pop_symbol;
#define	s_pop	((OOP)&pop_symbol)
extern struct symbol put_symbol;
#define	s_put	((OOP)&put_symbol)
extern struct symbol pull_symbol;
#define	s_pull	((OOP)&pull_symbol)

extern KIND(nil_kind);
extern struct object nil_object;
#define	o_nil	((OOP)&nil_object)

struct pair {
	struct object	o;
	OOP				h;
	OOP				t;
};
#define	as_pair(oop)	((struct pair *)(oop))
extern OOP		pair_new(OOP h, OOP t);
extern KIND(pair_kind);

/*
 * queue
 */

extern struct symbol give_x_symbol;
#define	s_give_x	((OOP)&give_x_symbol)
extern struct symbol take_x_symbol;
#define	s_take_x	((OOP)&take_x_symbol)

extern OOP		queue_new();
extern KIND(queue_kind);

/*
 * dictionary
 */

extern struct symbol bind_symbol;
#define	s_bind	((OOP)&bind_symbol)
extern struct symbol lookup_symbol;
#define	s_lookup	((OOP)&lookup_symbol)

extern struct object fail_object;
#define	o_fail	((OOP)&fail_object)

extern KIND(empty_dict_kind);
extern struct object empty_dict;
#define	o_empty_dict	((OOP)&empty_dict)

struct dict {
	struct object	o;
	OOP				name;
	OOP				value;
	OOP				next;
};
#define	as_dict(oop)	((struct dict *)(oop))
extern OOP		dict_new(OOP name, OOP value, OOP next);
extern KIND(dict_kind);

/*
 * integer
 */

extern struct symbol eq_p_symbol;
#define	s_eq_p	((OOP)&eq_p_symbol)
extern struct symbol add_symbol;
#define	s_add	((OOP)&add_symbol)

struct integer {
	struct object	o;
	int				n;
};
#define	as_integer(oop)	((struct integer *)(oop))
extern OOP		integer_new(int value);
extern KIND(integer_kind);

extern struct integer minus_1_integer;
#define	n_minus_1	((OOP)&minus_1_integer)
extern struct integer _0_integer;
#define	n_0	((OOP)&_0_integer)
extern struct integer _1_integer;
#define	n_1	((OOP)&_1_integer)
extern struct integer _2_integer;
#define	n_2	((OOP)&_2_integer)

#endif /* _PAIR_H_ */
