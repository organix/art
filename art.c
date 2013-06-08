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
	OOP self = (OOP)calloc(1, size);
	self->kind = kind;
	return self;
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

#define	object_call_1(obj, arg_1)	\
	object_call((obj), (OOP)finger_1_new((arg_1)))
#define	object_call_2(obj, arg_1, arg_2)	\
	object_call((obj), (OOP)finger_2_new((arg_1), (arg_2)))
#define	object_call_3(obj, arg_1, arg_2, arg_3)	\
	object_call((obj), (OOP)finger_3_new((arg_1), (arg_2), (arg_3)))
#define	object_call_4(obj, arg_1, arg_2, arg_3, arg_4)	\
	object_call((obj), (OOP)finger_4_new((arg_1), (arg_2), (arg_3), (arg_4)))

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
	struct pair * this = (struct pair *)object_new(pair_kind, sizeof(struct pair));
	this->h = h;
	this->t = t;
	return this;
}

KIND(pair_kind)
{
	if (pair_kind == self->kind) {
		struct pair * this = (struct pair *)self;
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
	struct finger * this = (struct finger *)object_new(finger_1_kind, sizeof(struct finger));
	this->item_1 = item_1;
	return this;
}

struct finger *
finger_2_new(OOP item_1, OOP item_2) {
	struct finger * this = (struct finger *)object_new(finger_2_kind, sizeof(struct finger));
	this->item_1 = item_1;
	this->item_2 = item_2;
	return this;
}

struct finger *
finger_3_new(OOP item_1, OOP item_2, OOP item_3) {
	struct finger * this = (struct finger *)object_new(finger_3_kind, sizeof(struct finger));
	this->item_1 = item_1;
	this->item_2 = item_2;
	this->item_3 = item_3;
	return this;
}

struct finger *
finger_4_new(OOP item_1, OOP item_2, OOP item_3, OOP item_4) {
	struct finger * this = (struct finger *)object_new(finger_4_kind, sizeof(struct finger));
	this->item_1 = item_1;
	this->item_2 = item_2;
	this->item_3 = item_3;
	this->item_4 = item_4;
	return this;
}

struct symbol put_symbol;
struct symbol pop_symbol;
struct symbol push_symbol;
struct symbol pull_symbol;

KIND(finger_1_kind)
{
	if (finger_1_kind == self->kind) {
		struct finger * this = (struct finger *)self;
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)finger_2_new(this->item_1, mfp->item_2);
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)finger_2_new(mfp->item_2, this->item_1);
			}
		}
	}
	return &undef_object;
}

KIND(finger_2_kind)
{
	if (finger_2_kind == self->kind) {
		struct finger * this = (struct finger *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				return (OOP)pair_new(this->item_1, 
						(OOP)finger_1_new(this->item_2));
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return (OOP)pair_new(this->item_2, 
						(OOP)finger_1_new(this->item_1));
			}
		}
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)finger_3_new(this->item_1, this->item_2, mfp->item_2);
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)finger_3_new(mfp->item_2, this->item_1, this->item_2);
			}
		}
	}
	return &undef_object;
}

KIND(finger_3_kind)
{
	if (finger_3_kind == self->kind) {
		struct finger * this = (struct finger *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				return (OOP)pair_new(this->item_1, 
						(OOP)finger_2_new(this->item_2, this->item_3));
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return (OOP)pair_new(this->item_3, 
						(OOP)finger_2_new(this->item_1, this->item_2));
			}
		}
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)finger_4_new(this->item_1, this->item_2, this->item_3, mfp->item_2);
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)finger_4_new(mfp->item_2, this->item_1, this->item_2, this->item_3);
			}
		}
	}
	return &undef_object;
}

