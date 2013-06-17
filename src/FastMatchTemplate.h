#ifndef FastMatchTemplate_h
#define FastMatchTemplate_h
/***************************************************************************
 *            FastMatchTemplate.h
 *
 *
 *  Copyright  2010  Tristen Georgiou
 *  tristen_georgiou@hotmail.com
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


    How fast match template works:
    1. Both target and source image are down sampled numDownPyrs times
    2. cvMatchTemplate() function is called on shrunken images
       (uses CCORR_NORMED algorithm)
    3. The numMaxima best locations are found
    4. For each point where a maxima was located:
       a) Original source image is searched at point +/- searchExpansion
          pixels in both x and y direction
    5. If match score is above matchPercentage then the location and score is
       saved in the foundPointsList and confidencesList, respectively
    6. If findMultipleTargets is true, an attempt will be made to find up to
       numMaxima targets
    7. (Optional) The targets can be drawn to a color version of the source
       image using the DrawFoundTargets() function
 */

#include <stdio.h>
#include <cv.h>

using namespace cv;
using namespace std;

/*=============================================================================
  FastMatchTemplate
  Performs a fast match template
  Returns: true on success, false on failure
  Parameters:
    source - source image (where we are searching)
    target - target image (what we are searching for)
    foundPointsList - contains a list of the points where the target was found
    confidencesList - contains a list of the confidence value (0-100) for each
                      found target
    matchPercentage - the minimum required match score to consider the target
                      found
    findMultipleTargets - if set to true, the function will attempt to find a
                          maximum of numMaxima targets
    numMaxima - the maximum number of search locations to try before exiting
                (i.e. when image is down-sampled and searched, we collect the
                best numMaxima locations - those with the highest confidence -
                and search the original image at these locations)
    numDownPyrs - the number of times to down-sample the image (only increase
                  this number if your images are really large)
    searchExpansion - The original source image is searched at the top locations
                      with +/- searchExpansion pixels in both the x and y
                      directions
*/
bool
FastMatchTemplate(const Mat&      source,
                  const Mat&      target,
                  vector<cv::Point>*  foundPointsList,
                  vector<double>* confidencesList,
                  int             matchPercentage = 70,
                  bool            findMultipleTargets = true,
                  int             numMaxima = 10,
                  int             numDownPyrs = 2,
                  int             searchExpansion = 15);

/*=============================================================================
  MultipleMaxLoc
  Searches an image for multiple maxima
  Assumes a single channel, floating point image
  Parameters:
    image - the input image, generally the result from a cvMatchTemplate call
    locations - array of CvPoint (pass in a NULL point)
    numMaxima - the maximum number of best match maxima to locate
*/
void
MultipleMaxLoc(const Mat& image,
               cv::Point**    locations,
               int        numMaxima);

/*=============================================================================
  DrawFoundTargets
  Draws a rectangle of dimension size, at the given positions in the list,
  in the given RGB color space
  Parameters:
    image - a color image to draw on
    size - the size of the rectangle to draw
    pointsList - a list of points where a rectangle should be drawn
    confidencesList - a list of the confidences associated with the points
    red - the red value (0-255)
    green - the green value (0-255)
    blue - the blue value (0-255)
*/
void
DrawFoundTargets(Mat*                  image,
                 const cv::Size&           size,
                 const vector<cv::Point>&  pointsList,
                 const vector<double>& confidencesList,
                 int                   red   = 0,
                 int                   green = 255,
                 int                   blue  = 0);

#endif
