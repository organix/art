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
#define	undef_oop	(&undef_object)
struct object nil_object = { object_kind };
#define	nil_oop	(&nil_object)

KIND(object_kind)
{
	return undef_oop;
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
struct symbol create_symbol = { { symbol_kind }, "create" };
#define	create_oop	((OOP)&create_symbol)
struct symbol send_symbol = { { symbol_kind }, "send" };
#define	send_oop	((OOP)&send_symbol)
struct symbol become_symbol = { { symbol_kind }, "become" };
#define	become_oop	((OOP)&become_symbol)
struct symbol dispatch_symbol = { { symbol_kind }, "dispatch" };
#define	dispatch_oop	((OOP)&dispatch_symbol)

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
		/* no object protocol for pair */
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
		OOP cmd = take_arg();
		if (cmd == empty_p_oop) {
			if (this->h == nil_oop) {
				return _t_oop;
			}
			return _f_oop;
		} else if (cmd == take_x_oop) {
			if (this->h != nil_oop) {
				struct pair * entry = as_pair(this->h);
				this->h = entry->t;
				OOP item = entry->h;  // entry is garbage after this (use custom free?)
				return item;
			}
		} else if (cmd == give_x_oop) {
			OOP item = take_arg();
			OOP oop = pair_new(item, nil_oop);  // could be a custom allocator
			if (this->h == nil_oop) {
				this->h = oop;
			} else {
				as_pair(this->t)->t = oop;
			}
			this->t = oop;
			return self;
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
KIND(empty_dict_kind);
struct object empty_dict = { empty_dict_kind };
#define	empty_dict_oop	(&empty_dict)

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
		if (cmd == lookup_oop) {
			OOP name = take_arg();
			TRACE(fprintf(stderr, "  %p: name=%p\n", self, name));
			return undef_oop;
		} else if (cmd == bind_oop) {
			OOP name = take_arg();
			OOP value = take_arg();
			TRACE(fprintf(stderr, "  %p: name=%p value=%p\n", self, name, value));
			return dict_new(name, value, self);
		}
	}
	return undef_oop;
}

KIND(dict_kind)
{
	if (dict_kind == self->kind) {
//		struct dict * this = as_dict(self);  -- moved inside "do" loop...
		TRACE(fprintf(stderr, "%p(dict_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == lookup_oop) {
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
			return object_call(self, lookup_oop, name);  // delegate call
		} else if (cmd == bind_oop) {
			OOP name = take_arg();
			OOP value = take_arg();
			TRACE(fprintf(stderr, "  %p: name=%p value=%p\n", self, name, value));
			return dict_new(name, value, self);
		}
	}
	return undef_oop;
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
		if (cmd == eq_p_oop) {
			OOP other = take_arg();
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
		} else if (cmd == add_oop) {
			OOP other = take_arg();
			if (integer_kind == other->kind) {
				struct integer * that = as_integer(other);
				return integer_new(this->n + that->n);
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

#ifdef _ENABLE_FINGER_TREE_
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
		OOP a = this->item_1;
		OOP cmd = take_arg();
		if (cmd == put_oop) {
			OOP x = take_arg();
			return finger_2_new(a, x);
		} else if (cmd == push_oop) {
			OOP x = take_arg();
			return finger_2_new(x, a);
		}
	}
	return undef_oop;
}

KIND(finger_2_kind)
{
	if (finger_2_kind == self->kind) {
		struct finger * this = as_finger(self);
		OOP a = this->item_1;
		OOP b = this->item_2;
		OOP cmd = take_arg();
		if (cmd == pop_oop) {
			return pair_new(a, finger_1_new(b));
		} else if (cmd == pull_oop) {
			return pair_new(b, finger_1_new(a));
		} else if (cmd == put_oop) {
			OOP x = take_arg();
			return finger_3_new(a, b, x);
		} else if (cmd == push_oop) {
			OOP x = take_arg();
			return finger_3_new(x, a, b);
		}
	}
	return undef_oop;
}

KIND(finger_3_kind)
{
	if (finger_3_kind == self->kind) {
		struct finger * this = as_finger(self);
		OOP a = this->item_1;
		OOP b = this->item_2;
		OOP c = this->item_3;
		OOP cmd = take_arg();
		if (cmd == pop_oop) {
			return pair_new(a, finger_2_new(b, c));
		} else if (cmd == pull_oop) {
			return pair_new(c, finger_2_new(a, b));
		} else if (cmd == put_oop) {
			OOP x = take_arg();
			return finger_4_new(a, b, c, x);
		} else if (cmd == push_oop) {
			OOP x = take_arg();
			return finger_4_new(x, a, b, c);
		}
	}
	return undef_oop;
}

KIND(finger_4_kind)
{
	if (finger_4_kind == self->kind) {
		struct finger * this = as_finger(self);
		OOP a = this->item_1;
		OOP b = this->item_2;
		OOP c = this->item_3;
		OOP d = this->item_4;
		OOP cmd = take_arg();
		if (cmd == pop_oop) {
			return pair_new(a, finger_3_new(b, c, d));
		} else if (cmd == pull_oop) {
			return pair_new(d, finger_3_new(a, b, c));
		}
	}
	return undef_oop;
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
	OOP args = take_arg();
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
	OOP args = take_arg();
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
							struct pair * pp = (struct pair *)object_call_1(this->right, args);  // delegate to right
							OOP left = (OOP)finger_1_new(pp->h);
							return pair_new(
									fp->item_1,
									(OOP)ft_many_new(left, this->mid, pp->t));
						}
					} else {
						struct pair * pp = (struct pair *)object_call_1(this->mid, args);  // delegate to mid
						return pair_new(
								fp->item_1,
								(OOP)ft_many_new(pp->h, pp->t, this->right));
					}
				} else {
					struct pair * pp = (struct pair *)object_call_1(this->left, args);  // delegate to left
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
#endif /* _ENABLE_FINGER_TREE_ */

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
	return _f_oop;  // failure?
}

