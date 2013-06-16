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
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#undef	_ENABLE_FINGER_TREE_	/**/

#define	TRACE(x)	x		/* enable tracing */

/*
object:
	Object is the root kind, containing just a polymorphic dispatch procedure.
*/

typedef struct object * OOP;
#define KIND(kind) OOP kind(OOP self, va_list args)
typedef OOP (*DISP)(OOP self, va_list args);

struct object {
	DISP			kind;
};

OOP
object_call(OOP obj, ...)
{
	va_list args;

	va_start(args, obj);
	OOP result = (obj->kind)(obj, args);
	va_end(args);
	return result;
}

#define	take_arg()	va_arg(args, OOP)

OOP
object_new(DISP kind, size_t size)
{
	OOP self = (OOP)calloc(1, size);
	self->kind = kind;
	return self;
}
#define	object_alloc(structure, kind)	((structure *)object_new((kind), sizeof(structure)))

KIND(object_kind);

struct object undef_object = { object_kind };
#define	o_undef	(&undef_object)

KIND(object_kind)
{
	return o_undef;
}

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
		/* no object protocol for symbol */
	}
	return o_undef;
}

struct symbol _t_symbol = { { symbol_kind }, "#t" };
#define	o_true	((OOP)&_t_symbol)
struct symbol _f_symbol = { { symbol_kind }, "#f" };
#define	o_false	((OOP)&_f_symbol)

struct symbol eq_p_symbol = { { symbol_kind }, "eq?" };
#define	s_eq_p	((OOP)&eq_p_symbol)
struct symbol give_x_symbol = { { symbol_kind }, "give!" };
#define	s_give_x	((OOP)&give_x_symbol)
struct symbol take_x_symbol = { { symbol_kind }, "take!" };
#define	s_take_x	((OOP)&take_x_symbol)
struct symbol empty_p_symbol = { { symbol_kind }, "empty?" };
#define	s_empty_p	((OOP)&empty_p_symbol)
struct symbol push_symbol = { { symbol_kind }, "push" };
#define	s_push	((OOP)&push_symbol)
struct symbol pop_symbol = { { symbol_kind }, "pop" };
#define	s_pop	((OOP)&pop_symbol)
struct symbol put_symbol = { { symbol_kind }, "put" };
#define	s_put	((OOP)&put_symbol)
struct symbol pull_symbol = { { symbol_kind }, "pull" };
#define	s_pull	((OOP)&pull_symbol)
struct symbol bind_symbol = { { symbol_kind }, "bind" };
#define	s_bind	((OOP)&bind_symbol)
struct symbol lookup_symbol = { { symbol_kind }, "lookup" };
#define	s_lookup	((OOP)&lookup_symbol)
struct symbol add_symbol = { { symbol_kind }, "add" };
#define	s_add	((OOP)&add_symbol)
struct symbol match_symbol = { { symbol_kind }, "match" };
#define	s_match	((OOP)&match_symbol)
struct symbol eval_symbol = { { symbol_kind }, "eval" };
#define	s_eval	((OOP)&eval_symbol)
struct symbol combine_symbol = { { symbol_kind }, "combine" };
#define	s_combine	((OOP)&match_combine)
struct symbol create_x_symbol = { { symbol_kind }, "create!" };
#define	s_create_x	((OOP)&create_x_symbol)
struct symbol send_x_symbol = { { symbol_kind }, "send!" };
#define	s_send_x	((OOP)&send_x_symbol)
struct symbol become_x_symbol = { { symbol_kind }, "become!" };
#define	s_become_x	((OOP)&become_x_symbol)
struct symbol dispatch_x_symbol = { { symbol_kind }, "dispatch!" };
#define	s_dispatch_x	((OOP)&dispatch_x_symbol)

/*
pair:
	A combination of 2 object pointers.  May be treated as a list, terminated by 'o_nil'.

	o' := o.push(x)			-- return a new list with 'x' at the head
	boolean := o.empty?()	-- return true if list is empty, otherwise false
	(x, o') := o.pop()		-- remove 'x' from the head, returning it and the new list
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
//		struct pair * this = as_pair(self);
		OOP cmd = take_arg();
		if (cmd == s_empty_p) {
			return o_false;
		} else if (cmd == s_pop) {
			return self;
		} else if (cmd == s_push) {
			OOP x = take_arg();
			return pair_new(x, self);
		}
	}
	return o_undef;
}

KIND(nil_kind)
{
	if (nil_kind == self->kind) {
		OOP cmd = take_arg();
		if (cmd == s_empty_p) {
			return o_true;
		} else if (cmd == s_push) {
			OOP x = take_arg();
			return pair_new(x, self);
		}
	}
	return o_undef;
}

struct object nil_object = { nil_kind };
#define	o_nil	(&nil_object)

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
		OOP cmd = take_arg();
		if (cmd == s_empty_p) {
			if (this->h == o_nil) {
				return o_true;
			}
			return o_false;
		} else if (cmd == s_take_x) {
			if (this->h != o_nil) {
				struct pair * entry = as_pair(this->h);
				this->h = entry->t;
				OOP item = entry->h;  // entry is garbage after this (use custom free?)
				return item;
			}
		} else if (cmd == s_give_x) {
			OOP item = take_arg();
			OOP oop = pair_new(item, o_nil);  // could be a custom allocator
			if (this->h == o_nil) {
				this->h = oop;
			} else {
				as_pair(this->t)->t = oop;
			}
			this->t = oop;
			return self;
		}
	}
	return o_undef;
}

OOP
queue_new() {
	struct pair * this = object_alloc(struct pair, queue_kind);
	this->h = o_nil;
	this->t = o_nil;
	return (OOP)this;
}

/*
dict:
	Dictionaries define mappings from names to values.
	
	In this implementation, each node holds a single 'name'/'value' pair.
	The 'next' pointer delegates to a linear chain of dictionaries.

	x := o.lookup(name)		-- return value 'x' bound to 'name', or 'o_fail'
	o' := o.bind(name, x)	-- return new dictionary with 'name' bound to 'x'
*/

