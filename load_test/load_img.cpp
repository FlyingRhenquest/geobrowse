// Quick'n dirty test to make sure raster_image::load works

/*
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

#include <QtGui>
#include <QtGui/QImage>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include "viewport.hpp"
#include "raster_image.hpp"
#include <iostream>

#include <ogr_spatialref.h>

// Just give it an image to open as the first argument

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QGraphicsScene scene;
  QGraphicsView view(&scene);
  fr::geobrowse::gui_info xinfo;
  fr::geobrowse::raster_image img = fr::geobrowse::raster_image(std::string(argv[1]));
  std::cout << "Image: " << argv[1] << std::endl;
  std::cout << "Bands: " << img.bands.size() << std::endl;
  std::cout << "Width: " << img.width << std::endl;
  std::cout << "Height: " << img.height << std::endl;
  std::cout << "Red band: " << img.red_band << std::endl;
  std::cout << "Green band: " << img.green_band << std::endl;
  std::cout << "Blue band: " << img.blue_band << std::endl;
  std::string coordinate_system(GDALGetProjectionRef(img.dataset.get()));
  if (coordinate_system.length() > 0) {
    std::cout << "Image coordinate system: " << coordinate_system << std::endl;
  }
  std::cout << std::endl << "Origin X: " << img.transform[0] << std::endl;
  std::cout << "Origin Y: " << img.transform[3] << std::endl;
  std::cout << "Pixel size west-east: " << img.transform[1] << std::endl;
  std::cout << "Pixel-size north-south: " << img.transform[5] << std::endl;
  
  if (coordinate_system.length() > 0) {
    OGRSpatialReference source_srs(coordinate_system.c_str());
    OGRSpatialReference target_srs;
    target_srs.SetWellKnownGeogCS("WGS84");
    OGRCoordinateTransformation *transform;
    transform = OGRCreateCoordinateTransformation(&source_srs, &target_srs);
    double x,y;
    x = img.transform[0];
    y = img.transform[3];
    if (nullptr == transform || ! transform->Transform(1, &x, &y)) {
      std::cout << "Reproject to WGS84 failed" << std::endl;
    } else {
      std::cout << "Origin coordinates (x,y): " << x << " , " << y << std::endl;
    }
    delete transform;
  }

  fr::geobrowse::viewport v(0,0,img.width, img.height, xinfo, img.bands.size());
  img.load(v);
  QImage qi(v.data_buffer, img.width, img.height, QImage::Format_ARGB32);
  QPixmap pm(QPixmap::fromImage(qi));
  scene.addPixmap(pm);
  view.show();
  return app.exec();
}
