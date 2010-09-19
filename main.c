/*
 * Copyright (c) 2010 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#define MAX_PROPERTY_VALUE_LEN 4096

Display *display;
Window window;
int release_key = XK_Alt_L;

void grab(int grab)
{
    if(grab)
    {
        XGrabPointer(display, window, True, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
        XGrabKey(display, XKeysymToKeycode(display, release_key), AnyModifier, window, False, GrabModeAsync, GrabModeAsync);
    } else {
        XUngrabPointer(display, CurrentTime);
        XUngrabKey(display, XKeysymToKeycode(display, release_key), AnyModifier, window);
    }
}

int main(int argc, char **argv)
{
    Window root;
    XEvent event;

    int i;
    Window *client_list;
    unsigned long size = 0;
    char *window_name;

    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    char desktop_window[256];

    snprintf(desktop_window, 256, "%s - Wine desktop", argc > 1 ? argv[1] : "Default");
    if(argc > 2)
    {
        release_key = atoi(argv[2]);
    }

    display = XOpenDisplay(0);
    if(display == NULL) {
        fprintf(stderr, "Error: Could not open display!\n");
        return 1;
    }

    root = XDefaultRootWindow(display);

    XGetWindowProperty(display, root, XInternAtom(display, "_NET_CLIENT_LIST", False), 0, MAX_PROPERTY_VALUE_LEN / 4, False,
            XA_WINDOW, &xa_ret_type, &ret_format, &ret_nitems, &ret_bytes_after, (unsigned char **)&client_list);

    size = (ret_format / 8) * ret_nitems;

    if(xa_ret_type != XA_WINDOW) {
        fprintf(stderr, "Error: XGetWindowPropery did not return XA_WINDOW\n");
        return 1;
    }

    window = 0;
    for(i=0;i < size / sizeof(Window);i++) {
        XFetchName(display, client_list[i], &window_name);
        if(window_name && strcmp(window_name, desktop_window) == 0) {
            window = client_list[i];
            XFree(window_name);
            break;
        }
        XFree(window_name);
    }

    XFree(client_list);

    if(!window) {
        fprintf(stderr, "Error: \"%s\" was not found\n", desktop_window);
        return 1;
    }

    XSelectInput(display, window, FocusChangeMask);

    do {
        XNextEvent(display, &event);

        if(event.type == FocusIn)
        {
            grab(1);
        }
        else if(event.type == FocusOut)
        {
            grab(0);
        }
        else if(event.type == MotionNotify)
        {
            XSendEvent(display, window, True, PointerMotionMask, &event);
        }
        else if(event.type == ButtonPress)
        {
            XSendEvent(display, window, True, ButtonPressMask, &event);
        }
        else if(event.type == ButtonRelease)
        {
            XSendEvent(display, window, True, ButtonReleaseMask, &event);
        }
        else if(event.type == KeyPress)
        {
            grab(0);
        }
    } while(1);

    return 0;
}