struct object fail_object = { object_kind };
#define	o_fail	(&fail_object)

struct dict {
	struct object	o;
	OOP				name;
	OOP				value;
	OOP				next;
};

#define	as_dict(oop)	((struct dict *)(oop))

KIND(dict_kind);
KIND(empty_dict_kind);
struct object empty_dict = { empty_dict_kind };
#define	o_empty_dict	(&empty_dict)

OOP
dict_new(OOP name, OOP value, OOP next)
{
	struct dict * this = object_alloc(struct dict, dict_kind);
	this->name = name;
	this->value = value;
	this->next = next;
	return (OOP)this;
}

KIND(empty_dict_kind)
{
	if (empty_dict_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(empty_dict_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_lookup) {
			OOP name = take_arg();
			TRACE(fprintf(stderr, "  %p: name=%p\n", self, name));
			return o_fail;
		} else if (cmd == s_bind) {
			OOP name = take_arg();
			OOP value = take_arg();
			TRACE(fprintf(stderr, "  %p: name=%p value=%p\n", self, name, value));
			return dict_new(name, value, self);
		}
	}
	return o_undef;
}

KIND(dict_kind)
{
	if (dict_kind == self->kind) {
//		struct dict * this = as_dict(self);  -- moved inside "do" loop...
		TRACE(fprintf(stderr, "%p(dict_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_lookup) {
			OOP name = take_arg();
			TRACE(fprintf(stderr, "  %p: name=%p\n", self, name));
			do {
				struct dict * this = as_dict(self);  // init/update "this"
				TRACE(fprintf(stderr, "  %p(dict_kind, %p, %p, %p)\n", this, this->name, this->value, this->next));
				if (name == this->name) {  // NOTE: identity comparison on names
					return this->value;
				}
				self = this->next;  // iterate to simulate tail-recursion
			} while (dict_kind == self->kind);
			return object_call(self, s_lookup, name);  // delegate call
		} else if (cmd == s_bind) {
			OOP name = take_arg();
			OOP value = take_arg();
			TRACE(fprintf(stderr, "  %p: name=%p value=%p\n", self, name, value));
			return dict_new(name, value, self);
		}
	}
	return o_undef;
}

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
		OOP cmd = take_arg();
		if (cmd == s_eq_p) {
			OOP other = take_arg();
			if (other == self) {  // compare identities
				return o_true;
			}
			if (integer_kind == other->kind) {
				struct integer * that = as_integer(other);
				if (that->n == this->n) {  // compare values
					return o_true;
				}
			}
			return o_false;
		} else if (cmd == s_add) {
			OOP other = take_arg();
			if (integer_kind == other->kind) {
				struct integer * that = as_integer(other);
				return integer_new(this->n + that->n);
			}
		}
	}
	return o_undef;
}

struct integer minus_1_integer = { { integer_kind }, -1 };
#define	n_minus_1	((OOP)&minus_1_integer)
struct integer _0_integer = { { integer_kind }, 0 };
#define	n_0	((OOP)&_0_integer)
struct integer _1_integer = { { integer_kind }, 1 };
#define	n_1	((OOP)&_1_integer)
struct integer _2_integer = { { integer_kind }, 2 };
#define	n_2	((OOP)&_2_integer)

#ifdef _ENABLE_FINGER_TREE_
/*
finger:
	Small collections of 1 to 4 items.
   
	o' := o.push(x)			-- return a new list with 'x' at the head
	(x, o') := o.pop()		-- remove 'x' from the head, returning it and the new list
	o' := o.put(x)			-- return a new list with 'x' at the tail
	(x, o') := o.pull()		-- remove 'x' from the tail, returning it and the new list

	push -> [ head ] -> pop
			[ ...  ]
	 put -> [ tail ] -> pull
*/

struct finger {
	struct object	o;
	OOP				item1;
	OOP				item2;
	OOP				item3;
	OOP				item4;
};

#define	as_finger(oop)	((struct finger *)(oop))

KIND(finger1_kind);
KIND(finger2_kind);
KIND(finger3_kind);
KIND(finger4_kind);

OOP
finger1_new(OOP item1) {
	struct finger * this = object_alloc(struct finger, finger1_kind);
	this->item1 = item1;
	return (OOP)this;
}

OOP
finger2_new(OOP item1, OOP item2) {
	struct finger * this = object_alloc(struct finger, finger2_kind);
	this->item1 = item1;
	this->item2 = item2;
	return (OOP)this;
}

OOP
finger3_new(OOP item1, OOP item2, OOP item3) {
	struct finger * this = object_alloc(struct finger, finger3_kind);
	this->item1 = item1;
	this->item2 = item2;
	this->item3 = item3;
	return (OOP)this;
}

OOP
finger4_new(OOP item1, OOP item2, OOP item3, OOP item4) {
	struct finger * this = object_alloc(struct finger, finger4_kind);
	this->item1 = item1;
	this->item2 = item2;
	this->item3 = item3;
	this->item4 = item4;
	return (OOP)this;
}