KIND(finger_4_kind)
{
	if (finger_4_kind == self->kind) {
		struct finger * this = (struct finger *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				return (OOP)pair_new(this->item_1, 
						(OOP)finger_3_new(this->item_2, this->item_3, this->item_4));
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return (OOP)pair_new(this->item_4, 
						(OOP)finger_3_new(this->item_1, this->item_2, this->item_3));
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
struct symbol eq_p_symbol = { { symbol_kind }, "eq?" };
struct symbol give_symbol = { { symbol_kind }, "give" };
struct symbol take_symbol = { { symbol_kind }, "take" };
struct symbol empty_p_symbol = { { symbol_kind }, "empty?" };
struct symbol put_symbol = { { symbol_kind }, "put" };
struct symbol pop_symbol = { { symbol_kind }, "pop" };
struct symbol push_symbol = { { symbol_kind }, "push" };
struct symbol pull_symbol = { { symbol_kind }, "pull" };
struct symbol bind_symbol = { { symbol_kind }, "bind" };
struct symbol lookup_symbol = { { symbol_kind }, "lookup" };
struct symbol add_symbol = { { symbol_kind }, "add" };

/*
  push-->[ head ]-->pop
         [ ...  ]
   put-->[ tail ]-->pull
   
  d' := d.put(x)
  (x, d') := d.pop()

  - := q.give(x)
  x := q.take()
*/

struct symbol *
symbol_new(char * name)
{
	struct symbol * this = (struct symbol *)object_new(symbol_kind, sizeof(struct symbol));
	this->s = name;
	return this;
}

KIND(symbol_kind)
{
	if (symbol_kind == self->kind) {
		struct symbol * this = (struct symbol *)self;
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&eq_p_symbol) {
				if (mfp->item_2 == self) {  // compare identities
					return (OOP)&_t_symbol;
				}
				return (OOP)&_f_symbol;
			}
		}
	}
	return &undef_object;
}

/*
integer:
	Integers are constants with a numeric representation 'n'.
*/

struct integer {
	struct object	o;
	int				n;
};

KIND(integer_kind);

struct integer minus_1_integer = { { integer_kind }, -1 };
struct integer _0_integer = { { integer_kind }, 0 };
struct integer _1_integer = { { integer_kind }, 1 };

struct integer *
integer_new(int value)
{
	struct integer * this = (struct integer *)object_new(integer_kind, sizeof(struct integer));
	this->n = value;
	return this;
}

KIND(integer_kind)
{
	if (integer_kind == self->kind) {
		struct integer * this = (struct integer *)self;
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&eq_p_symbol) {
				OOP other = mfp->item_2;
				if (other == self) {  // compare identities
					return (OOP)&_t_symbol;
				}
				if (integer_kind == other->kind) {
					struct integer * that = (struct integer *)other;
					if (that->n == this->n) {  // compare values
						return (OOP)&_t_symbol;
					}
				}
				return (OOP)&_f_symbol;
			}
			if (mfp->item_1 == (OOP)&add_symbol) {
				OOP other = mfp->item_2;
				if (integer_kind == other->kind) {
					struct integer * that = (struct integer *)other;
					return (OOP)integer_new(this->n + that->n);
				}
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

KIND(empty_dict_kind);
KIND(dict_kind);

struct object empty_dict = { empty_dict_kind };

struct dict *
dict_new(OOP name, OOP value, OOP next)
{
	struct dict * this = (struct dict *)object_new(dict_kind, sizeof(struct dict));
	this->name = name;
	this->value = value;
	this->next = next;
	return this;
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

KIND(dict_kind)
{
	if (dict_kind == self->kind) {
		struct dict * this = (struct dict *)self;
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&lookup_symbol) {
				if (mfp->item_2 == this->name) {  // [FIXME] consider using "eq" message here
					return this->value;
				}
				return object_call(this->next, msg);  // delegate to next entry
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
	struct finger_tree * this = 
		(struct finger_tree *)object_new(ft_one_kind, sizeof(struct finger_tree));
	this->mid = item;
	return this;
}

struct finger_tree *
ft_many_new(OOP left, OOP mid, OOP right) {
	struct finger_tree * this = 
		(struct finger_tree *)object_new(ft_many_kind, sizeof(struct finger_tree));
	this->left = left;
	this->mid = mid;
	this->right = right;
	return this;
}

KIND(ft_zero_kind)
{
	if (ft_zero_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)ft_one_new(mfp->item_2);
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)ft_one_new(mfp->item_2);
			}
		}
	}
	return &undef_object;
}

KIND(ft_one_kind)
{
	if (ft_one_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				return (OOP)pair_new(this->mid, (OOP)&empty_ft);
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return (OOP)pair_new(this->mid, (OOP)&empty_ft);
			}
		}
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				return (OOP)ft_many_new(
						(OOP)ft_one_new(this->mid), 
						(OOP)&empty_ft, 
						(OOP)ft_one_new(mfp->item_2));
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return (OOP)ft_many_new(
						(OOP)ft_one_new(mfp->item_2), 
						(OOP)&empty_ft, 
						(OOP)ft_one_new(this->mid));
			}
		}
	}
	return &undef_object;
}

