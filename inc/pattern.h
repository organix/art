/*

pattern.h -- Actor Run-Time

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
#ifndef _PATTERN_H_
#define _PATTERN_H_

#include "art.h"
#include "object.h"

struct match {
    struct object   o;
    OOP             in;         // position in input value
    OOP             env;        // dictionary of identifier bindings
    OOP             out;        // semantic value (output)
};
#define as_match(oop) ((struct match *)(oop))
extern OOP match_new(OOP in, OOP env, OOP out);
extern KIND(match_kind);

/*
 * pattern
 */

extern struct symbol match_symbol;
#define s_match ((OOP)&match_symbol)

//extern KIND(fail_pattern_kind);
extern struct object fail_pattern;
#define ptrn_fail ((OOP)&fail_pattern)

//extern KIND(empty_pattern_kind);
extern struct object empty_pattern;
#define ptrn_empty ((OOP)&empty_pattern)

//extern KIND(all_pattern_kind);
extern struct object all_pattern;
#define ptrn_all ((OOP)&all_pattern)

//extern KIND(end_pattern_kind);
extern struct object end_pattern;
#define ptrn_end ((OOP)&end_pattern)

//extern KIND(any_pattern_kind);
extern struct object any_pattern;
#define ptrn_any ((OOP)&any_pattern)

struct eq_pattern {
    struct object   o;
    OOP             value;      // value to match
};
#define as_eq_pattern(oop) ((struct eq_pattern *)(oop))
extern OOP eq_pattern_new(OOP value);
extern KIND(eq_pattern_kind);

struct if_pattern {
    struct object   o;
    OOP             test;       // predicate to test tokens
};
#define as_if_pattern(oop) ((struct if_pattern *)(oop))
extern OOP if_pattern_new(OOP test);
extern KIND(if_pattern_kind);

struct charset_p {
    struct object   o;
    char *          s;
};
#define as_charset_p(oop) ((struct charset_p *)(oop))
extern OOP charset_p_new(char * set);
extern KIND(charset_p_kind);
extern OOP exclset_p_new(char * excl);
extern KIND(exclset_p_kind);

struct or_pattern {
    struct object   o;
    OOP             head;       // first pattern
    OOP             tail;       // second pattern
};
#define as_or_pattern(oop) ((struct or_pattern *)(oop))
extern OOP or_pattern_new(OOP head, OOP tail);
extern KIND(or_pattern_kind);

struct and_pattern {
    struct object   o;
    OOP             head;       // first pattern
    OOP             tail;       // second pattern
};
#define as_and_pattern(oop) ((struct and_pattern *)(oop))
extern OOP and_pattern_new(OOP head, OOP tail);
extern KIND(and_pattern_kind);

struct bind_pattern {
    struct object   o;
    OOP             name;       // symbol to bind
    OOP             ptrn;       // pattern to match
};
#define as_bind_pattern(oop) ((struct bind_pattern *)(oop))
extern OOP bind_pattern_new(OOP name, OOP ptrn);
extern KIND(bind_pattern_kind);

struct ref_pattern {
    struct object   o;
    OOP             ptrn;       // pattern reference
};
#define as_ref_pattern(oop) ((struct ref_pattern *)(oop))
extern OOP opt_pattern_new(OOP ptrn);  // 0 or 1
extern OOP star_pattern_new(OOP ptrn);  // 0 or more
extern KIND(star_pattern_kind);
extern OOP plus_pattern_new(OOP ptrn);  // 1 or more

/*
 * expression
 */

extern struct symbol eval_symbol;
#define s_eval ((OOP)&eval_symbol)
extern struct symbol combine_symbol;
#define s_combine ((OOP)&combine_symbol)

// "bottom" represents the inability to determine a result when evaluating an expression
extern struct object bottom_object;
#define o_bottom ((OOP)&bottom_object)

extern KIND(const_expr_kind);

struct ident_expr {
    struct object   o;
    OOP             name;       // identifier to lookup
};
#define as_ident_expr(oop) ((struct ident_expr *)(oop))
extern OOP ident_expr_new(OOP name);
extern KIND(ident_expr_kind);

struct combine_expr {
    struct object   o;
    OOP             oper;       // operator
    OOP             opnd;       // operand
};
#define as_combine_expr(oop) ((struct combine_expr *)(oop))
extern OOP combine_expr_new(OOP oper, OOP opnd);
extern KIND(combine_expr_kind);

struct appl_expr {
    struct object   o;
    OOP             comb;       // combiner
};
#define as_appl_expr(oop) ((struct appl_expr *)(oop))
extern OOP appl_expr_new(OOP comb);
extern KIND(appl_expr_kind);

struct thunk_expr {
    struct object   o;
    OOP             env;        // static environment
    OOP             ptrn;       // formal parameter pattern
    OOP             expr;       // body expression
};
#define as_thunk_expr(oop) ((struct thunk_expr *)(oop))
extern OOP thunk_expr_new(OOP env, OOP ptrn, OOP expr);
extern KIND(thunk_expr_kind);

struct lambda_expr {
    struct object   o;
    OOP             ptrn;       // formal parameter pattern
    OOP             expr;       // body expression
};
#define as_lambda_expr(oop) ((struct lambda_expr *)(oop))
extern OOP lambda_expr_new(OOP ptrn, OOP expr);
extern KIND(lambda_expr_kind);

struct oper_expr {
    struct object   o;
    OOP             env;        // static environment
    OOP             ptrn;       // formal parameter pattern
    OOP             evar;       // environment variable name
    OOP             expr;       // body expression
};
#define as_oper_expr(oop) ((struct oper_expr *)(oop))
extern OOP oper_expr_new(OOP env, OOP ptrn, OOP evar, OOP expr);
extern KIND(oper_expr_kind);

struct quote_expr {  // FIXME: this is deprecated, but temporarily used for testing
    struct object   o;
    OOP             value;      // literal value
};
#define as_quote_expr(oop) ((struct quote_expr *)(oop))
extern OOP quote_expr_new(OOP value);
extern KIND(quote_expr_kind);

#endif /* _PATTERN_H_ */
