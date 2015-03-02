/**
 * Image is a virtual base class that prepares to draw pixels
 * onto a vewport. Images are expected to be very large, so
 * the image itself has no way to just draw an image. It's
 * mostly just a handle that knows things about the image so
 * can work with it later.
 *
 * Typically we're going to run with the GDAL convention that
 * the image origin is in the top left corner of the image.
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

#ifndef _HPP_IMAGE
#define _HPP_IMAGE

namespace fr {

  namespace geobrowse {

    class viewport;

    class image {
      
    public:

      long width;
      long height;

      image() : width(0), height(0)
      {}

      virtual ~image()
      {}

      virtual void load(viewport &v) = 0;

    };

  }

}

#endif