KIND(ft_many_kind)
{
	if (ft_many_kind == self->kind) {
		struct finger_tree * this = (struct finger_tree *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&pop_symbol) {
				if (finger_1_kind == this->left->kind) {
					struct finger * fp = (struct finger *)this->left;
					if (this->mid == (OOP)&empty_ft) {
						if (finger_1_kind == this->right->kind) {
							struct finger * rfp = (struct finger *)this->right;
							return (OOP)pair_new(
									fp->item_1,
									(OOP)ft_one_new(rfp->item_1));
						} else {
							struct pair * pp = (struct pair *)object_call(this->right, msg);  // delegate to right
							OOP left = (OOP)finger_1_new(pp->h);
							return (OOP)pair_new(
									fp->item_1,
									(OOP)ft_many_new(left, this->mid, pp->t));
						}
					} else {
						struct pair * pp = (struct pair *)object_call(this->mid, msg);  // delegate to mid
						return (OOP)pair_new(
								fp->item_1,
								(OOP)ft_many_new(pp->h, pp->t, this->right));
					}
				} else {
					struct pair * pp = (struct pair *)object_call(this->left, msg);  // delegate to left
					return (OOP)pair_new(
							pp->h,
							(OOP)ft_many_new(pp->t, this->mid, this->right));
				}
			}
			if (mfp->item_1 == (OOP)&pull_symbol) {
				return &undef_object; // [NOT IMPLEMENTED]
			}
		}
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&put_symbol) {
				if (finger_4_kind == this->right->kind) {
					struct finger * fp = (struct finger *)this->right;
					OOP mid = object_call_2(this->mid,
							(OOP)&put_symbol, (OOP)finger_3_new(fp->item_1, fp->item_2, fp->item_3));
					OOP right = (OOP)finger_2_new(fp->item_4, mfp->item_2);
					return (OOP)ft_many_new(this->left, mid, right);
				} else {
					OOP right = object_call(this->right, msg);  // delegate to right
					return (OOP)ft_many_new(this->left, this->mid, right);
				}
			}
			if (mfp->item_1 == (OOP)&push_symbol) {
				return &undef_object; // [NOT IMPLEMENTED]
			}
		}
	}
	return &undef_object;
}

/*
queue:
	An efficient mutable (stateful) queue data-structure built from pairs.
*/

/*
	item := o.take()		-- remove and return item from the head of the queue
	boolean := o.empty?()	-- return true if queue is empty, otherwise false
	o := o.give(item)		-- add item to the tail of the queue
*/
KIND(queue_kind)
{
	if (queue_kind == self->kind) {
		struct pair * this = (struct pair *)self;
		if (finger_1_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&empty_p_symbol) {
				if (this->h == &nil_object) {
					return (OOP)&_t_symbol;
				}
				return (OOP)&_f_symbol;
			}
			if (mfp->item_1 == (OOP)&take_symbol) {
				if (this->h != &nil_object) {
					struct pair * entry = (struct pair *)(this->h);
					this->h = entry->t;
					return entry->h;
				}
			}
		}
		if (finger_2_kind == msg->kind) {
			struct finger * mfp = (struct finger *)msg;
			if (mfp->item_1 == (OOP)&give_symbol) {
				struct pair * entry = pair_new(mfp->item_2, &nil_object);
				if (this->h == &nil_object) {
					this->h = (OOP)entry;
				} else {
					struct pair * tail = (struct pair *)(this->t);
					tail->t = (OOP)entry;
				}
				this->t = (OOP)entry;
				return self;
			}
		}
	}
	return &undef_object;
}

