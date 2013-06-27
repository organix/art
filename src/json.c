/*

json.c -- Actor Run-Time

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
#include "json.h"
#include "pattern.h"
#include "pair.h"
#include "object.h"

/*
json       = (_ value)+ _
value      = object | array | string | number | name
object     = '{' _ (property _ (',' _ property _)*)? '}'
property   = string _ ':' _ value
array      = '[' _ (value _ (',' _ value _)*)? ']'
string     = '"' character* '"'
character  = ('\\' escape) | [^"\\]
escape     = 'u' [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F]
           | [\\/tnrbf"]
number     = integer fraction? exponent?
integer    = '-'? ('0' | [1-9] [0-9]*)
fraction   = '.' [0-9]+
exponent   = [eE] [-+]? [0-9]+
name       = 'n' 'u' 'l' 'l'
           | 't' 'r' 'u' 'e'
           | 'f' 'a' 'l' 's' 'e'
_          = [ \t\n\r\b\f]*
*/

static struct symbol json_symbol = { { symbol_kind }, "json" };
#define s_json ((OOP)&json_symbol)
static struct symbol value_symbol = { { symbol_kind }, "value" };
#define s_value ((OOP)&value_symbol)
static struct symbol object_symbol = { { symbol_kind }, "object" };
#define s_object ((OOP)&object_symbol)
static struct symbol array_symbol = { { symbol_kind }, "array" };
#define s_array ((OOP)&array_symbol)
static struct symbol string_symbol = { { symbol_kind }, "string" };
#define s_string ((OOP)&string_symbol)
static struct symbol number_symbol = { { symbol_kind }, "number" };
#define s_number ((OOP)&number_symbol)
static struct symbol name_symbol = { { symbol_kind }, "name" };
#define s_name ((OOP)&name_symbol)
static struct symbol ws_symbol = { { symbol_kind }, "ws" };
#define s_ws ((OOP)&ws_symbol)

/*
scope:
    Scopes are mutable mappings from names to values.
    
    In this implementation, each node holds a dictionary of 'name'/'value' pairs.
    The 'next' pointer delegates to a linear chain of scopes.

    x := o.lookup(name)         -- return value 'x' bound to 'name', or 'o_fail'
    o.bind(name, x)             -- bind 'name' to 'x' in this scope
*/

OOP
scope_new(OOP next)
{
    struct scope * this = object_alloc(struct scope, scope_kind);
    this->dict = o_empty_dict;
    this->next = next;
    return (OOP)this;
}
KIND(scope_kind)
{
    struct scope * this = as_scope(self);
    TRACE(fprintf(stderr, "%p(scope_kind, %p, %p)\n", this, this->dict, this->next));
    OOP cmd = take_arg();
    TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
    if (cmd == s_eq_p) {
        OOP other = take_arg();
        if (other == self) {  // compare identities
            return o_true;
        }
        return o_false;
    } else if (cmd == s_lookup) {
        OOP name = take_arg();
        TRACE(fprintf(stderr, "  %p: name=%p \"%s\"\n", self, name, as_symbol(name)->s));
        OOP result = object_call(this->dict, s_lookup, name);
        if (result == o_fail) {
            return object_call(this->next, s_lookup, name);  // delegate call
        }
        return result;
    } else if (cmd == s_bind) {
        OOP name = take_arg();
        OOP value = take_arg();
        TRACE(fprintf(stderr, "  %p: name=%p \"%s\" value=%p\n", self, name, as_symbol(name)->s, value));
        this->dict = object_call(this->dict, s_bind, name, value);
        return self;
    }
    return o_undef;
}

/*
 * named_pattern
 */

OOP
named_pattern_new(OOP name, OOP scope)
{
    struct named_pattern * this = object_alloc(struct named_pattern, named_pattern_kind);
    this->name = name;
    this->scope = scope;
    return (OOP)this;
}
KIND(named_pattern_kind)
{
    struct named_pattern * this = as_named_pattern(self);
    TRACE(fprintf(stderr, "%p(named_pattern_kind, %p, %p)\n", this, this->name, this->scope));
    OOP cmd = take_arg();
    TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
    if (cmd == s_eq_p) {
        OOP other = take_arg();
        if (other == self) {  // compare identities
            return o_true;
        }
        return o_false;
    } else if (cmd == s_match) {
        OOP match = take_arg();
        OOP ptrn = object_call(this->scope, s_lookup, this->name);
        if (o_fail != ptrn) {
            return object_call(ptrn, s_match, match);
        }
        return o_fail;
    }
    return o_undef;
}

