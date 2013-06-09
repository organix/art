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
#define KIND(kind) OOP kind(OOP self, OOP args)
typedef OOP (*DPROC)(OOP self, OOP args);

struct object {
	DPROC			kind;
};

OOP
object_new(DPROC kind, size_t size)
{
	OOP self = (OOP)calloc(1, size);
	self->kind = kind;
	return self;
}
#define	object_alloc(structure, kind)	((structure *)object_new((kind), sizeof(structure)))

KIND(object_kind)
{
	return self;
}

struct object undef_object = { object_kind };
#define	undef_oop	(&undef_object)
struct object nil_object = { object_kind };
#define	nil_oop	(&nil_object)

#define	object_call(obj, args)	(((obj)->kind)((obj), (args)))

#define	object_call_1(obj, arg_1)	\
	object_call((obj), finger_1_new((arg_1)))
#define	object_call_2(obj, arg_1, arg_2)	\
	object_call((obj), finger_2_new((arg_1), (arg_2)))
#define	object_call_3(obj, arg_1, arg_2, arg_3)	\
	object_call((obj), finger_3_new((arg_1), (arg_2), (arg_3)))
#define	object_call_4(obj, arg_1, arg_2, arg_3, arg_4)	\
	object_call((obj), finger_4_new((arg_1), (arg_2), (arg_3), (arg_4)))

/*
symbol:
	Symbols are constants with a string representation 's'.
*/

struct symbol {
	struct object	o;
	char *			s;
};

#define	as_symbol(oop)	((struct symbol *)(oop))

KIND(symbol_kind);

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
		/* no message protocol for symbol */
	}
	return undef_oop;
}

struct symbol _t_symbol = { { symbol_kind }, "#t" };
#define	_t_oop	((OOP)&_t_symbol)
struct symbol _f_symbol = { { symbol_kind }, "#f" };
#define	_f_oop	((OOP)&_f_symbol)
struct symbol eq_p_symbol = { { symbol_kind }, "eq?" };
#define	eq_p_oop	((OOP)&eq_p_symbol)
struct symbol give_x_symbol = { { symbol_kind }, "give!" };
#define	give_x_oop	((OOP)&give_x_symbol)
struct symbol take_x_symbol = { { symbol_kind }, "take!" };
#define	take_x_oop	((OOP)&take_x_symbol)
struct symbol empty_p_symbol = { { symbol_kind }, "empty?" };
#define	empty_p_oop	((OOP)&empty_p_symbol)
struct symbol push_symbol = { { symbol_kind }, "push" };
#define	push_oop	((OOP)&push_symbol)
struct symbol pop_symbol = { { symbol_kind }, "pop" };
#define	pop_oop	((OOP)&pop_symbol)
struct symbol put_symbol = { { symbol_kind }, "put" };
#define	put_oop	((OOP)&put_symbol)
struct symbol pull_symbol = { { symbol_kind }, "pull" };
#define	pull_oop	((OOP)&pull_symbol)
struct symbol bind_symbol = { { symbol_kind }, "bind" };
#define	bind_oop	((OOP)&bind_symbol)
struct symbol lookup_symbol = { { symbol_kind }, "lookup" };
#define	lookup_oop	((OOP)&lookup_symbol)
struct symbol add_symbol = { { symbol_kind }, "add" };
#define	add_oop	((OOP)&add_symbol)

/*
pair:
	A combination of 2 object pointers.
*/

struct pair {
	struct object	o;
	OOP				h;
	OOP				t;
};

#define	as_pair(oop)	((struct pair *)(oop))

KIND(pair_kind);

OOP
pair_new(OOP h, OOP t)
{
	struct pair * this = object_alloc(struct pair, pair_kind);
	this->h = h;
	this->t = t;
	return (OOP)this;
}

KIND(pair_kind)
{
	if (pair_kind == self->kind) {
		struct pair * this = as_pair(self);
		/* no message protocol for pair */
	}
	return undef_oop;
}

