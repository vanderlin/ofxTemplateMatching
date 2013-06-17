/***************************************************************************
 *            FastMatchTemplate.cpp
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
 */

#include <math.h>
#include "FastMatchTemplate.h"

//=============================================================================
// Assumes that source image exists and numDownPyrs > 1, no ROIs for either
//  image, and both images have the same depth and number of channels
bool
FastMatchTemplate( const Mat&      source,
                   const Mat&      target,
                   vector<cv::Point>*  foundPointsList,
                   vector<double>* confidencesList,
                   int             matchPercentage,
                   bool            findMultipleTargets,
                   int             numMaxima,
                   int             numDownPyrs,
                   int             searchExpansion )
{
    // make sure that the template image is smaller than the source
    if(target.size().width > source.size().width ||
        target.size().height > source.size().height)
    {
        printf( "\nSource image must be larger than target image.\n" );
        return false;
    }

    if(source.depth() != target.depth())
    {
        printf( "\nSource image and target image must have same depth.\n" );
        return false;
    }

    if(source.channels() != target.channels())
    {
        printf("\nSource image and target image must have same number of channels.\n" );
        return false;
    }

    cv::Size sourceSize = source.size();
    cv::Size targetSize = target.size();

    // create copies of the images to modify
    Mat copyOfSource = source.clone();
    Mat copyOfTarget = target.clone();

    // down pyramid the images
    for(int ii = 0; ii < numDownPyrs; ii++)
    {
        // start with the source image
        sourceSize.width  = (sourceSize.width  + 1) / 2;
        sourceSize.height = (sourceSize.height + 1) / 2;

        Mat smallSource(sourceSize, source.type());
        pyrDown(copyOfSource, smallSource);

        // prepare for next loop, if any
        copyOfSource = smallSource.clone();

        // next, do the target
        targetSize.width  = (targetSize.width  + 1) / 2;
        targetSize.height = (targetSize.height + 1) / 2;

        Mat smallTarget(targetSize, target.type());
        pyrDown(copyOfTarget, smallTarget);

        // prepare for next loop, if any
        copyOfTarget = smallTarget.clone();
    }

    // perform the match on the shrunken images
    cv::Size smallTargetSize = copyOfTarget.size();
    cv::Size smallSourceSize = copyOfSource.size();

    cv::Size resultSize;
    resultSize.width = smallSourceSize.width - smallTargetSize.width + 1;
    resultSize.height = smallSourceSize.height - smallTargetSize.height + 1;

    Mat result(resultSize, CV_32FC1);
    matchTemplate(copyOfSource, copyOfTarget, result, CV_TM_CCOEFF_NORMED);

    // find the top match locations
    cv::Point* locations = NULL;
    MultipleMaxLoc(result, &locations, numMaxima);

    // search the large images at the returned locations
    sourceSize = source.size();
    targetSize = target.size();

    // create a copy of the source in order to adjust its ROI for searching
    for(int currMax = 0; currMax < numMaxima; currMax++)
    {
        // transform the point to its corresponding point in the larger image
        locations[currMax].x *= (int)pow(2.0f, numDownPyrs);
        locations[currMax].y *= (int)pow(2.0f, numDownPyrs);
        locations[currMax].x += targetSize.width / 2;
        locations[currMax].y += targetSize.height / 2;

        const cv::Point& searchPoint = locations[currMax];

        // if we are searching for multiple targets and we have found a target or
        //  multiple targets, we don't want to search in the same location(s) again
        if(findMultipleTargets && !foundPointsList->empty())
        {
            bool thisTargetFound = false;

            int numPoints = foundPointsList->size();
            for(int currPoint = 0; currPoint < numPoints; currPoint++)
            {
                const cv::Point& foundPoint = (*foundPointsList)[currPoint];
                if(abs(searchPoint.x - foundPoint.x) <= searchExpansion * 2 &&
                    abs(searchPoint.y - foundPoint.y) <= searchExpansion * 2)
                {
                    thisTargetFound = true;
                    break;
                }
            }

            // if the current target has been found, continue onto the next point
            if(thisTargetFound)
            {
                continue;
            }
        }

        // set the source image's ROI to slightly larger than the target image,
        //  centred at the current point
        cv::Rect searchRoi;
        searchRoi.x = searchPoint.x - (target.size().width) / 2 - searchExpansion;
        searchRoi.y = searchPoint.y - (target.size().height) / 2 - searchExpansion;
        searchRoi.width = target.size().width + searchExpansion * 2;
        searchRoi.height = target.size().height + searchExpansion * 2;

        // make sure ROI doesn't extend outside of image
        if(searchRoi.x < 0)
        {
            searchRoi.x = 0;
        }

        if(searchRoi.y < 0)
        {
            searchRoi.y = 0;
        }

        if((searchRoi.x + searchRoi.width) > (sourceSize.width - 1))
        {
            int numPixelsOver
                = (searchRoi.x + searchRoi.width) - (sourceSize.width - 1);

            searchRoi.width -= numPixelsOver;
        }

        if((searchRoi.y + searchRoi.height) > (sourceSize.height - 1))
        {
            int numPixelsOver
                = (searchRoi.y + searchRoi.height) - (sourceSize.height - 1);

            searchRoi.height -= numPixelsOver;
        }

        Mat searchImage = Mat(source, searchRoi);

        // perform the search on the large images
        resultSize.width = searchRoi.width - target.size().width + 1;
        resultSize.height = searchRoi.height - target.size().height + 1;

        result = Mat(resultSize, CV_32FC1);
        matchTemplate(searchImage, target, result, CV_TM_CCOEFF_NORMED);

        // find the best match location
        double minValue, maxValue;
        cv::Point minLoc, maxLoc;
        minMaxLoc(result, &minValue, &maxValue, &minLoc, &maxLoc);
        maxValue *= 100;

        // transform point back to original image
        maxLoc.x += searchRoi.x + target.size().width / 2;
        maxLoc.y += searchRoi.y + target.size().height / 2;

        if(maxValue >= matchPercentage)
        {
            // add the point to the list
            foundPointsList->push_back(maxLoc);
            confidencesList->push_back(maxValue);

            // if we are only looking for a single target, we have found it, so we
            //  can return
            if(!findMultipleTargets)
            {
                break;
            }
        }
    }

    if(foundPointsList->empty())
    {
        //printf( "\nTarget was not found to required confidence of %d.\n",
        //    matchPercentage );
    }

    delete [] locations;

    return true;
}

