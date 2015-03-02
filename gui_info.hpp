/**
 * Info about the GUI. I'm currently using X11, and want to avoid
 * exposing this to the rest of my program as much as possible.
 *
 * Copyright 2015 Bruce Ide
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _HPP_GUI_INFO
#define _HPP_GUI_INFO

#include <stdexcept>
#include <X11/Xlib.h>

namespace fr {

  namespace geobrowse {

    /**
     * My tools assume I'm working in X11. gui_info
     * is just a useful place to stash X11-related crap
     * that X11 functions need, like the default visual,
     * display, etc.
     */

    class gui_info {

      // I assume the masks are contiguous, which is generally
      // safe for this application
      //
      // Note alpha mask will shift 1s over from the left,
      // which is not what you want. But X11 doesn't understand
      // the concept of an alpha mask anyway, so I set it manually

      int compute_offset(int32_t mask, int32_t &scale)
      {	
	int retval = 1;
	if (mask & 1) {
	  retval = 0;
	} else {
	  while ((mask >>= 1) && !(mask & 1)) {
	    retval++;
	  }
	}
	scale = mask; // Assign shifted mask
	return retval; // Return count of right-trailing zero-bits
      }

    public:

      Visual *visual;
      Display *display;
      Screen *screen; // Default screen of display

      /**
       * The X11 visual contains masks for each of the primary
       * colors. These are bitmasks that tell you where in the
       * pixel that color falls. To set a color correctly, 
       * it needs to be masked to the mask so no pixels
       * fall outside the mask, and shifted to the correct
       * location in the pixel (The data buffer is in bytes,
       * but the actual pixel is in int32_t, for great justice!)
       */

      int red_offset;
      int green_offset;
      int blue_offset;
      int alpha_offset;

      // Actual Mask (Copied in from visual)
      int32_t red_mask;
      int32_t green_mask;
      int32_t blue_mask;
      int32_t alpha_mask;

      // Value of the mask once the 1s are shifted
      // to the far right (Generally will be 255)
      int32_t red_scale;
      int32_t green_scale;
      int32_t blue_scale;
      int32_t alpha_scale;
      
      gui_info()
      {
	// Open an X11 display. I don't close this when the object is destroyed,
	// so you can copy this object around pretty safely. At some point you
	// might want to close it yourself. I'm not sure it'll be completely cleaned
	// up otherwise, since you're maintaining a session to another process
	// for the graphical display.
	//
	// XOpenDisplay uses the DISPLAY environment variable if one isn't
	// specified
	display = XOpenDisplay(NULL);
	if (!display) {
	  throw std::logic_error("Error opening display");
	}
	screen = DefaultScreenOfDisplay(display);
	visual = DefaultVisual(display, DefaultScreen(display));
       
	red_mask = visual->red_mask;
	green_mask = visual->green_mask;
	blue_mask = visual->blue_mask;
	alpha_mask = 0xff000000; // probably
	red_offset = compute_offset(red_mask, red_scale);
	green_offset = compute_offset(green_mask, green_scale);
	blue_offset = compute_offset(blue_mask, blue_scale);
	alpha_offset = 24; // probably
	alpha_scale = 255; // 8 bits worth of ones
      }

    };

  }

}

#endif