KIND(finger1_kind)
{
	if (finger1_kind == self->kind) {
		struct finger * this = as_finger(self);
		OOP a = this->item1;
		OOP cmd = take_arg();
		if (cmd == s_put) {
			OOP x = take_arg();
			return finger2_new(a, x);
		} else if (cmd == s_push) {
			OOP x = take_arg();
			return finger2_new(x, a);
		}
	}
	return o_undef;
}

KIND(finger2_kind)
{
	if (finger2_kind == self->kind) {
		struct finger * this = as_finger(self);
		OOP a = this->item1;
		OOP b = this->item2;
		OOP cmd = take_arg();
		if (cmd == s_pop) {
			return pair_new(a, finger1_new(b));
		} else if (cmd == s_pull) {
			return pair_new(b, finger1_new(a));
		} else if (cmd == s_put) {
			OOP x = take_arg();
			return finger3_new(a, b, x);
		} else if (cmd == s_push) {
			OOP x = take_arg();
			return finger3_new(x, a, b);
		}
	}
	return o_undef;
}

KIND(finger3_kind)
{
	if (finger3_kind == self->kind) {
		struct finger * this = as_finger(self);
		OOP a = this->item1;
		OOP b = this->item2;
		OOP c = this->item3;
		OOP cmd = take_arg();
		if (cmd == s_pop) {
			return pair_new(a, finger2_new(b, c));
		} else if (cmd == s_pull) {
			return pair_new(c, finger2_new(a, b));
		} else if (cmd == s_put) {
			OOP x = take_arg();
			return finger4_new(a, b, c, x);
		} else if (cmd == s_push) {
			OOP x = take_arg();
			return finger4_new(x, a, b, c);
		}
	}
	return o_undef;
}

KIND(finger4_kind)
{
	if (finger4_kind == self->kind) {
		struct finger * this = as_finger(self);
		OOP a = this->item1;
		OOP b = this->item2;
		OOP c = this->item3;
		OOP d = this->item4;
		OOP cmd = take_arg();
		if (cmd == s_pop) {
			return pair_new(a, finger3_new(b, c, d));
		} else if (cmd == s_pull) {
			return pair_new(d, finger3_new(a, b, c));
		}
	}
	return o_undef;
}

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
	OOP args = take_arg();
	if (ft_zero_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger2_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item1 == s_put) {
				return (OOP)ft_one_new(mfp->item2);
			}
			if (mfp->item1 == s_push) {
				return (OOP)ft_one_new(mfp->item2);
			}
		}
	}
	return o_undef;
}

KIND(ft_one_kind)
{
	OOP args = take_arg();
	if (ft_one_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger1_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item1 == s_pop) {
				return pair_new(this->mid, (OOP)&empty_ft);
			}
			if (mfp->item1 == s_pull) {
				return pair_new(this->mid, (OOP)&empty_ft);
			}
		}
		if (finger2_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item1 == s_put) {
				return (OOP)ft_many_new(
						(OOP)ft_one_new(this->mid), 
						(OOP)&empty_ft, 
						(OOP)ft_one_new(mfp->item2));
			}
			if (mfp->item1 == s_push) {
				return (OOP)ft_many_new(
						(OOP)ft_one_new(mfp->item2), 
						(OOP)&empty_ft, 
						(OOP)ft_one_new(this->mid));
			}
		}
	}
	return o_undef;
}

KIND(ft_many_kind)
{
	OOP args = take_arg();
	if (ft_many_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger1_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item1 == s_pop) {
				if (finger1_kind == this->left->kind) {
					struct finger * fp = (struct finger *)this->left;
					if (this->mid == (OOP)&empty_ft) {
						if (finger1_kind == this->right->kind) {
							struct finger * rfp = (struct finger *)this->right;
							return pair_new(
									fp->item1,
									(OOP)ft_one_new(rfp->item1));
						} else {
							struct pair * pp = (struct pair *)object_call_1(this->right, args);  // delegate to right
							OOP left = (OOP)finger1_new(pp->h);
							return pair_new(
									fp->item1,
									(OOP)ft_many_new(left, this->mid, pp->t));
						}
					} else {
						struct pair * pp = (struct pair *)object_call_1(this->mid, args);  // delegate to mid
						return pair_new(
								fp->item1,
								(OOP)ft_many_new(pp->h, pp->t, this->right));
					}
				} else {
					struct pair * pp = (struct pair *)object_call_1(this->left, args);  // delegate to left
					return pair_new(
							pp->h,
							(OOP)ft_many_new(pp->t, this->mid, this->right));
				}
			}
			if (mfp->item1 == s_pull) {
				return o_undef; // [NOT IMPLEMENTED]
			}
		}
		if (finger2_kind == args->kind) {
			struct finger * mfp = (struct finger *)args;
			if (mfp->item1 == s_put) {
				if (finger4_kind == this->right->kind) {
					struct finger * fp = (struct finger *)this->right;
					OOP mid = object_call_2(this->mid,
							s_put, (OOP)finger3_new(fp->item1, fp->item2, fp->item3));
					OOP right = (OOP)finger2_new(fp->item4, mfp->item2);
					return (OOP)ft_many_new(this->left, mid, right);
				} else {
					OOP right = object_call(this->right, args);  // delegate to right
					return (OOP)ft_many_new(this->left, this->mid, right);
				}
			}
			if (mfp->item1 == s_push) {
				return o_undef; // [NOT IMPLEMENTED]
			}
		}
	}
	return o_undef;
}
#endif /* _ENABLE_FINGER_TREE_ */

