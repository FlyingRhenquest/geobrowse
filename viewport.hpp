/**
 * A window into an image. The viewport can be created at
 * any location inside an image, and calling image::load
 * against the viewport will load the pixels from that
 * location into the viewport.
 *
 * The viewport contains the pixels as an unsigned char
 * array in 32 bit XImage format. I can easily use this
 * data to create an XImage or a QImage. I do populate
 * the alpha channel, if the image has one.
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

#ifndef _HPP_VIEWPORT
#define _HPP_VIEWPORT

#include "gui_info.hpp"
#include <vector>

namespace fr {

  namespace geobrowse {

    class viewport {
      
    public:
      int x, y, width, height;
      unsigned char *data_buffer;
      int32_t *current_pixel; // Used by image::load
      gui_info &xinfo;
      int nbands;

      // Vector of band buffers. GDAL seems to want to read them in as floating
      // point. You can specify type in GDALRasterIO, but it got
      // irritable with me when I tried to specify something other than
      // floating point. Buffer is for 1 line of raster data
      std::vector<double *> band_buffers;

      viewport(int x, int y, int width, int height, gui_info &xinfo, int nbands) : x(x), y(y), width(width), height(height), xinfo(xinfo), data_buffer(new unsigned char[width * height * sizeof(int32_t)]), current_pixel((int32_t *) data_buffer), nbands(nbands)
      {
	for (int i = 0 ; i < nbands; ++i) {
	  band_buffers.push_back(new double[width]);
	}
      }

      ~viewport()
      {
	delete data_buffer;
	for (int i = 0 ; i < nbands; ++i) {
	  delete band_buffers[i];
	}
      }

    };

  }

}


#endif