/*
finger:
	Small collections of 1 to 4 items.
   
	o' := o.push(x)
	(x, o') := o.pop()
	o' := o.put(x)
	(x, o') := o.pull()

	push -> [ head ] -> pop
			[ ...  ]
	 put -> [ tail ] -> pull
*/

struct finger {
	struct object	o;
	OOP				item_1;
	OOP				item_2;
	OOP				item_3;
	OOP				item_4;
};

#define	as_finger(oop)	((struct finger *)(oop))

KIND(finger_1_kind);
KIND(finger_2_kind);
KIND(finger_3_kind);
KIND(finger_4_kind);

OOP
finger_1_new(OOP item_1) {
	struct finger * this = object_alloc(struct finger, finger_1_kind);
	this->item_1 = item_1;
	return (OOP)this;
}

OOP
finger_2_new(OOP item_1, OOP item_2) {
	struct finger * this = object_alloc(struct finger, finger_2_kind);
	this->item_1 = item_1;
	this->item_2 = item_2;
	return (OOP)this;
}

OOP
finger_3_new(OOP item_1, OOP item_2, OOP item_3) {
	struct finger * this = object_alloc(struct finger, finger_3_kind);
	this->item_1 = item_1;
	this->item_2 = item_2;
	this->item_3 = item_3;
	return (OOP)this;
}

OOP
finger_4_new(OOP item_1, OOP item_2, OOP item_3, OOP item_4) {
	struct finger * this = object_alloc(struct finger, finger_4_kind);
	this->item_1 = item_1;
	this->item_2 = item_2;
	this->item_3 = item_3;
	this->item_4 = item_4;
	return (OOP)this;
}

KIND(finger_1_kind)
{
	if (finger_1_kind == self->kind) {
		struct finger * this = as_finger(self);
		if (finger_2_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == put_oop) {
				return finger_2_new(this->item_1, ap->item_2);
			}
			if (ap->item_1 == push_oop) {
				return finger_2_new(ap->item_2, this->item_1);
			}
		}
	}
	return undef_oop;
}

KIND(finger_2_kind)
{
	if (finger_2_kind == self->kind) {
		struct finger * this = as_finger(self);
		if (finger_1_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == pop_oop) {
				return pair_new(this->item_1, 
						finger_1_new(this->item_2));
			}
			if (ap->item_1 == pull_oop) {
				return pair_new(this->item_2, 
						finger_1_new(this->item_1));
			}
		}
		if (finger_2_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == put_oop) {
				return finger_3_new(this->item_1, this->item_2, ap->item_2);
			}
			if (ap->item_1 == push_oop) {
				return finger_3_new(ap->item_2, this->item_1, this->item_2);
			}
		}
	}
	return undef_oop;
}

KIND(finger_3_kind)
{
	if (finger_3_kind == self->kind) {
		struct finger * this = as_finger(self);
		if (finger_1_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == pop_oop) {
				return pair_new(this->item_1, 
						finger_2_new(this->item_2, this->item_3));
			}
			if (ap->item_1 == pull_oop) {
				return pair_new(this->item_3, 
						finger_2_new(this->item_1, this->item_2));
			}
		}
		if (finger_2_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == put_oop) {
				return finger_4_new(this->item_1, this->item_2, this->item_3, ap->item_2);
			}
			if (ap->item_1 == push_oop) {
				return finger_4_new(ap->item_2, this->item_1, this->item_2, this->item_3);
			}
		}
	}
	return undef_oop;
}

KIND(finger_4_kind)
{
	if (finger_4_kind == self->kind) {
		struct finger * this = as_finger(self);
		if (finger_1_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == pop_oop) {
				return pair_new(this->item_1, 
						finger_3_new(this->item_2, this->item_3, this->item_4));
			}
			if (ap->item_1 == pull_oop) {
				return pair_new(this->item_4, 
						finger_3_new(this->item_1, this->item_2, this->item_3));
			}
		}
	}
	return undef_oop;
}