/*
match:
	Matches represent composable contexts for pattern matching.

	(in, env, out) -> (in', env', out') | #fail
*/

struct match {
	struct object	o;
	OOP				in;			// position in input value
	OOP				env;		// dictionary of identifier bindings
	OOP				out;		// semantic value (output)
};

#define	as_match(oop)	((struct match *)(oop))

KIND(match_kind);

OOP
match_new(OOP in, OOP env, OOP out)
{
	struct match * this = object_alloc(struct match, match_kind);
	this->in = in;
	this->env = env;
	this->out = out;
	return (OOP)this;
}

KIND(match_kind)
{
	/* no object protocol */
	return o_undef;
}

/*
pattern:
	Patterns are used to match structured values, possibly binding identifiers to the components.

	match' := o.apply(match)	-- return the result of applying pattern to 'match', or 'o_fail'
*/

/* LET fail = \in.(#fail, in) */
KIND(fail_pattern_kind)
{
	if (fail_pattern_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(fail_pattern_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			return o_fail;
		}
	}
	return o_undef;
}
struct object fail_pattern = { fail_pattern_kind };
#define	ptrn_fail	(&fail_pattern)

/* LET empty = \in.(#ok, (), in) */
KIND(empty_pattern_kind)
{
	if (empty_pattern_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(empty_pattern_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			return match_new(mp->in, mp->env, o_nil);
		}
	}
	return o_undef;
}
struct object empty_pattern = { empty_pattern_kind };
#define	ptrn_empty	(&empty_pattern)

/* LET any = \in.(
	CASE in OF
	() : (#fail, in)
	(token, rest) : (#ok, token, rest)
	END
) */
KIND(any_pattern_kind)
{
	if (any_pattern_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(any_pattern_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			if (object_call(mp->in, s_empty_p) == o_false) {
				struct pair * pp = as_pair(object_call(mp->in, s_pop));
				return match_new(pp->t, mp->env, pp->h);
			}
			return o_fail;
		}
	}
	return o_undef;
}
struct object any_pattern = { any_pattern_kind };
#define	ptrn_any	(&any_pattern)

/* LET eq(value) = \in.(
	CASE in OF
	() : (#fail, in)
	($value, rest) : (#ok, value, rest)
	_ : (#fail, in)
	END
) */
struct eq_pattern {
	struct object	o;
	OOP				value;		// value to match
};
#define	as_eq_pattern(oop)	((struct eq_pattern *)(oop))
KIND(eq_pattern_kind)
{
	if (eq_pattern_kind == self->kind) {
		struct eq_pattern * this = as_eq_pattern(self);
		TRACE(fprintf(stderr, "%p(eq_pattern_kind, %p)\n", this, this->value));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			if (object_call(mp->in, s_empty_p) == o_false) {
				struct pair * pp = as_pair(object_call(mp->in, s_pop));
				if (object_call(this->value, s_eq_p, pp->h) == o_true) {
					return match_new(pp->t, mp->env, pp->h);
				}
			}
			return o_fail;
		}
	}
	return o_undef;
}
OOP
eq_pattern_new(OOP value)
{
	struct eq_pattern * this = object_alloc(struct eq_pattern, eq_pattern_kind);
	this->value = value;
	return (OOP)this;
}

/* LET if(test) = \in.(
	CASE in OF
	() : (#fail, in)
	(token, rest) : (
		CASE test(token) OF
		TRUE : (#ok, token, rest)
		_ : (#fail, in)
		END
	)
	END
) */
struct if_pattern {
	struct object	o;
	OOP				test;		// predicate to test tokens
};
#define	as_if_pattern(oop)	((struct if_pattern *)(oop))
KIND(if_pattern_kind)
{
	if (if_pattern_kind == self->kind) {
		struct if_pattern * this = as_if_pattern(self);
		TRACE(fprintf(stderr, "%p(if_pattern_kind, %p)\n", this, this->test));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			if (object_call(mp->in, s_empty_p) == o_false) {
				struct pair * pp = as_pair(object_call(mp->in, s_pop));
				if (object_call(this->test, pp->h) == o_true) {		// FIXME: IS THIS THE RIGHT PROTOCOL FOR PREDICATE FUNCTIONS?
					return match_new(pp->t, mp->env, pp->h);
				}
			}
			return o_fail;
		}
	}
	return o_undef;
}
OOP
if_pattern_new(OOP test)
{
	struct if_pattern * this = object_alloc(struct if_pattern, if_pattern_kind);
	this->test = test;
	return (OOP)this;
}

/* LET or(left, right) = \in.(
	CASE left(in) OF
	(#ok, value, in') : (#ok, value, in')
	(#fail, in') : right(in)
	END
) */
struct or_pattern {
	struct object	o;
	OOP				head;		// first pattern
	OOP				tail;		// second pattern
};
#define	as_or_pattern(oop)	((struct or_pattern *)(oop))
KIND(or_pattern_kind)
{
	if (or_pattern_kind == self->kind) {
//		struct or_pattern * this = as_or_pattern(self);  -- moved inside loop...
		TRACE(fprintf(stderr, "%p(or_pattern_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			do {
				struct or_pattern * this = as_or_pattern(self);
				TRACE(fprintf(stderr, "%p(or_pattern_kind, %p, %p)\n", this, this->head, this->tail));
				OOP match1 = object_call(this->head, s_match, match);
				if (match_kind == match1->kind) {
					return match1;  // success
				}
				self = this->tail;  // simulate tail-recursion
			} while (or_pattern_kind == self->kind);
			return object_call(self, s_match, match);
		}
	}
	return o_undef;
}
OOP
or_pattern_new(OOP head, OOP tail)
{
	struct or_pattern * this = object_alloc(struct or_pattern, or_pattern_kind);
	this->head = head;
	this->tail = tail;
	return (OOP)this;
}

