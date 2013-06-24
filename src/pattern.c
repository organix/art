/*

pattern.c -- Actor Run-Time

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
#include "pattern.h"
#include "pair.h"

/*
match:
	Matches represent composable contexts for pattern matching.

	(in, env, out) -> (in', env', out') | #fail
*/

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

	match_out := o.match(match_in)	-- return the result of matching pattern to 'match_in', or 'o_fail'
*/

struct symbol match_symbol = { { symbol_kind }, "match" };

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

/* LET all = \in.(#ok, in, in) */
KIND(all_pattern_kind)
{
	if (all_pattern_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(all_pattern_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			return match_new(mp->in, mp->env, mp->in);
		}
	}
	return o_undef;
}
struct object all_pattern = { all_pattern_kind };

/* LET end = \in.(
	CASE in OF
	() : (#ok, (), in)
	_ : (#fail, in)
	END
) */
KIND(end_pattern_kind)
{
	if (end_pattern_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(end_pattern_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_match) {
			OOP match = take_arg();
			struct match * mp = as_match(match);
			TRACE(fprintf(stderr, "  %p: match {in:%p env:%p out:%p}\n", self, mp->in, mp->env, mp->out));
			if (object_call(mp->in, s_empty_p) == o_true) {
				return match_new(mp->in, mp->env, o_nil);
			}
			return o_fail;
		}
	}
	return o_undef;
}
struct object end_pattern = { end_pattern_kind };

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

OOP
eq_pattern_new(OOP value)
{
	struct eq_pattern * this = object_alloc(struct eq_pattern, eq_pattern_kind);
	this->value = value;
	return (OOP)this;
}
/* LET eq(value) = \in.(
	CASE in OF
	() : (#fail, in)
	($value, rest) : (#ok, value, rest)
	_ : (#fail, in)
	END
) */
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
if_pattern_new(OOP test)
{
	struct if_pattern * this = object_alloc(struct if_pattern, if_pattern_kind);
	this->test = test;
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
or_pattern_new(OOP head, OOP tail)
{
	struct or_pattern * this = object_alloc(struct or_pattern, or_pattern_kind);
	this->head = head;
	this->tail = tail;
	return (OOP)this;
}
/* LET or(left, right) = \in.(
	CASE left(in) OF
	(#ok, value, in') : (#ok, value, in')
	(#fail, in') : right(in)
	END
) */
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
and_pattern_new(OOP head, OOP tail)
{
	struct and_pattern * this = object_alloc(struct and_pattern, and_pattern_kind);
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
bind_pattern_new(OOP name, OOP ptrn)
{
	struct bind_pattern * this = object_alloc(struct bind_pattern, bind_pattern_kind);
	this->name = name;
	this->ptrn = ptrn;
	return (OOP)this;
}
/* LET bind(name, ptrn) = \(value, env, in).(
	CASE ptrn(value, env, in) OF
	(#ok, value', env', in') : (#ok, value', env'.bind(name, value'), in')
	(#fail, value', env', in') : (#fail, value, env, in)
	END
) */
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

struct symbol eval_symbol = { { symbol_kind }, "eval" };
struct symbol combine_symbol = { { symbol_kind }, "combine" };

// "bottom" represents the inability to determine a result when evaluating an expression
struct object bottom_object = { object_kind };

KIND(const_expr_kind)
{
	if (const_expr_kind == self->kind) {
		TRACE(fprintf(stderr, "%p(const_expr_kind)\n", self));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();  // constants evaluate to themselves
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			return self;
		}
	}
	return o_undef;
}

OOP
ident_expr_new(OOP name)
{
	struct ident_expr * this = object_alloc(struct ident_expr, ident_expr_kind);
	this->name = name;
	return (OOP)this;
}
KIND(ident_expr_kind)
{
	if (ident_expr_kind == self->kind) {
		struct ident_expr * this = as_ident_expr(self);
		TRACE(fprintf(stderr, "%p(ident_expr_kind, %p \"%s\")\n", this, this->name, as_symbol(this->name)->s));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			return object_call(env, s_lookup, this->name);
		}
	}
	return o_undef;
}

OOP
combine_expr_new(OOP oper, OOP opnd)
{
	struct combine_expr * this = object_alloc(struct combine_expr, combine_expr_kind);
	this->oper = oper;
	this->opnd = opnd;
	return (OOP)this;
}
KIND(combine_expr_kind)
{
	if (combine_expr_kind == self->kind) {
		struct combine_expr * this = as_combine_expr(self);
		TRACE(fprintf(stderr, "%p(combine_expr_kind, %p, %p)\n", this, this->oper, this->opnd));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			OOP comb = object_call(this->oper, s_eval, env);  // evaluate oper (in env) to yield combiner
			TRACE(fprintf(stderr, "  %p: combiner=%p\n", self, comb));
			return object_call(comb, s_combine, this->opnd, env);  // send operand to combiner (with env)
		}
	}
	return o_undef;
}