/*
queue:
	An efficient mutable (stateful) queue data-structure built from pairs.

	item := o.take!()		-- remove and return 'item' from the head of the queue
	boolean := o.empty?()	-- return true if queue is empty, otherwise false
	o := o.give!(item)		-- add 'item' to the tail of the queue
*/

KIND(queue_kind)
{
	if (queue_kind == self->kind) {
		struct pair * this = as_pair(self);
		if (finger_1_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == empty_p_oop) {
				if (this->h == nil_oop) {
					return _t_oop;
				}
				return _f_oop;
			}
			if (ap->item_1 == take_x_oop) {
				if (this->h != nil_oop) {
					struct pair * entry = as_pair(this->h);
					this->h = entry->t;
					return entry->h;
				}
			}
		}
		if (finger_2_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == give_x_oop) {
				OOP oop = pair_new(ap->item_2, nil_oop);
				if (this->h == nil_oop) {
					this->h = oop;
				} else {
					as_pair(this->t)->t = oop;
				}
				this->t = oop;
				return self;
			}
		}
	}
	return undef_oop;
}

OOP
queue_new() {
	struct pair * this = object_alloc(struct pair, queue_kind);
	this->h = nil_oop;
	this->t = nil_oop;
	return (OOP)this;
}

/*
dict:
	Dictionaries define mappings from names to values.
	
	In this implementation, each node holds a single 'name'/'value' pair.
	The 'next' pointer delegates to a linear chain of dictionaries.

	x := o.lookup(name)		-- return value 'x' bound to 'name', or undefined
	o' := o.bind(name, x)	-- return new dictionary with 'name' bound to 'x'
*/

struct dict {
	struct object	o;
	OOP				name;
	OOP				value;
	OOP				next;
};

#define	as_dict(oop)	((struct dict *)(oop))

KIND(dict_kind);

OOP
dict_new(OOP name, OOP value, OOP next)
{
	struct dict * this = object_alloc(struct dict, dict_kind);
	this->name = name;
	this->value = value;
	this->next = next;
	return (OOP)this;
}

KIND(dict_kind)
{
	TRACE(fprintf(stderr, "%p(dict_kind).[%p %p %p %p]\n", self, as_finger(args)->item_1, as_finger(args)->item_2, as_finger(args)->item_3, as_finger(args)->item_4));
	if (dict_kind == self->kind) {
		struct dict * this = as_dict(self);
		if (finger_2_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == lookup_oop) {
				if (ap->item_2 == this->name) {  // NOTE: identity comparison on names
					return this->value;
				}
				return object_call(this->next, args);  // delegate to next entry
			}
		}
		if (finger_3_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == bind_oop) {
				return dict_new(ap->item_2, ap->item_3, self);
			}
		}
	}
	return undef_oop;
}

KIND(empty_dict_kind)
{
	TRACE(fprintf(stderr, "%p(empty_dict_kind).[%p %p %p %p]\n", self, as_finger(args)->item_1, as_finger(args)->item_2, as_finger(args)->item_3, as_finger(args)->item_4));
	if (empty_dict_kind == self->kind) {
		if (finger_2_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == lookup_oop) {
				return undef_oop;
			}
		}
		if (finger_3_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == bind_oop) {
				return dict_new(ap->item_2, ap->item_3, self);
			}
		}
	}
	return undef_oop;
}

struct object empty_dict = { empty_dict_kind };

/*
integer:
	Integers are constants with a numeric representation 'n'.

	boolean := o.eq?(x)		-- return true if 'o' is equal to 'x', otherwise false
	integer := o.add(x)		-- return new integer equal to ('o' + 'x')
*/

struct integer {
	struct object	o;
	int				n;
};

#define	as_integer(oop)	((struct integer *)(oop))

KIND(integer_kind);

OOP
integer_new(int value)
{
	struct integer * this = object_alloc(struct integer, integer_kind);
	this->n = value;
	return (OOP)this;
}

