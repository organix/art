/*

actor.c -- Actor Run-Time

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
#include "actor.h"
#include "pair.h"

/*
actor:
	Actors encapsulate state/behavior reacting to asynchronous events (messages).
	
	NOTE: Actor objects are not intended to be called directly, but if they are
		they simply delegate to their behavior 'beh'
*/

OOP
actor_new(OOP beh)
{
	struct actor * this = object_alloc(struct actor, actor_kind);
	this->beh = beh;
	return (OOP)this;
}

KIND(actor_kind)
{
	struct actor * this = as_actor(self);
	TRACE(fprintf(stderr, "%p(actor_kind, %p)\n", this, this->beh));
	OOP event = take_arg();
	TRACE(fprintf(stderr, "  %p: event=%p\n", this, event));
	if (event == s_eq_p) {  // support default object protocol
		OOP other = take_arg();
		if (other == self) {  // compare identities
			return o_true;
		}
		return o_false;
	}
	return object_call(this->beh, event);
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

struct symbol create_x_symbol = { { symbol_kind }, "create!" };
struct symbol send_x_symbol = { { symbol_kind }, "send!" };
struct symbol become_x_symbol = { { symbol_kind }, "become!" };
struct symbol dispatch_x_symbol = { { symbol_kind }, "dispatch!" };

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
	struct event * this = as_event(self);
	TRACE(fprintf(stderr, "%p event_kind {actor:%p, msg:%p}\n", this, this->actor, this->msg));
	OOP cmd = take_arg();
	TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", this, cmd, as_symbol(cmd)->s));
	if (cmd == s_eq_p) {
		OOP other = take_arg();
		if (other == self) {  // compare identities
			return o_true;
		}
		return o_false;
	} else if (cmd == s_dispatch_x) {
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
	return o_undef;
}

/*
behavior:
	Behaviors cause "effects" for an actor, using a sponsor

	o.(event)	-- cause effects triggered by 'event'
*/

static KIND(empty_beh_kind)
{
	TRACE(fprintf(stderr, "%p(empty_beh_kind)\n", self));
	OOP evt = take_arg();
	OOP act = as_event(evt)->actor;
	OOP msg = as_event(evt)->msg;
	TRACE(fprintf(stderr, "  %p: event=%p {actor:%p, msg:%p}\n", self, evt, act, msg));
	return o_true;  // commit
}

struct object empty_beh = { empty_beh_kind };
struct actor sink_actor = { { actor_kind }, beh_empty };

OOP
forward_beh_new(OOP target)
{
	struct forward_beh * this = object_alloc(struct forward_beh, forward_beh_kind);
	this->target = target;
	return (OOP)this;
}

KIND(forward_beh_kind)
{
	struct forward_beh * this = as_forward_beh(self);
	TRACE(fprintf(stderr, "%p forward_beh_kind {target:%p}\n", this, this->target));
	OOP evt = take_arg();
	OOP act = as_event(evt)->actor;
	OOP msg = as_event(evt)->msg;
	TRACE(fprintf(stderr, "  %p: event=%p {actor:%p, msg:%p}\n", self, evt, act, msg));
	object_call(evt, s_send_x, this->target, msg);
	return o_true;  // commit
}

OOP
one_shot_beh_new(OOP target)
{
	struct forward_beh * this = object_alloc(struct forward_beh, one_shot_beh_kind);
	this->target = target;
	return (OOP)this;
}

KIND(one_shot_beh_kind)
{
	struct forward_beh * this = as_forward_beh(self);
	TRACE(fprintf(stderr, "%p one_shot_beh_kind {target:%p}\n", this, this->target));
	OOP evt = take_arg();
	OOP act = as_event(evt)->actor;
	OOP msg = as_event(evt)->msg;
	TRACE(fprintf(stderr, "  %p: event=%p {actor:%p, msg:%p}\n", self, evt, act, msg));
	object_call(evt, s_send_x, this->target, msg);
	object_call(evt, s_become_x, beh_empty);  // BECOME sink_beh
	return o_true;  // commit
}

/*
config:
	Configurations are collections of actors and in-flight message-events.
	
	remain := o.dispatch!(count)-- dispatch up to 'count' events, return how many 'remain'
	remain := o.give!(event)	-- add 'event' to the queue of in-flight events
*/

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
	struct config * this = as_config(self);
	TRACE(fprintf(stderr, "%p config_kind {remain:%d}\n", this, as_integer(this->remain)->n));
	OOP cmd = take_arg();
	TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", this, cmd, as_symbol(cmd)->s));
	if (cmd == s_eq_p) {
		OOP other = take_arg();
		if (other == self) {  // compare identities
			return o_true;
		}
		return o_false;
	} else if (cmd == s_give_x) {
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
			if (o_true == result) {
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
			} else {
				TRACE(fprintf(stderr, "  %p: event=%p ---ABORTED---", this, evt));
			}
			// decrement count
			count = object_call(count, s_add, n_minus_1);
			TRACE(fprintf(stderr, "  %p: count=%d\n", this, as_integer(count)->n));
		}
		return this->remain;
	}
	return o_undef;
}
