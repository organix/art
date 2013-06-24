/*

art.c -- Actor Run-Time

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

#include <stdio.h>
#include <assert.h>
//#include <time.h>
#include "art.h"
#include "object.h"
#include "symbol.h"
#include "pair.h"
#include "pattern.h"
#include "actor.h"

#undef	_ENABLE_FINGER_TREE_	/**/

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

	TRACE(fprintf(stderr, "---- expression evaluation ----\n"));
	TRACE(fprintf(stderr, "s_eval = %p\n", s_eval));
	TRACE(fprintf(stderr, "s_combine = %p\n", s_combine));
	// (\x.x)(42) -> 42
//struct object const_42_expr = { const_expr_kind };
//#define	expr_const_42	(&const_42_expr)
	OOP expr_const_42 = object_alloc(struct object, const_expr_kind);
	TRACE(fprintf(stderr, "expr_const_42 = %p\n", expr_const_42));
	OOP expr_ident_x = ident_expr_new(s_x);
	TRACE(fprintf(stderr, "expr_ident_x = %p\n", expr_ident_x));
	OOP expr_example = combine_expr_new(
		lambda_expr_new(bind_pattern_new(s_x, ptrn_all), expr_ident_x),
		expr_const_42);
	result = object_call(expr_example, s_eval, o_empty_dict);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(expr_const_42 == result);
	// (\[x, y].y)([0, 1]) -> 1
	// (($lambda (x y) y) ($quote (0 1))) -> 1
	// (($lambda (x y) y) (list 0 1))) -> 1
	TRACE(fprintf(stderr, "n_0 = %p\n", n_0));
	TRACE(fprintf(stderr, "n_1 = %p\n", n_1));
	OOP ptrn_parm_x_y = and_pattern_new(
		bind_pattern_new(s_x, ptrn_any),
		and_pattern_new(
			bind_pattern_new(s_y, ptrn_any),
			ptrn_end));
	OOP list_0_1 = pair_new(n_0, pair_new(n_1, o_nil));
	expr_example = combine_expr_new(
		lambda_expr_new(
			ptrn_parm_x_y, 
			ident_expr_new(s_y)),
		quote_expr_new(list_0_1));
	result = object_call(expr_example, s_eval, o_empty_dict);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(n_1 == result);

	TRACE(fprintf(stderr, "---- actor primitives ----\n"));
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
