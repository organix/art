/*

actor.h -- Actor Run-Time

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
#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "art.h"
#include "object.h"

/*
 * actor
 */

struct actor {
    struct object   o;
    OOP             beh;
};
#define as_actor(oop) ((struct actor *)(oop))
extern OOP actor_new(OOP beh);
extern KIND(actor_kind);

/*
 * event
 */

extern struct symbol create_x_symbol;
#define s_create_x ((OOP)&create_x_symbol)
extern struct symbol send_x_symbol;
#define s_send_x ((OOP)&send_x_symbol)
extern struct symbol become_x_symbol;
#define s_become_x ((OOP)&become_x_symbol)
extern struct symbol dispatch_x_symbol;
#define s_dispatch_x ((OOP)&dispatch_x_symbol)

struct event {
    struct object   o;
    OOP             actor;      // target actor
    OOP             msg;        // message to deliver
    OOP             actors;     // actors created
    OOP             events;     // messages sent
    OOP             beh;        // replacement behavior
};
#define as_event(oop) ((struct event *)(oop))
extern OOP event_new(OOP actor, OOP msg);
extern KIND(event_kind);

/*
 * behavior
 */

//extern KIND(empty_beh_kind);
extern struct object empty_beh;
#define beh_empty ((OOP)&empty_beh)
extern struct actor sink_actor;
#define a_sink ((OOP)&sink_actor)

struct forward_beh {
    struct object   o;
    OOP             target;     // messages are forwarded to 'target' actor
};
#define as_forward_beh(oop) ((struct forward_beh *)(oop))
extern OOP forward_beh_new(OOP target);
extern KIND(forward_beh_kind);
extern OOP one_shot_beh_new(OOP target);
extern KIND(one_shot_beh_kind);

/*
 * configuration
 */

struct config {
    struct object   o;
    OOP             events;     // queue of message-events to be delivered
    OOP             remain;     // number of queued message-events
};
#define as_config(oop) ((struct config *)(oop))
extern OOP config_new();
extern KIND(config_kind);

#endif /* _ACTOR_H_ */
