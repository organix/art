/*

json.h -- Actor Run-Time

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
#ifndef _JSON_H_
#define _JSON_H_

#include "art.h"
#include "object.h"
#include "pair.h"

/*
 * scope
 */

#define    o_empty_scope    (o_empty_dict)

struct scope {
    struct object   o;
    OOP             dict;
    OOP             next;
};
#define as_scope(oop)   ((struct scope *)(oop))
extern OOP scope_new(OOP next);
extern KIND(scope_kind);

/*
 * pattern
 */

struct named_pattern {
    struct object   o;
    OOP             name;        // symbol to lookup
    OOP             scope;        // scope to search
};
#define as_named_pattern(oop)   ((struct named_pattern *)(oop))
extern OOP named_pattern_new(OOP name, OOP scope);
extern KIND(named_pattern_kind);

/*
 * stream
 */

#define    o_empty_stream    (o_nil)

struct string_stream {
	struct object	o;
	char *			s;
};
#define	as_string_stream(oop)	((struct string_stream *)(oop))
extern OOP		string_stream_new(char * s);
extern KIND(string_stream_kind);

/*
 * grammar
 */
 
extern OOP json_grammar_new();

#endif /* _JSON_H_ */