/*
 * stream
 */

OOP
string_stream_new(char * s)
{
    if ((s == NULL) || (*s == '\0')) {
        return o_empty_stream;
    }
    struct string_stream * this = object_alloc(struct string_stream, string_stream_kind);
    this->s = s;
    return (OOP)this;
}
KIND(string_stream_kind)
{
    struct string_stream * this = as_string_stream(self);
    TRACE(fprintf(stderr, "%p(string_stream_kind, %p)\n", this, this->s));
    OOP cmd = take_arg();
    TRACE(fprintf(stderr, "  %p: cmd=%p \"%s\"\n", self, cmd, as_symbol(cmd)->s));
    if (cmd == s_eq_p) {
        OOP other = take_arg();
        if (other == self) {  // compare identities
            return o_true;
        }
        return o_false;
    } else if (cmd == s_empty_p) {
        return o_false;
    } else if (cmd == s_pop) {
        char * s = this->s;
        OOP n_ch = integer_new(*s);
        OOP next = string_stream_new(++s);
        int ch = as_integer(n_ch)->n;
        TRACE(fprintf(stderr, "  %p: ch@%p #%d '%c'\n", self, n_ch, ch, ch));
        return pair_new(n_ch, next);
    }
    return o_undef;
}


/*
name       = 'n' 'u' 'l' 'l'
           | 't' 'r' 'u' 'e'
           | 'f' 'a' 'l' 's' 'e'
*/
static OOP
name_grammar_new(OOP scope)
{
    OOP g_null = and_pattern_new(
        eq_pattern_new(integer_new('n')),
        and_pattern_new(
            eq_pattern_new(integer_new('u')),
            and_pattern_new(
                eq_pattern_new(integer_new('l')),
                eq_pattern_new(integer_new('l')))));
    OOP g_true = and_pattern_new(
        eq_pattern_new(integer_new('t')),
        and_pattern_new(
            eq_pattern_new(integer_new('r')),
            and_pattern_new(
                eq_pattern_new(integer_new('u')),
                eq_pattern_new(integer_new('e')))));
    OOP g_false = and_pattern_new(
        eq_pattern_new(integer_new('f')),
        and_pattern_new(
            eq_pattern_new(integer_new('a')),
            and_pattern_new(
                eq_pattern_new(integer_new('l')),
                and_pattern_new(
                    eq_pattern_new(integer_new('s')),
                    eq_pattern_new(integer_new('e'))))));
    OOP g_name = or_pattern_new(g_null, or_pattern_new(g_true, g_false));
    object_call(scope, s_bind, s_name, g_name);
    return g_name;
}

/*
number     = integer fraction? exponent?
integer    = '-'? ('0' | [1-9] [0-9]*)
fraction   = '.' [0-9]+
exponent   = [eE] [-+]? [0-9]+
*/
static OOP
number_grammar_new(OOP scope)
{
    OOP g_digit = if_pattern_new(charset_p_new("0123456789"));
    OOP g_integer = and_pattern_new(
        opt_pattern_new(
            eq_pattern_new(integer_new('-'))),
        or_pattern_new(
            eq_pattern_new(integer_new('0')),
            and_pattern_new(
                if_pattern_new(charset_p_new("123456789")),
                star_pattern_new(g_digit))));
    OOP g_fraction = and_pattern_new(
        eq_pattern_new(integer_new('.')),
        plus_pattern_new(g_digit));
    OOP g_exponent = and_pattern_new(
        or_pattern_new(
            eq_pattern_new(integer_new('e')),
            eq_pattern_new(integer_new('E'))),
        and_pattern_new(
            opt_pattern_new(
                or_pattern_new(
                    eq_pattern_new(integer_new('+')),
                    eq_pattern_new(integer_new('-')))),
            plus_pattern_new(g_digit)));
    OOP g_number = and_pattern_new(
        g_integer, 
        and_pattern_new(
            opt_pattern_new(g_fraction), 
            opt_pattern_new(g_exponent)));
    object_call(scope, s_bind, s_number, g_number);
    return g_number;
}