/* LET and(left, right) = \in.(
	CASE left(in) OF
	(#fail, in') : (#fail, in)
	(#ok, value, in') : (
		CASE right(in') OF
		(#fail, in'') : (#fail, in)
		(#ok, value', in'') : (#ok, (value, value'), in'')
		END
	)
	END
) */
struct and_pattern {
	struct object	o;
	OOP				head;		// first pattern
	OOP				tail;		// second pattern
};
#define	as_and_pattern(oop)	((struct and_pattern *)(oop))
KIND(and_pattern_kind)
{
	if (and_pattern_kind == self->kind) {
		struct and_pattern * this = as_and_pattern(self);
		TRACE(fprintf(stderr, "%p(and_pattern_kind, %p, %p)\n", this, this->head, this->tail));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			OOP match1 = object_call(this->head, s_match, match);
			if (match_kind == match1->kind) {
				struct match * mp1 = as_match(match1);
				OOP match2 = object_call(this->tail, s_match, match1);
				if (match_kind == match2->kind) {
					struct match * mp2 = as_match(match2);
					OOP out = pair_new(mp1->out, mp2->out);
					return match_new(mp2->in, mp2->env, out);
				}
			}
			return o_fail;
		}
	}
	return o_undef;
}
OOP
and_pattern_new(OOP head, OOP tail)
{
	struct and_pattern * this = object_alloc(struct and_pattern, and_pattern_kind);
	this->head = head;
	this->tail = tail;
	return (OOP)this;
}

/* LET bind(name, ptrn) = \(value, env, in).(
	CASE ptrn(value, env, in) OF
	(#ok, value', env', in') : (#ok, value', env'.bind(name, value'), in')
	(#fail, value', env', in') : (#fail, value, env, in)
	END
) */
struct bind_pattern {
	struct object	o;
	OOP				name;		// identifier to bind
	OOP				ptrn;		// pattern to match
};
#define	as_bind_pattern(oop)	((struct bind_pattern *)(oop))
KIND(bind_pattern_kind)
{
	if (bind_pattern_kind == self->kind) {
		struct bind_pattern * this = as_bind_pattern(self);
		TRACE(fprintf(stderr, "%p(bind_pattern_kind, %p, %p)\n", this, this->name, this->ptrn));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			OOP match1 = object_call(this->ptrn, s_match, match);
			if (match_kind == match1->kind) {
				struct match * mp1 = as_match(match1);
				OOP env = object_call(mp1->env, s_bind, this->name, mp1->out);
				return match_new(mp1->in, env, mp1->out);
			}
			return o_fail;
		}
	}
	return o_undef;
}
OOP
bind_pattern_new(OOP name, OOP ptrn)
{
	struct bind_pattern * this = object_alloc(struct bind_pattern, bind_pattern_kind);
	this->name = name;
	this->ptrn = ptrn;
	return (OOP)this;
}

/*
input = "2#1011.\n"
digit = '0' | '1'
digits = digit digits | digit
grammar = any '#' digits
*/

/* LET alt(matches) = (
	CASE matches OF
	() : fail
	(first, rest) : or(first, alt(rest))
	last : last
	END
) */

/* LET seq(matches) = (
	CASE matches OF
	() : empty
	(first, rest) : and(first, seq(rest))
	last : last
	END
) */

/* LET opt(match) = or(match, empty) */

/* LET star(match) = \in.((opt(and(match, star(match))))(in)) */

/* LET plus(match) = and(match, star(match)) */

/* LET not(match) = \in.(
	CASE match(in) OF
	(#ok, value, in') : (#fail, in)
	(#fail, in') : (#ok, (), in)
	END
) */

/* LET peek(match) = not(not(match)) */

/*
expr:
	Expressions represent procedures for computing a value.
	
	value := o.eval(env)		-- return result of evaluating this expression in environment 'env'
*/

KIND(const_expr_kind)
{
	if (const_expr_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(const_expr_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			//...
		}
	}
	return o_undef;
}


/*
actor:
	Actors encapsulate state/behavior reacting to asynchronous events (messages).
	
	NOTE: Actor objects are not intended to be called directly, but if they are
		they simply delegate to their behavior 'beh'
*/

struct actor {
	struct object	o;
	OOP				beh;
};

#define	as_actor(oop)	((struct actor *)(oop))

KIND(actor_kind);

OOP
actor_new(OOP beh)
{
	struct actor * this = object_alloc(struct actor, actor_kind);
	this->beh = beh;
	return (OOP)this;
}

KIND(actor_kind)
{
	if (actor_kind == self->kind) {
		struct actor * this = as_actor(self);
		TRACE(fprintf(stderr, "%p(actor_kind, %p)\n", this, this->beh));
		OOP event = take_arg();
		TRACE(fprintf(stderr, "  %p: event=%p\n", this, event));
		return object_call(this->beh, event);
	}
	return o_false;  // failure?
}