OOP
queue_new() {
	struct pair * this = (struct pair *)object_new(queue_kind, sizeof(struct pair));
	this->h = &nil_object;
	this->t = &nil_object;
	return (OOP)this;
}

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
	TRACE(fprintf(stderr, "&eq_p_symbol = %p\n", &eq_p_symbol));
	TRACE(fprintf(stderr, "symbol_kind = %p\n", (void*)symbol_kind));
	TRACE(fprintf(stderr, "eq_p_symbol.o.kind = %p\n", (void*)eq_p_symbol.o.kind));
	TRACE(fprintf(stderr, "eq_p_symbol.s = \"%s\"\n", eq_p_symbol.s));

	TRACE(fprintf(stderr, "finger_1_kind = %p\n", (void*)finger_1_kind));
	TRACE(fprintf(stderr, "finger_2_kind = %p\n", (void*)finger_2_kind));
	TRACE(fprintf(stderr, "finger_3_kind = %p\n", (void*)finger_3_kind));
	TRACE(fprintf(stderr, "finger_4_kind = %p\n", (void*)finger_4_kind));

//	struct finger * fp = finger_2_new((OOP)&eq_p_symbol, (OOP)&_f_symbol);
//	TRACE(fprintf(stderr, "fp = %p\n", fp));
//	TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
//		fp->item_1, fp->item_2, fp->item_3, fp->item_4));
//	OOP result = object_call((OOP)&_f_symbol, (OOP)fp);
	OOP result = object_call_2((OOP)&_f_symbol, (OOP)&eq_p_symbol, (OOP)&_f_symbol);
//	OOP result = object_call((OOP)&_f_symbol, (OOP)finger_2_new((OOP)&eq_p_symbol, (OOP)&_f_symbol));
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert((OOP)&_t_symbol == result);
	
	struct symbol * x_symbol_ptr = symbol_new("x");
	TRACE(fprintf(stderr, "x_symbol_ptr = %p\n", x_symbol_ptr));
	TRACE(fprintf(stderr, "x_symbol_ptr->s = \"%s\"\n", x_symbol_ptr->s));
	result = object_call_2(&empty_dict, (OOP)&lookup_symbol, (OOP)x_symbol_ptr);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(&undef_object == result);
	
	struct symbol * _42_symbol_ptr = symbol_new("42");
	TRACE(fprintf(stderr, "_42_symbol_ptr = %p\n", _42_symbol_ptr));
	TRACE(fprintf(stderr, "_42_symbol_ptr->s = \"%s\"\n", _42_symbol_ptr->s));
	OOP env_ptr = object_call_3(&empty_dict, (OOP)&bind_symbol, (OOP)x_symbol_ptr, (OOP)_42_symbol_ptr);
	TRACE(fprintf(stderr, "env_ptr = %p\n", env_ptr));
	result = object_call_2(env_ptr, (OOP)&lookup_symbol, (OOP)x_symbol_ptr);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert((OOP)_42_symbol_ptr == result);

	struct symbol * y_symbol_ptr = symbol_new("y");
	TRACE(fprintf(stderr, "y_symbol_ptr = %p\n", y_symbol_ptr));
	TRACE(fprintf(stderr, "y_symbol_ptr->s = \"%s\"\n", y_symbol_ptr->s));
	result = object_call_2(env_ptr, (OOP)&lookup_symbol, (OOP)y_symbol_ptr);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(&undef_object == result);