/*
string     = '"' character* '"'
character  = ('\\' escape) | [^"\\]
escape     = 'u' [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F] [0-9a-fA-F]
           | [\\/tnrbf"]
*/
static OOP
string_grammar_new(OOP scope)
{
    OOP g_hexdigit = if_pattern_new(charset_p_new("0123456789ABCDEFabcdef"));
    OOP g_escape = or_pattern_new(
        and_pattern_new(
            eq_pattern_new(integer_new('u')),
            and_pattern_new(
                g_hexdigit,
                and_pattern_new(
                    g_hexdigit,
                    and_pattern_new(
                        g_hexdigit,
                        g_hexdigit)))), 
        if_pattern_new(charset_p_new("\\/tnrbf\"")));
    OOP g_character = or_pattern_new(
        and_pattern_new(
            eq_pattern_new(integer_new('\\')),
            g_escape),
        if_pattern_new(exclset_p_new("\"\\")));
    OOP g_string = and_pattern_new(
        eq_pattern_new(integer_new('"')), 
        and_pattern_new(
            star_pattern_new(g_character), 
            eq_pattern_new(integer_new('"'))));
    object_call(scope, s_bind, s_string, g_string);
    return g_string;
}

/*
array      = '[' _ (value _ (',' _ value _)*)? ']'
*/
static OOP
array_grammar_new(OOP scope)
{
    OOP g_ws = object_call(scope, s_lookup, s_ws);  // already bound
    OOP g_value = named_pattern_new(s_value, scope);  // defered lookup
    OOP g_array = and_pattern_new(
        eq_pattern_new(integer_new('[')),
        and_pattern_new(
            g_ws,
            and_pattern_new(
                opt_pattern_new(
                    and_pattern_new(
                        g_value,
                        and_pattern_new(
                            g_ws,
                            star_pattern_new(
                                and_pattern_new(
                                    eq_pattern_new(integer_new(',')),
                                    and_pattern_new(
                                        g_ws,
                                        and_pattern_new(
                                            g_value, 
                                            g_ws))))))),
                eq_pattern_new(integer_new(']')))));
    object_call(scope, s_bind, s_array, g_array);
    return g_array;
}

/*
object     = '{' _ (property _ (',' _ property _)*)? '}'
property   = string _ ':' _ value
*/
static OOP
object_grammar_new(OOP scope)
{
    OOP g_ws = object_call(scope, s_lookup, s_ws);  // already bound
    OOP g_property = and_pattern_new(
        named_pattern_new(s_string, scope),
        and_pattern_new(
            g_ws,
            and_pattern_new(
                eq_pattern_new(integer_new(':')),
                and_pattern_new(
                    g_ws,
                    named_pattern_new(s_value, scope)))));
    OOP g_object = and_pattern_new(
        eq_pattern_new(integer_new('{')),
        and_pattern_new(
            g_ws,
            and_pattern_new(
                opt_pattern_new(
                    and_pattern_new(
                        g_property,
                        and_pattern_new(
                            g_ws,
                            star_pattern_new(
                                and_pattern_new(
                                    eq_pattern_new(integer_new(',')),
                                    and_pattern_new(
                                        g_ws,
                                        and_pattern_new(
                                            g_property, 
                                            g_ws))))))),
                eq_pattern_new(integer_new('}')))));
    object_call(scope, s_bind, s_object, g_object);
    return g_object;
}

/*
value      = object | array | string | number | name
*/
static OOP
value_grammar_new(OOP scope)
{
    OOP g_object = object_grammar_new(scope);
    OOP g_array = array_grammar_new(scope);
    OOP g_string = string_grammar_new(scope);
    OOP g_number = number_grammar_new(scope);
    OOP g_name = name_grammar_new(scope);
    OOP g_value = or_pattern_new(
        g_object,
        or_pattern_new(
            g_array,
            or_pattern_new(
                g_string,
                or_pattern_new(
                    g_number,
                    g_name))));
    object_call(scope, s_bind, s_value, g_value);
    return g_value;
}

/*
json       = (_ value)+ _
_          = [ \t\n\r\b\f]*
*/
OOP
json_grammar_new()
{
    OOP scope = scope_new(o_empty_scope);
    OOP g_ws = star_pattern_new(
        if_pattern_new(charset_p_new(" \t\n\r\b\f")));
    object_call(scope, s_bind, s_ws, g_ws);
    OOP g_value = value_grammar_new(scope);
    OOP g_json = and_pattern_new(
        plus_pattern_new(
            and_pattern_new(
                g_ws,
                g_value)),
            g_ws);
    object_call(scope, s_bind, s_json, g_json);
    return g_json;
}