/*
event:
	Events are unique occurances of a particular 'msg' for a specific 'actor'.
	When the message is delivered to the actor, the effects are held in the event.
	The event acts as the "sponsor" for the computation, providing resources to the actor.
	
	actor := o.create(beh)	-- return a new actor with initial behavior 'beh'
	o.send(actor, message)	-- send 'message' to 'actor' asynchronously
	o.become(beh)			-- use behavior 'beh' to process subsequent messages
	o.dispatch()			-- deliver 'msg' to 'actor'
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
		if (cmd == dispatch_oop) {
			OOP beh = as_actor(this->actor)->beh;
			TRACE(fprintf(stderr, "  %p: dispatch {beh:%p}\n", this, beh));
			this->actors = nil_oop;
			this->events = nil_oop;
			this->beh = beh;
			return object_call(beh, this);  // invoke actor behavior
		} else if (cmd == create_oop) {
			OOP beh = take_arg();
			TRACE(fprintf(stderr, "  %p: create {beh:%p}\n", this, beh));
			OOP actor = actor_new(beh);
			this->actors = pair_new(actor, this->actors);  // add actor to stack
			return actor;
		} else if (cmd == send_oop) {
			OOP actor = take_arg();
			OOP msg = take_arg();
			TRACE(fprintf(stderr, "  %p: send {actor:%p, msg:%p}\n", this, actor, msg));
			OOP event = event_new(actor, msg);
			this->events = pair_new(actor, this->events);  // add event to stack
			return event;
		} else if (cmd == become_oop) {
			OOP beh = take_arg();
			TRACE(fprintf(stderr, "  %p: become {beh:%p}\n", this, beh));
			this->beh = beh;
			return beh;
		}
	}
	return _f_oop;  // failure?
}

/*
behavior:
	Behaviors cause "effects" for an actor, using a sponsor

	o.(event)	-- cause effects triggered by 'event'
*/

KIND(empty_beh_kind)
{
	TRACE(fprintf(stderr, "%p(empty_beh_kind)\n", self));
	OOP event = take_arg();
	OOP actor = as_event(event)->actor;
	OOP msg = as_event(event)->msg;
	TRACE(fprintf(stderr, "  %p: event=%p {actor:%p, msg:%p}\n", self, event, actor, msg));
	return _t_oop;  // success?
}