OOP
appl_expr_new(OOP comb)
{
	struct appl_expr * this = object_alloc(struct appl_expr, appl_expr_kind);
	this->comb = comb;
	return (OOP)this;
}
KIND(appl_expr_kind)
{
	if (appl_expr_kind == self->kind) {
		struct appl_expr * this = as_appl_expr(self);
		TRACE(fprintf(stderr, "%p(appl_expr_kind, %p)\n", this, this->comb));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			return self;  // applicatives evaluate to themselves
		} else if (cmd == s_combine) {
			OOP opnd = take_arg();
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: combine {opnd:%p env:%p}\n", self, opnd, env));
			OOP arg = object_call(opnd, s_eval, env);  // evaluate opnd (in env) to yield argument
			TRACE(fprintf(stderr, "  %p: arg=%p\n", self, arg));
			return object_call(this->comb, s_combine, arg, env);  // send arg to combiner (with env)
		}
	}
	return o_undef;
}

OOP
thunk_expr_new(OOP env, OOP ptrn, OOP expr)
{
	struct thunk_expr * this = object_alloc(struct thunk_expr, thunk_expr_kind);
	this->env = env;
	this->ptrn = ptrn;
	this->expr = expr;
	return (OOP)this;
}
KIND(thunk_expr_kind)
{
	if (thunk_expr_kind == self->kind) {
		struct thunk_expr * this = as_thunk_expr(self);
		TRACE(fprintf(stderr, "%p(thunk_expr_kind, %p, %p, %p)\n", this, this->env, this->ptrn, this->expr));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			return self;  // closures evaluate to themselves
		} else if (cmd == s_combine) {
			OOP opnd = take_arg();
			OOP env = take_arg();		// dynamic environment (ignored)
			TRACE(fprintf(stderr, "  %p: combine {opnd:%p env:%p}\n", self, opnd, env));
			OOP match = match_new(opnd, this->env, o_undef);
			match = object_call(this->ptrn, s_match, match);
			if (match_kind == match->kind) {
				struct match * mp = as_match(match);
				TRACE(fprintf(stderr, "  %p: env'=%p\n", self, mp->env));
				return object_call(this->expr, s_eval, mp->env);  // evaluate body in extended environment
			}
			return o_bottom;  // parameter pattern mismatch
		}
	}
	return o_undef;
}

OOP
lambda_expr_new(OOP ptrn, OOP expr)
{
	struct lambda_expr * this = object_alloc(struct lambda_expr, lambda_expr_kind);
	this->ptrn = ptrn;
	this->expr = expr;
	return (OOP)this;
}
KIND(lambda_expr_kind)
{
	if (lambda_expr_kind == self->kind) {
		struct lambda_expr * this = as_lambda_expr(self);
		TRACE(fprintf(stderr, "%p(lambda_expr_kind, %p, %p)\n", this, this->ptrn, this->expr));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			OOP oper = thunk_expr_new(env, this->ptrn, this->expr);
			TRACE(fprintf(stderr, "  %p: oper=%p\n", self, oper));
			return appl_expr_new(oper);
		}
	}
	return o_undef;
}

OOP
oper_expr_new(OOP env, OOP ptrn, OOP evar, OOP expr)
{
	struct oper_expr * this = object_alloc(struct oper_expr, oper_expr_kind);
	this->env = env;
	this->ptrn = ptrn;
	this->evar = evar;
	this->expr = expr;
	return (OOP)this;
}
KIND(oper_expr_kind)
{
	if (oper_expr_kind == self->kind) {
		struct oper_expr * this = as_oper_expr(self);
		TRACE(fprintf(stderr, "%p(oper_expr_kind, %p, %p, %p, %p)\n", this, this->env, this->ptrn, this->evar, this->expr));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			return self;  // operatives evaluate to themselves
		} else if (cmd == s_combine) {
			OOP opnd = take_arg();
			OOP env = take_arg();		// dynamic environment
			TRACE(fprintf(stderr, "  %p: combine {opnd:%p env:%p}\n", self, opnd, env));

			OOP match = match_new(opnd, this->env, o_undef);
			match = object_call(this->ptrn, s_match, match);
			if (match_kind == match->kind) {
				OOP env1 = as_match(match)->env;
				TRACE(fprintf(stderr, "  %p: env1=%p\n", self, env1));
				OOP env2 = object_call(env1, s_bind, this->evar, env);  // bind dynamic environment
				TRACE(fprintf(stderr, "  %p: env2=%p\n", self, env2));
				return object_call(this->expr, s_eval, env2);  // evaluate body in extended environment
			}
			return o_bottom;  // parameter pattern mismatch

		}
	}
	return o_undef;
}

OOP
quote_expr_new(OOP value)
{
	struct quote_expr * this = object_alloc(struct quote_expr, quote_expr_kind);
	this->value = value;
	return (OOP)this;
}
KIND(quote_expr_kind)
{
	if (quote_expr_kind == self->kind) {
		struct quote_expr * this = as_quote_expr(self);
		TRACE(fprintf(stderr, "%p(quote_expr_kind, %p)\n", this, this->value));
		OOP cmd = take_arg();
		TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
		if (cmd == s_eval) {
			OOP env = take_arg();
			TRACE(fprintf(stderr, "  %p: eval {env:%p}\n", self, env));
			return this->value;
		}
	}
	return o_undef;
}