//=============================================================================

void MultipleMaxLoc(const Mat& image,
                    cv::Point**    locations,
                    int        numMaxima)
{
    // initialize input variable locations
    *locations = new cv::Point[numMaxima];

    // create array for tracking maxima
    float* maxima = new float[numMaxima];
    for(int i = 0; i < numMaxima; i++)
    {
        maxima[i] = 0.0;
    }

    cv::Size size = image.size();

    // extract the raw data for analysis
    for(int y = 0; y < size.height; y++)
    {
        for(int x = 0; x < size.width; x++)
        {
            float data = image.at<float>(y, x);

            // insert the data value into the array if it is greater than any of the
            //  other array values, and bump the other values below it, down
            for(int j = 0; j < numMaxima; j++)
            {
                // require at least 50% confidence on the sub-sampled image
                // in order to make this as fast as possible
                if(data > 0.5 && data > maxima[j])
                {
                    // move the maxima down
                    for(int k = numMaxima - 1; k > j; k--)
                    {
                        maxima[k] = maxima[k-1];
                        (*locations)[k] = ( *locations )[k-1];
                    }

                    // insert the value
                    maxima[j] = data;
                    (*locations)[j].x = x;
                    (*locations)[j].y = y;
                    break;
                }
            }
        }
    }

    delete [] maxima;
}

//=============================================================================

void
DrawFoundTargets(Mat*                  image,
                 const cv::Size&           size,
                 const vector<cv::Point>&  pointsList,
                 const vector<double>& confidencesList,
                 int                   red,
                 int                   green,
                 int                   blue)
{
    int numPoints = pointsList.size();
    for(int currPoint = 0; currPoint < numPoints; currPoint++)
    {
        const cv::Point& point = pointsList[currPoint];

        // write the confidences to stdout
        /*printf("\nTarget found at (%d, %d), with confidence = %3.3f %%.\n",
            point.x,
            point.y,
            confidencesList[currPoint]);*/

        // draw a circle at the center
        circle(*image, point, 2, CV_RGB(red, green, blue));

        // draw a rectangle around the found target
        cv::Point topLeft;
        topLeft.x = point.x - size.width / 2;
        topLeft.y = point.y - size.height / 2;

        cv::Point bottomRight;
        bottomRight.x = point.x + size.width / 2;
        bottomRight.y = point.y + size.height / 2;

        rectangle(*image, topLeft, bottomRight, CV_RGB(red, green, blue));
    }
}