/*
event:
	Events are unique occurances of a particular 'msg' for a specific 'actor'.
	When the message is delivered to the actor, the effects are held in the event.
	The event acts as the "sponsor" for the computation, providing resources to the actor.
	
	actor := o.create!(beh)	-- return a new actor with initial behavior 'beh'
	o.send!(actor, message)	-- send 'message' to 'actor' asynchronously
	o.become!(beh)			-- use behavior 'beh' to process subsequent messages
	o.dispatch!()			-- deliver 'msg' to 'actor'
*/

struct event {
	struct object	o;
	OOP				actor;		// target actor
	OOP				msg;		// message to deliver
	OOP				actors;		// actors created
	OOP				events;		// messages sent
	OOP				beh;		// replacement behavior
};

#define	as_event(oop)	((struct event *)(oop))

KIND(event_kind);

OOP
event_new(OOP actor, OOP msg)
{
	struct event * this = object_alloc(struct event, event_kind);
	this->actor = actor;
	this->msg = msg;
	return (OOP)this;
}

KIND(event_kind)
{
	if (event_kind == self->kind) {
		struct event * this = as_event(self);
		TRACE(fprintf(stderr, "%p event_kind {actor:%p, msg:%p}\n", this, this->actor, this->msg));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", this, cmd, as_symbol(cmd)->s));
		if (cmd == s_dispatch_x) {
			OOP beh = as_actor(this->actor)->beh;
			TRACE(fprintf(stderr, "  %p: dispatch {beh:%p}\n", this, beh));
			this->actors = o_nil;  // empty actor stack
			this->events = o_nil;  // empty event stack
			this->beh = beh;
			return object_call(beh, this);  // invoke actor behavior
		} else if (cmd == s_create_x) {
			OOP beh = take_arg();
			TRACE(fprintf(stderr, "  %p: create {beh:%p}\n", this, beh));
			OOP actor = actor_new(beh);
			this->actors = object_call(this->actors, s_push, actor);  // add actor to stack
			return actor;
		} else if (cmd == s_send_x) {
			OOP actor = take_arg();
			OOP msg = take_arg();
			TRACE(fprintf(stderr, "  %p: send {actor:%p, msg:%p}\n", this, actor, msg));
			OOP event = event_new(actor, msg);
			this->events = object_call(this->events, s_push, event);  // add event to stack
			return event;
		} else if (cmd == s_become_x) {
			OOP beh = take_arg();
			TRACE(fprintf(stderr, "  %p: become {beh:%p}\n", this, beh));
			this->beh = beh;
			return beh;
		}
	}
	return o_false;  // failure?
}

/*
behavior:
	Behaviors cause "effects" for an actor, using a sponsor

	o.(event)	-- cause effects triggered by 'event'
*/

KIND(empty_beh_kind)
{
	TRACE(fprintf(stderr, "%p(empty_beh_kind)\n", self));
	OOP evt = take_arg();
	OOP act = as_event(evt)->actor;
	OOP msg = as_event(evt)->msg;
	TRACE(fprintf(stderr, "  %p: event=%p {actor:%p, msg:%p}\n", self, evt, act, msg));
	return o_true;  // success?
}

struct object empty_beh = { empty_beh_kind };
#define	beh_empty	(&empty_beh)

struct actor sink_actor = { { actor_kind }, beh_empty };
#define	a_sink	((OOP)&sink_actor)

struct forward_beh {
	struct object	o;
	OOP				target;		// messages are forwarded to 'target' actor
};

#define	as_forward_beh(oop)	((struct forward_beh *)(oop))

KIND(forward_beh_kind);

OOP
forward_beh_new(OOP target)
{
	struct forward_beh * this = object_alloc(struct forward_beh, forward_beh_kind);
	this->target = target;
	return (OOP)this;
}

KIND(forward_beh_kind)
{
	if (forward_beh_kind == self->kind) {
		struct forward_beh * this = as_forward_beh(self);
		TRACE(fprintf(stderr, "%p forward_beh_kind {target:%p}\n", this, this->target));
		OOP evt = take_arg();
		OOP act = as_event(evt)->actor;
		OOP msg = as_event(evt)->msg;
		TRACE(fprintf(stderr, "  %p: event=%p {actor:%p, msg:%p}\n", self, evt, act, msg));
		object_call(evt, s_send_x, this->target, msg);
		return o_true;  // success?
	}
	return o_false;  // failure?
}

KIND(one_shot_beh_kind);

OOP
one_shot_beh_new(OOP target)
{
	struct forward_beh * this = object_alloc(struct forward_beh, one_shot_beh_kind);
	this->target = target;
	return (OOP)this;
}

KIND(one_shot_beh_kind)
{
	if (one_shot_beh_kind == self->kind) {
		struct forward_beh * this = as_forward_beh(self);
		TRACE(fprintf(stderr, "%p one_shot_beh_kind {target:%p}\n", this, this->target));

		OOP evt = take_arg();
		OOP act = as_event(evt)->actor;
		OOP msg = as_event(evt)->msg;
		TRACE(fprintf(stderr, "  %p: event=%p {actor:%p, msg:%p}\n", self, evt, act, msg));
		object_call(evt, s_send_x, this->target, msg);
		object_call(evt, s_become_x, beh_empty);  // BECOME sink_beh
		return o_true;  // success?
	}
	return o_false;  // failure?
}