KIND(integer_kind)
{
	if (integer_kind == self->kind) {
		struct integer * this = as_integer(self);
		if (finger_2_kind == args->kind) {
			struct finger * ap = as_finger(args);
			if (ap->item_1 == eq_p_oop) {
				OOP other = ap->item_2;
				if (other == self) {  // compare identities
					return _t_oop;
				}
				if (integer_kind == other->kind) {
					struct integer * that = as_integer(other);
					if (that->n == this->n) {  // compare values
						return _t_oop;
					}
				}
				return _f_oop;
			}
			if (ap->item_1 == add_oop) {
				OOP other = ap->item_2;
				if (integer_kind == other->kind) {
					struct integer * that = as_integer(other);
					return integer_new(this->n + that->n);
				}
			}
		}
	}
	return undef_oop;
}

struct integer minus_1_integer = { { integer_kind }, -1 };
#define	minus_1_oop	((OOP)&minus_1_integer)
struct integer _0_integer = { { integer_kind }, 0 };
#define	_0_oop	((OOP)&_0_integer)
struct integer _1_integer = { { integer_kind }, 1 };
#define	_1_oop	((OOP)&_1_integer)

/*
finger tree:
	An efficient functional deque data-structure of arbitrary size.
*/

struct finger_tree {
	struct object	o;
	OOP				left;
	OOP				mid;
	OOP				right;
};

KIND(ft_zero_kind);
KIND(ft_one_kind);
KIND(ft_many_kind);

struct object empty_ft = { ft_zero_kind };

struct finger_tree *
ft_one_new(OOP item) {
	struct finger_tree * this = object_alloc(struct finger_tree, ft_one_kind);
	this->mid = item;
	return this;
}

struct finger_tree *
ft_many_new(OOP left, OOP mid, OOP right) {
	struct finger_tree * this = object_alloc(struct finger_tree, ft_many_kind);
	this->left = left;
	this->mid = mid;
	this->right = right;
	return this;
}

KIND(ft_zero_kind)
{
	if (ft_zero_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger_2_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item_1 == put_oop) {
				return (OOP)ft_one_new(mfp->item_2);
			}
			if (mfp->item_1 == push_oop) {
				return (OOP)ft_one_new(mfp->item_2);
			}
		}
	}
	return undef_oop;
}

KIND(ft_one_kind)
{
	if (ft_one_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger_1_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item_1 == pop_oop) {
				return pair_new(this->mid, (OOP)&empty_ft);
			}
			if (mfp->item_1 == pull_oop) {
				return pair_new(this->mid, (OOP)&empty_ft);
			}
		}
		if (finger_2_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item_1 == put_oop) {
				return (OOP)ft_many_new(
						(OOP)ft_one_new(this->mid), 
						(OOP)&empty_ft, 
						(OOP)ft_one_new(mfp->item_2));
			}
			if (mfp->item_1 == push_oop) {
				return (OOP)ft_many_new(
						(OOP)ft_one_new(mfp->item_2), 
						(OOP)&empty_ft, 
						(OOP)ft_one_new(this->mid));
			}
		}
	}
	return undef_oop;
}

KIND(ft_many_kind)
{
	if (ft_many_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger_1_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item_1 == pop_oop) {
				if (finger_1_kind == this->left->kind) {
					struct finger * fp = (struct finger *)this->left;
					if (this->mid == (OOP)&empty_ft) {
						if (finger_1_kind == this->right->kind) {
							struct finger * rfp = (struct finger *)this->right;
							return pair_new(
									fp->item_1,
									(OOP)ft_one_new(rfp->item_1));
						} else {
							struct pair * pp = (struct pair *)object_call(this->right, args);  // delegate to right
							OOP left = (OOP)finger_1_new(pp->h);
							return pair_new(
									fp->item_1,
									(OOP)ft_many_new(left, this->mid, pp->t));
						}
					} else {
						struct pair * pp = (struct pair *)object_call(this->mid, args);  // delegate to mid
						return pair_new(
								fp->item_1,
								(OOP)ft_many_new(pp->h, pp->t, this->right));
					}
				} else {
					struct pair * pp = (struct pair *)object_call(this->left, args);  // delegate to left
					return pair_new(
							pp->h,
							(OOP)ft_many_new(pp->t, this->mid, this->right));
				}
			}
			if (mfp->item_1 == pull_oop) {
				return undef_oop; // [NOT IMPLEMENTED]
			}
		}
		if (finger_2_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item_1 == put_oop) {
				if (finger_4_kind == this->right->kind) {
					struct finger * fp = (struct finger *)this->right;
					OOP mid = object_call_2(this->mid,
							put_oop, (OOP)finger_3_new(fp->item_1, fp->item_2, fp->item_3));
					OOP right = (OOP)finger_2_new(fp->item_4, mfp->item_2);
					return (OOP)ft_many_new(this->left, mid, right);
				} else {
					OOP right = object_call(this->right, args);  // delegate to right
					return (OOP)ft_many_new(this->left, this->mid, right);
				}
			}
			if (mfp->item_1 == push_oop) {
				return undef_oop; // [NOT IMPLEMENTED]
			}
		}
	}
	return undef_oop;
}