/*
	struct integer * a_integer_ptr = integer_new('A');
	struct integer * z_integer_ptr = integer_new('Z');
	OOP ft_deque_ptr = (OOP)&empty_ft;
	TRACE(fprintf(stderr, "&empty_ft = %p\n", &empty_ft));
	struct integer * n_integer_ptr = a_integer_ptr;
	struct finger * eq_fp = finger_2_new((OOP)&eq_p_symbol, (OOP)z_integer_ptr);
	struct finger * inc_fp = finger_2_new((OOP)&add_symbol, (OOP)&_1_integer);
	while (object_call((OOP)n_integer_ptr, (OOP)eq_fp) != (OOP)&_t_symbol) {
		struct finger * fp = finger_2_new((OOP)&put_symbol, (OOP)n_integer_ptr);
		TRACE(fprintf(stderr, "fp = %p\n", fp));
		TRACE(fprintf(stderr, "finger: %p [%p %p %p %p]\n", fp->o.kind, 
			fp->item_1, fp->item_2, fp->item_3, fp->item_4));
		TRACE(fprintf(stderr, "ft_deque_ptr = %p ^ [%c]\n", ft_deque_ptr, n_integer_ptr->n));
		ft_deque_ptr = object_call(ft_deque_ptr, (OOP)fp);
		n_integer_ptr = (struct integer *)object_call((OOP)n_integer_ptr, (OOP)inc_fp);
	}
	n_integer_ptr = a_integer_ptr;
	struct finger * dec_fp = finger_2_new((OOP)&add_symbol, (OOP)&minus_1_integer);
	struct finger * pop_fp = finger_1_new((OOP)&pop_symbol);
	while (object_call((OOP)n_integer_ptr, (OOP)eq_fp) != (OOP)&_t_symbol) {
		struct pair * pp = (struct pair *)object_call(ft_deque_ptr, (OOP)pop_fp);
		struct integer * ip = (struct integer *)pp->h;
		ft_deque_ptr = pp->t;
		TRACE(fprintf(stderr, "ft_deque_ptr = [%c] ^ %p\n", ip->n, ft_deque_ptr));
		fp = finger_2_new((OOP)&eq_p_symbol, (OOP)n_integer_ptr);
		result = object_call((OOP)ip, (OOP)fp);
		assert((OOP)&_t_symbol == result);
		n_integer_ptr = (struct integer *)object_call((OOP)n_integer_ptr, (OOP)dec_fp);
	}
*/	

	OOP a_integer_ptr = (OOP)integer_new('A');
	OOP z_integer_ptr = (OOP)integer_new('Z');
	OOP queue_ptr = (OOP)queue_new();
	TRACE(fprintf(stderr, "queue_ptr = %p\n", queue_ptr));
	result = object_call_1(queue_ptr, (OOP)&empty_p_symbol);
	assert((OOP)&_t_symbol == result);
	OOP n_integer_ptr = a_integer_ptr;
	while (object_call_2(n_integer_ptr, (OOP)&eq_p_symbol, z_integer_ptr) != (OOP)&_t_symbol) {
		queue_ptr = object_call_2(queue_ptr, (OOP)&give_symbol, n_integer_ptr);
		TRACE(fprintf(stderr, "queue_ptr = %p ^ [%c]\n", queue_ptr, ((struct integer *)n_integer_ptr)->n));
		n_integer_ptr = object_call_2(n_integer_ptr, (OOP)&add_symbol, (OOP)&_1_integer);
	}
	result = object_call_1(queue_ptr, (OOP)&empty_p_symbol);
	assert((OOP)&_f_symbol == result);
	n_integer_ptr = a_integer_ptr;
	while (object_call_2(n_integer_ptr, (OOP)&eq_p_symbol, z_integer_ptr) != (OOP)&_t_symbol) {
		OOP i_integer_ptr = object_call_1(queue_ptr, (OOP)&take_symbol);
		TRACE(fprintf(stderr, "queue_ptr = [%c] ^ %p\n", ((struct integer *)i_integer_ptr)->n, queue_ptr));
		result = object_call_2(i_integer_ptr, (OOP)&eq_p_symbol, n_integer_ptr);
		assert((OOP)&_t_symbol == result);
		n_integer_ptr = object_call_2(n_integer_ptr, (OOP)&add_symbol, (OOP)&_1_integer);
	}
	result = object_call_1(queue_ptr, (OOP)&empty_p_symbol);
	assert((OOP)&_t_symbol == result);

	return 0;
}