/*
config:
	Configurations are collections of actors and in-flight message-events.
	
	remain := o.dispatch!(count)-- dispatch up to 'count' events, return how many 'remain'
	remain := o.give!(event)	-- add 'event' to the queue of in-flight events
*/

struct config {
	struct object	o;
	OOP				events;		// queue of message-events to be delivered
	OOP				remain;		// number of queued message-events
};

#define	as_config(oop)	((struct config *)(oop))

KIND(config_kind);

OOP
config_new()
{
	struct config * this = object_alloc(struct config, config_kind);
	this->events = queue_new();
	this->remain = n_0;
	return (OOP)this;
}

KIND(config_kind)
{
	if (config_kind == self->kind) {
		struct config * this = as_config(self);
		TRACE(fprintf(stderr, "%p config_kind {remain:%d}\n", this, as_integer(this->remain)->n));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", this, cmd, as_symbol(cmd)->s));
		if (cmd == s_give_x) {
			// enqueue an event
			OOP evt = take_arg();
			TRACE(fprintf(stderr, "  %p: give! {event:%p}\n", this, evt));
			object_call(this->events, s_give_x, evt);
			this->remain = object_call(this->remain, s_add, n_1);
			TRACE(fprintf(stderr, "  %p: remain=%d\n", this, as_integer(this->remain)->n));
			return this->remain;
		} else if (cmd == s_dispatch_x) {
			// dispatch up to 'count' events
			OOP count = take_arg();
			TRACE(fprintf(stderr, "  %p: dispatch {count:%d}\n", this, as_integer(count)->n));
			while ((object_call(count, s_eq_p, n_0) != o_true)
			&&     (object_call(this->events, s_empty_p) == o_false)) {
				// dequeue next event
				OOP evt = object_call(this->events, s_take_x);
				TRACE(fprintf(stderr, "  %p: event=%p\n", this, evt));
				this->remain = object_call(this->remain, s_add, n_minus_1);
				TRACE(fprintf(stderr, "  %p: remain=%d\n", this, as_integer(this->remain)->n));
				// dispatch event
				OOP result = object_call(evt, s_dispatch_x);
				TRACE(fprintf(stderr, "  %p: result=%p\n", this, result));
				// apply result
				assert(o_true == result);
				struct event * ep = as_event(evt);
				OOP events = ep->events;
				while (o_nil != events) {  // enqueue events
					struct pair * pp = as_pair(object_call(events, s_pop));
					TRACE(fprintf(stderr, "  %p: events=[%p]^%p\n", this, pp->h, pp->t));
					object_call(self, s_give_x, pp->h);
					events = pp->t;
				}
				as_actor(ep->actor)->beh = ep->beh;  // replace behavior
				TRACE(fprintf(stderr, "  %p: beh=%p\n", this, ep->beh));
				// decrement count
				count = object_call(count, s_add, n_minus_1);
				TRACE(fprintf(stderr, "  %p: count=%d\n", this, as_integer(count)->n));
			}
			return this->remain;
		}
	}
	return o_undef;
}