/*
	Unit tests
*/
void
run_tests()
{
	TRACE(fprintf(stderr, "undef_oop = %p\n", undef_oop));
	TRACE(fprintf(stderr, "nil_oop = %p\n", nil_oop));

	TRACE(fprintf(stderr, "_t_oop = %p\n", _t_oop));
	TRACE(fprintf(stderr, "_f_oop = %p\n", _f_oop));
	TRACE(fprintf(stderr, "eq_p_oop = %p\n", eq_p_oop));
	TRACE(fprintf(stderr, "&eq_p_symbol = %p\n", &eq_p_symbol));
	TRACE(fprintf(stderr, "symbol_kind = %p\n", (void*)symbol_kind));
	TRACE(fprintf(stderr, "eq_p_symbol.o.kind = %p\n", (void*)eq_p_symbol.o.kind));
	TRACE(fprintf(stderr, "eq_p_symbol.s = \"%s\"\n", eq_p_symbol.s));

	TRACE(fprintf(stderr, "finger_1_kind = %p\n", (void*)finger_1_kind));
	TRACE(fprintf(stderr, "finger_2_kind = %p\n", (void*)finger_2_kind));
	TRACE(fprintf(stderr, "finger_3_kind = %p\n", (void*)finger_3_kind));
	TRACE(fprintf(stderr, "finger_4_kind = %p\n", (void*)finger_4_kind));

	TRACE(fprintf(stderr, "dict_kind = %p\n", (void*)dict_kind));
	TRACE(fprintf(stderr, "lookup_oop = %p\n", lookup_oop));
	TRACE(fprintf(stderr, "bind_oop = %p\n", bind_oop));

	OOP x_oop = symbol_new("x");
	TRACE(fprintf(stderr, "x_oop = %p\n", x_oop));
	TRACE(fprintf(stderr, "as_symbol(x_oop)->s = \"%s\"\n", as_symbol(x_oop)->s));
	OOP args_oop = finger_2_new(lookup_oop, x_oop);
	TRACE(fprintf(stderr, "args_oop = %p\n", args_oop));
	struct finger * fp = as_finger(args_oop);
	TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
		fp->item_1, fp->item_2, fp->item_3, fp->item_4));
	OOP result = object_call(&empty_dict, args_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(undef_oop == result);
	
	OOP _42_oop = integer_new(42);
	TRACE(fprintf(stderr, "_42_oop = %p\n", _42_oop));
	TRACE(fprintf(stderr, "as_integer(_42_oop)->n = %d\n", as_integer(_42_oop)->n));
	OOP env_oop = object_call_3(&empty_dict, bind_oop, x_oop, _42_oop);
	TRACE(fprintf(stderr, "env_oop = %p\n", env_oop));
	result = object_call_2(env_oop, lookup_oop, x_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(_42_oop == result);

	OOP y_oop = symbol_new("y");
	TRACE(fprintf(stderr, "y_oop = %p\n", y_oop));
	TRACE(fprintf(stderr, "as_symbol(y_oop)->s = \"%s\"\n", as_symbol(y_oop)->s));
	result = object_call_2(env_oop, lookup_oop, y_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(undef_oop == result);

/*
	struct integer * a_integer_oop = integer_new('A');
	struct integer * z_integer_oop = integer_new('Z');
	OOP ft_deque_ptr = (OOP)&empty_ft;
	TRACE(fprintf(stderr, "&empty_ft = %p\n", &empty_ft));
	struct integer * n_integer_oop = a_integer_oop;
	struct finger * eq_fp = finger_2_new(eq_p_oop, (OOP)z_integer_oop);
	struct finger * inc_fp = finger_2_new(add_oop, _1_oop);
	while (object_call((OOP)n_integer_oop, (OOP)eq_fp) != _t_oop) {
		struct finger * fp = finger_2_new(put_oop, (OOP)n_integer_oop);
		TRACE(fprintf(stderr, "fp = %p\n", fp));
		TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
			fp->item_1, fp->item_2, fp->item_3, fp->item_4));
		TRACE(fprintf(stderr, "ft_deque_ptr = %p ^ [%c]\n", ft_deque_ptr, n_integer_oop->n));
		ft_deque_ptr = object_call(ft_deque_ptr, (OOP)fp);
		n_integer_oop = (struct integer *)object_call((OOP)n_integer_oop, (OOP)inc_fp);
	}
	n_integer_oop = a_integer_oop;
	struct finger * dec_fp = finger_2_new(add_oop, minus_1_oop);
	struct finger * pop_fp = finger_1_new(pop_oop);
	while (object_call((OOP)n_integer_oop, (OOP)eq_fp) != _t_oop) {
		struct pair * pp = (struct pair *)object_call(ft_deque_ptr, (OOP)pop_fp);
		struct integer * ip = (struct integer *)pp->h;
		ft_deque_ptr = pp->t;
		TRACE(fprintf(stderr, "ft_deque_ptr = [%c] ^ %p\n", ip->n, ft_deque_ptr));
		fp = finger_2_new(eq_p_oop, (OOP)n_integer_oop);
		result = object_call((OOP)ip, (OOP)fp);
		assert(_t_oop == result);
		n_integer_oop = (struct integer *)object_call((OOP)n_integer_oop, (OOP)dec_fp);
	}
*/	

	OOP a_integer_oop = integer_new('A');
	OOP z_integer_oop = integer_new('Z');
	OOP queue_oop = queue_new();
	TRACE(fprintf(stderr, "queue_oop = %p\n", queue_oop));
	result = object_call_1(queue_oop, empty_p_oop);
	assert(_t_oop == result);
	OOP n_integer_oop = a_integer_oop;
	while (object_call_2(n_integer_oop, eq_p_oop, z_integer_oop) != _t_oop) {
		queue_oop = object_call_2(queue_oop, give_x_oop, n_integer_oop);
		TRACE(fprintf(stderr, "queue_oop = %p ^ [%c]\n", queue_oop, as_integer(n_integer_oop)->n));
		n_integer_oop = object_call_2(n_integer_oop, add_oop, _1_oop);
	}
	result = object_call_1(queue_oop, empty_p_oop);
	assert(_f_oop == result);
	n_integer_oop = a_integer_oop;
	while (object_call_2(n_integer_oop, eq_p_oop, z_integer_oop) != _t_oop) {
		OOP i_integer_oop = object_call_1(queue_oop, take_x_oop);
		TRACE(fprintf(stderr, "queue_oop = [%c] ^ %p\n", as_integer(i_integer_oop)->n, queue_oop));
		result = object_call_2(i_integer_oop, eq_p_oop, n_integer_oop);
		assert(_t_oop == result);
		n_integer_oop = object_call_2(n_integer_oop, add_oop, _1_oop);
	}
	result = object_call_1(queue_oop, empty_p_oop);
	assert(_t_oop == result);
}

/*
	Main entry-point
*/
int
main()
{
	run_tests();
	return 0;
}
