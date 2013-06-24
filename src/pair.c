/*

pair.c -- Actor Run-Time

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

#include <stdio.h>  /* for TRACE */
#include "pair.h"

/*
pair:
	A combination of 2 object pointers.  May be treated as a list, terminated by 'o_nil'.

	o' := o.push(x)			-- return a new list with 'x' at the head
	boolean := o.empty?()	-- return true if list is empty, otherwise false
	(x, o') := o.pop()		-- remove 'x' from the head, returning it and the new list
*/

struct symbol empty_p_symbol = { { symbol_kind }, "empty?" };
struct symbol push_symbol = { { symbol_kind }, "push" };
struct symbol pop_symbol = { { symbol_kind }, "pop" };
struct symbol put_symbol = { { symbol_kind }, "put" };
struct symbol pull_symbol = { { symbol_kind }, "pull" };

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

/*
queue:
	An efficient mutable (stateful) queue data-structure built from pairs.

	item := o.take!()		-- remove and return 'item' from the head of the queue
	boolean := o.empty?()	-- return true if queue is empty, otherwise false
	o := o.give!(item)		-- add 'item' to the tail of the queue
*/

struct symbol give_x_symbol = { { symbol_kind }, "give!" };
struct symbol take_x_symbol = { { symbol_kind }, "take!" };

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
queue_new()
{
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

struct symbol bind_symbol = { { symbol_kind }, "bind" };
struct symbol lookup_symbol = { { symbol_kind }, "lookup" };

struct object fail_object = { object_kind };

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

struct object empty_dict = { empty_dict_kind };

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

struct symbol eq_p_symbol = { { symbol_kind }, "eq?" };
struct symbol add_symbol = { { symbol_kind }, "add" };

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
struct integer _0_integer = { { integer_kind }, 0 };
struct integer _1_integer = { { integer_kind }, 1 };
struct integer _2_integer = { { integer_kind }, 2 };