/*
	Unit tests
*/
void
run_tests()
{
	TRACE(fprintf(stderr, "o_undef = %p\n", o_undef));
	TRACE(fprintf(stderr, "o_fail = %p\n", o_fail));
	TRACE(fprintf(stderr, "o_nil = %p\n", o_nil));

	TRACE(fprintf(stderr, "o_true = %p\n", o_true));
	TRACE(fprintf(stderr, "o_false = %p\n", o_false));
	TRACE(fprintf(stderr, "s_eq_p = %p\n", s_eq_p));
	TRACE(fprintf(stderr, "&eq_p_symbol = %p\n", &eq_p_symbol));
	TRACE(fprintf(stderr, "symbol_kind = %p\n", (void*)symbol_kind));
	TRACE(fprintf(stderr, "eq_p_symbol.o.kind = %p\n", (void*)eq_p_symbol.o.kind));
	TRACE(fprintf(stderr, "eq_p_symbol.s = \"%s\"\n", eq_p_symbol.s));

#ifdef _ENABLE_FINGER_TREE_
	TRACE(fprintf(stderr, "finger1_kind = %p\n", (void*)finger1_kind));
	TRACE(fprintf(stderr, "finger2_kind = %p\n", (void*)finger2_kind));
	TRACE(fprintf(stderr, "finger3_kind = %p\n", (void*)finger3_kind));
	TRACE(fprintf(stderr, "finger4_kind = %p\n", (void*)finger4_kind));
/*
	struct integer * ch_A = integer_new('A');
	struct integer * ch_Z = integer_new('Z');
	OOP ft_deque_ptr = (OOP)&empty_ft;
	TRACE(fprintf(stderr, "&empty_ft = %p\n", &empty_ft));
	struct integer * n_ch = ch_A;
	struct finger * eq_fp = finger2_new(s_eq_p, (OOP)ch_Z);
	struct finger * inc_fp = finger2_new(s_add, n_1);
	while (object_call((OOP)n_ch, (OOP)eq_fp) != o_true) {
		struct finger * fp = finger2_new(s_put, (OOP)n_ch);
		TRACE(fprintf(stderr, "fp = %p\n", fp));
		TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
			fp->item1, fp->item2, fp->item3, fp->item4));
		TRACE(fprintf(stderr, "ft_deque_ptr = %p ^ [%c]\n", ft_deque_ptr, n_ch->n));
		ft_deque_ptr = object_call(ft_deque_ptr, (OOP)fp);
		n_ch = (struct integer *)object_call((OOP)n_ch, (OOP)inc_fp);
	}
	n_ch = ch_A;
	struct finger * dec_fp = finger2_new(s_add, n_minus_1);
	struct finger * pop_fp = finger1_new(s_pop);
	while (object_call((OOP)n_ch, (OOP)eq_fp) != o_true) {
		struct pair * pp = (struct pair *)object_call(ft_deque_ptr, (OOP)pop_fp);
		struct integer * ip = (struct integer *)pp->h;
		ft_deque_ptr = pp->t;
		TRACE(fprintf(stderr, "ft_deque_ptr = [%c] ^ %p\n", ip->n, ft_deque_ptr));
		fp = finger2_new(s_eq_p, (OOP)n_ch);
		result = object_call((OOP)ip, (OOP)fp);
		assert(o_true == result);
		n_ch = (struct integer *)object_call((OOP)n_ch, (OOP)dec_fp);
	}
*/	
#endif /* _ENABLE_FINGER_TREE_ */

	TRACE(fprintf(stderr, "dict_kind = %p\n", (void*)dict_kind));
	TRACE(fprintf(stderr, "s_lookup = %p\n", s_lookup));
	TRACE(fprintf(stderr, "s_bind = %p\n", s_bind));

	OOP s_x = symbol_new("x");
	TRACE(fprintf(stderr, "s_x = %p\n", s_x));
	TRACE(fprintf(stderr, "as_symbol(s_x)->s = \"%s\"\n", as_symbol(s_x)->s));
	OOP result = object_call(o_empty_dict, s_lookup, s_x);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(o_fail == result);
	
	OOP n_42 = integer_new(42);
	TRACE(fprintf(stderr, "n_42 = %p\n", n_42));
	TRACE(fprintf(stderr, "as_integer(n_42)->n = %d\n", as_integer(n_42)->n));
	OOP d_env = object_call(o_empty_dict, s_bind, s_x, n_42);
	TRACE(fprintf(stderr, "d_env = %p\n", d_env));
	result = object_call(d_env, s_lookup, s_x);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(n_42 == result);

	OOP s_y = symbol_new("y");
	TRACE(fprintf(stderr, "s_y = %p\n", s_y));
	TRACE(fprintf(stderr, "as_symbol(s_y)->s = \"%s\"\n", as_symbol(s_y)->s));
	d_env = object_call(d_env, s_bind, s_y, n_minus_1);
	TRACE(fprintf(stderr, "d_env = %p\n", d_env));

	OOP s_z = symbol_new("z");
	TRACE(fprintf(stderr, "s_z = %p\n", s_z));
	TRACE(fprintf(stderr, "as_symbol(s_z)->s = \"%s\"\n", as_symbol(s_z)->s));
	result = object_call(d_env, s_lookup, s_z);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(o_fail == result);

	OOP ch_A = integer_new('A');
	OOP ch_Z = integer_new('Z');
	OOP q_oop = queue_new();
	TRACE(fprintf(stderr, "q_oop = %p\n", q_oop));
	result = object_call(q_oop, s_empty_p);
	assert(o_true == result);
	OOP n_ch = ch_A;
	while (object_call(n_ch, s_eq_p, ch_Z) != o_true) {
		q_oop = object_call(q_oop, s_give_x, n_ch);
		TRACE(fprintf(stderr, "q_oop = %p ^ [%c]\n", q_oop, as_integer(n_ch)->n));
		n_ch = object_call(n_ch, s_add, n_1);
	}
	result = object_call(q_oop, s_empty_p);
	assert(o_false == result);
	n_ch = ch_A;
	while (object_call(n_ch, s_eq_p, ch_Z) != o_true) {
		OOP n_i = object_call(q_oop, s_take_x);
		TRACE(fprintf(stderr, "q_oop = [%c] ^ %p\n", as_integer(n_i)->n, q_oop));
		result = object_call(n_i, s_eq_p, n_ch);
		assert(o_true == result);
		n_ch = object_call(n_ch, s_add, n_1);
	}
	result = object_call(q_oop, s_empty_p);
	assert(o_true == result);

	TRACE(fprintf(stderr, "s_create_x = %p\n", s_create_x));
	TRACE(fprintf(stderr, "s_send_x = %p\n", s_send_x));
	TRACE(fprintf(stderr, "s_become_x = %p\n", s_become_x));
	TRACE(fprintf(stderr, "s_dispatch_x = %p\n", s_dispatch_x));

	OOP cfg = config_new();
	// dispatch empty queue
	result = object_call(cfg, s_dispatch_x, n_0);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(o_true == object_call(result, s_eq_p, n_0));
	// enqueue test event(s)
//	OOP a_fwd = actor_new(forward_beh_new(a_sink));
	OOP a_fwd = actor_new(one_shot_beh_new(a_sink));
	TRACE(fprintf(stderr, "a_fwd = %p\n", a_fwd));
	OOP event = event_new(a_fwd, n_42);
	TRACE(fprintf(stderr, "event = %p\n", event));
	result = object_call(cfg, s_give_x, event);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(o_true == object_call(result, s_eq_p, n_1));
	event = event_new(a_fwd, n_1);
	TRACE(fprintf(stderr, "event = %p\n", event));
	result = object_call(cfg, s_give_x, event);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(o_true == object_call(result, s_eq_p, n_2));
	// dispatch up to 5 events
	OOP n_5 = integer_new(5);
	result = object_call(cfg, s_dispatch_x, n_5);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(o_true == object_call(result, s_eq_p, n_0));
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
