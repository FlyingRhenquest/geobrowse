/**
 * An image with raster pixels in. Gets loaded from a file. Uses GDAL
 * to load the pixels, and can support any format GDAL can. Image
 * can be georeferenced or not.
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

#ifndef _HPP_RASTER_IMAGE
#define _HPP_RASTER_IMAGE

#include "image.hpp"
#include "gdal_priv.h"
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

namespace fr {

  namespace geobrowse {

    class raster_image : public image {

      // I'm using a shared pointer here, which will delete the
      // owned object when the last copy of that pointer goes
      // out of scope. GDAL says don't use delete on the dataset
      // because it could not work correctly on windows. But the
      // X11 stuff is kind of Linux-Specific anyway, so I'm not
      // terribly worried about that right now.

      typedef std::shared_ptr<GDALDataset> dataset_pointer;

      // GDALRasterBands are owned by dataset, so you don't have to
      // worry about freeing them
      typedef std::vector<GDALRasterBand *> band_vector;

      std::string filename;

    public:
      dataset_pointer dataset;

    private:

      void populate_bands()
      {
	// GDAL Bands start at 1
	for (int i = 0; i < dataset->GetRasterCount(); ++i) {
	  bands.push_back(dataset->GetRasterBand(i + 1));
	  GDALColorInterp color = bands[i]->GetColorInterpretation();
	  if (bands.size() > 1) {
	    switch(color) {
	    case GCI_Undefined:
	      // Guess at color
	      switch(i) {
	      case 0: 
		red_band = i;
		break;
	      case 1:
		green_band = i;
		break;
	      case 2:
		blue_band = i;
		break;
	      case 3:
		alpha_band = i;
		break;
	      }
	    case GCI_RedBand:
	      red_band = i;
	      break;
	    case GCI_GreenBand:
	      green_band = i;
	      break;
	    case GCI_BlueBand:
	      blue_band = i;
	      break;
	    case GCI_AlphaBand:
	      alpha_band = i;
	      break;
	    } // switch(color)	    
	  } else {
	    // (probably) Monochrome
	    red_band = green_band = blue_band = 0;
	  }
	}
      }

      // I can query GDAL for a lookup table, too
      // but I'm not doing that right now

      /**
       * load_pixel loads the colors for red, green, blue and alpha into
       * the viewport's data buffer. Although I'm technically expecting
       * an 8 bit value here, nothing actually prevents a larger one.
       * If you have 10 bit colors or something, this should still work
       * with it as long as the color scales computed from visual
       * indicate that.
       */

      inline void load_pixel(viewport &v, int32_t red, int32_t green, int32_t blue, int32_t alpha = 0xff)
      {	
	(*v.current_pixel++) = ((red & v.xinfo.red_scale) << v.xinfo.red_offset) |
	  ((green & v.xinfo.green_scale) << v.xinfo.green_offset) |
	  ((blue & v.xinfo.blue_scale) << v.xinfo.blue_offset) |
	  ((alpha & v.xinfo.alpha_scale) << v.xinfo.alpha_offset);
      }

    /**
     * Load one line of greyscale (red = green = blue no alpha)
     */

      inline void load_gs(viewport &v)
      {
	double *buf = v.band_buffers[red_band]; // All the band indices are the same in this case
	for (int i = 0; i < v.width; ++i) {
	  load_pixel(v, (int32_t) buf[i], (int32_t) buf[i], (int32_t) buf[i]);
	}
      }

    /**
     * Load one line of 3 channel no alpha
     */

    inline void load_rgb(viewport &v)
    {
      double *r = v.band_buffers[red_band];
      double *g = v.band_buffers[green_band];
      double *b = v.band_buffers[blue_band];
      for (int i = 0; i < v.width; ++i) {
	load_pixel(v, (int32_t) r[i], (int32_t) g[i], (int32_t) b[i]);
      }
    }

    /**
     * Load rgba
     */

    inline void load_rgba(viewport &v)
    {
      double *r = v.band_buffers[red_band];
      double *g = v.band_buffers[green_band];
      double *b = v.band_buffers[blue_band];
      double *a = v.band_buffers[alpha_band];
      for (int i = 0; i < v.width; ++i) {
	load_pixel(v, (int32_t) r[i], (int32_t) g[i], (int32_t) b[i], (int32_t) a[i]);
      }
    }

    public:

      // Indices to color bands
      int red_band;
      int green_band;
      int blue_band;
      int alpha_band;

      // Georeferencing Transform. See http://www.gdal.org/gdal_tutorial.html
      double transform[6];
      band_vector bands;

      raster_image(const std::string &filename) : image(), filename(filename)
      {
	// Register all GDAL drivers
	GDALAllRegister();
	dataset = dataset_pointer((GDALDataset *)GDALOpen(filename.c_str(), GA_ReadOnly));
	if (nullptr == dataset.get()) {
	  std::string err("Error opening ");
	  err.append(filename);
	  throw std::logic_error(err);
	}
	// Getting transform will set a transform even if the
	// image isn't georferenced.
	dataset->GetGeoTransform(transform);
	this->height = dataset->GetRasterYSize();
	this->width = dataset->GetRasterXSize();
	populate_bands();
      }

      void load(viewport &v)
      {
	for (int current_row = v.y; current_row < v.y + v.height; ++current_row) {
	  for(int i = 0; i < bands.size() ; ++i) {
	    // Load line into viewport buffers
	    GDALRasterIO(bands[i], GF_Read, v.x, v.y + current_row, v.width, 1, v.band_buffers[i], v.width, 1, GDT_Float64, 0,0);
	  }
	  switch(bands.size()) {
	    // Load lines into viewport data_buffer
	  case 1: 
	    load_gs(v);
	    break;
	  case 3:
	    load_rgb(v);
	    break;
	  case 4:
	    load_rgba(v);
	    break;
	  }
	}
      }

    };

  }

}

#endif