struct object empty_beh = { empty_beh_kind };
#define	empty_beh_oop	(&empty_beh)

struct actor sink_actor = { { actor_kind }, empty_beh_oop };
#define	sink_actor_oop	((OOP)&sink_actor)

/*
config:
	Configurations are collections of actors and in-flight message-events.
	
	remain := o.dispatch(count)	-- dispatch up to 'count' events, return how many 'remain'
	remain := o.give!(event)	-- add 'event' to the queue of in-flight events

	actor := o.create(beh)	-- return a new actor with initial behavior 'beh'
	o.send(actor, message)	-- send 'message' to 'actor' asynchronously
	o.become(beh)			-- use behavior 'beh' to process subsequent messages
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
	this->remain = _0_oop;
	return (OOP)this;
}

KIND(config_kind)
{
	if (config_kind == self->kind) {
		struct config * this = as_config(self);
		TRACE(fprintf(stderr, "%p config_kind {remain:%d}\n", this, as_integer(this->remain)->n));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", this, cmd, as_symbol(cmd)->s));
		if (cmd == give_x_oop) {
			// enqueue an event
			OOP event_oop = take_arg();
			TRACE(fprintf(stderr, "  %p: give! {event_oop:%p}\n", this, event_oop));
			object_call(this->events, give_x_oop, event_oop);
			this->remain = object_call(this->remain, add_oop, _1_oop);
			TRACE(fprintf(stderr, "  %p: remain=%d\n", this, as_integer(this->remain)->n));
			return this->remain;
		} else if (cmd == dispatch_oop) {
			// dispatch up to 'count' events
			OOP count = take_arg();
			TRACE(fprintf(stderr, "  %p: dispatch {count:%d}\n", this, as_integer(count)->n));
			while ((object_call(count, eq_p_oop, _0_oop) != _t_oop)
			&&     (object_call(this->events, empty_p_oop) == _f_oop)) {
				// dequeue next event
				OOP event_oop = object_call(this->events, take_x_oop);
				TRACE(fprintf(stderr, "  %p: event_oop=%p\n", this, event_oop));
				// dispatch event
				OOP result = object_call(event_oop, dispatch_oop);
				TRACE(fprintf(stderr, "  %p: result=%p\n", this, result));
				// apply result
				assert(_t_oop == result);
				struct event * ep = as_event(event_oop);
				OOP events = ep->events;
				while (nil_oop != events) {  // enqueue events
					object_call(this->events, give_x_oop, as_pair(events)->h);
					events = as_pair(events)->t;
				}
				as_actor(ep->actor)->beh = ep->beh;  // replace behavior
				TRACE(fprintf(stderr, "  %p: beh=%p\n", this, ep->beh));
				// update counters
				this->remain = object_call(this->remain, add_oop, minus_1_oop);
				TRACE(fprintf(stderr, "  %p: remain=%d\n", this, as_integer(this->remain)->n));
				count = object_call(count, add_oop, minus_1_oop);
				TRACE(fprintf(stderr, "  %p: count=%d\n", this, as_integer(count)->n));
			}
			return this->remain;
		}
	}
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

#ifdef _ENABLE_FINGER_TREE_
	TRACE(fprintf(stderr, "finger_1_kind = %p\n", (void*)finger_1_kind));
	TRACE(fprintf(stderr, "finger_2_kind = %p\n", (void*)finger_2_kind));
	TRACE(fprintf(stderr, "finger_3_kind = %p\n", (void*)finger_3_kind));
	TRACE(fprintf(stderr, "finger_4_kind = %p\n", (void*)finger_4_kind));
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
#endif /* _ENABLE_FINGER_TREE_ */

	TRACE(fprintf(stderr, "dict_kind = %p\n", (void*)dict_kind));
	TRACE(fprintf(stderr, "lookup_oop = %p\n", lookup_oop));
	TRACE(fprintf(stderr, "bind_oop = %p\n", bind_oop));

	OOP x_oop = symbol_new("x");
	TRACE(fprintf(stderr, "x_oop = %p\n", x_oop));
	TRACE(fprintf(stderr, "as_symbol(x_oop)->s = \"%s\"\n", as_symbol(x_oop)->s));
	OOP result = object_call(empty_dict_oop, lookup_oop, x_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(undef_oop == result);
	
	OOP _42_oop = integer_new(42);
	TRACE(fprintf(stderr, "_42_oop = %p\n", _42_oop));
	TRACE(fprintf(stderr, "as_integer(_42_oop)->n = %d\n", as_integer(_42_oop)->n));
	OOP env_oop = object_call(empty_dict_oop, bind_oop, x_oop, _42_oop);
	TRACE(fprintf(stderr, "env_oop = %p\n", env_oop));
	result = object_call(env_oop, lookup_oop, x_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(_42_oop == result);

	OOP y_oop = symbol_new("y");
	TRACE(fprintf(stderr, "y_oop = %p\n", y_oop));
	TRACE(fprintf(stderr, "as_symbol(y_oop)->s = \"%s\"\n", as_symbol(y_oop)->s));
	env_oop = object_call(env_oop, bind_oop, y_oop, minus_1_oop);
	TRACE(fprintf(stderr, "env_oop = %p\n", env_oop));

	OOP z_oop = symbol_new("z");
	TRACE(fprintf(stderr, "z_oop = %p\n", z_oop));
	TRACE(fprintf(stderr, "as_symbol(z_oop)->s = \"%s\"\n", as_symbol(z_oop)->s));
	result = object_call(env_oop, lookup_oop, z_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(undef_oop == result);

	OOP a_integer_oop = integer_new('A');
	OOP z_integer_oop = integer_new('Z');
	OOP queue_oop = queue_new();
	TRACE(fprintf(stderr, "queue_oop = %p\n", queue_oop));
	result = object_call(queue_oop, empty_p_oop);
	assert(_t_oop == result);
	OOP n_integer_oop = a_integer_oop;
	while (object_call(n_integer_oop, eq_p_oop, z_integer_oop) != _t_oop) {
		queue_oop = object_call(queue_oop, give_x_oop, n_integer_oop);
		TRACE(fprintf(stderr, "queue_oop = %p ^ [%c]\n", queue_oop, as_integer(n_integer_oop)->n));
		n_integer_oop = object_call(n_integer_oop, add_oop, _1_oop);
	}
	result = object_call(queue_oop, empty_p_oop);
	assert(_f_oop == result);
	n_integer_oop = a_integer_oop;
	while (object_call(n_integer_oop, eq_p_oop, z_integer_oop) != _t_oop) {
		OOP i_integer_oop = object_call(queue_oop, take_x_oop);
		TRACE(fprintf(stderr, "queue_oop = [%c] ^ %p\n", as_integer(i_integer_oop)->n, queue_oop));
		result = object_call(i_integer_oop, eq_p_oop, n_integer_oop);
		assert(_t_oop == result);
		n_integer_oop = object_call(n_integer_oop, add_oop, _1_oop);
	}
	result = object_call(queue_oop, empty_p_oop);
	assert(_t_oop == result);

	TRACE(fprintf(stderr, "create_oop = %p\n", create_oop));
	TRACE(fprintf(stderr, "send_oop = %p\n", send_oop));
	TRACE(fprintf(stderr, "become_oop = %p\n", become_oop));
	TRACE(fprintf(stderr, "dispatch_oop = %p\n", dispatch_oop));

	OOP config_oop = config_new();
	result = object_call(config_oop, dispatch_oop, _0_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(_t_oop == object_call(result, eq_p_oop, _0_oop));
	OOP event_oop = event_new(sink_actor_oop, _42_oop);
	TRACE(fprintf(stderr, "event_oop = %p\n", event_oop));
	result = object_call(config_oop, give_x_oop, event_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(_t_oop == object_call(result, eq_p_oop, _1_oop));
	result = object_call(config_oop, dispatch_oop, _1_oop);
	TRACE(fprintf(stderr, "result = %p\n", result));
	assert(_t_oop == object_call(result, eq_p_oop, _0_oop));
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
