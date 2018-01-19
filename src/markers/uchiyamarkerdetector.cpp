#include "uchiyamarkerdetector.h"
#include <stdio.h>
#include <iostream>
#include <uchiya/mylib/mytimer.h>
#include <QDebug>
#include <QElapsedTimer>
#include <QMatrix4x4>
#include "QtOpenCV/cvmatandqimage.h"
#include "opencv2/core.hpp"
#include <vector>
#include <opencv2/calib3d.hpp>
#include "opencv2/imgproc.hpp"
#include "mymatconverter.h"
#include "timelogger.h"
#include "config.h"
#include <QGenericMatrix>
#include <QJsonArray>

using std::vector;

// see main.cpp and main.h from
// UchiyaMarkers project

UchiyaMarkerDetector::UchiyaMarkerDetector(): MarkerDetector()
{
    is_initialized = false;
}

void UchiyaMarkerDetector::initialize(int h, int w)
{
    Q_ASSERT(is_initialized == false);
    TimeLoggerLog("init %d %d", h, w);
    m_camimg.Init(w, h);		// allocate image
    m_img.Init(w, h);			// allocate image
    initMarkers();		// tracking initialization
    is_initialized = true;
}

void UchiyaMarkerDetector::initMarkers()
{
    m_llah.Init(m_img.w, m_img.h);		// set image size

    // going through all markers
    QMap<int, Marker>::iterator marker_in_map;
    for(marker_in_map = markers.begin(); marker_in_map != markers.end(); marker_in_map++)
    {
        // will write marker config here
        QString s;
        QTextStream ss(&s);

        // dots as json array
        QJsonArray dots = marker_in_map.value().getConfig().value("dots_uchiya").toArray();

        // going through dots and adding them to ss
        QJsonArray::iterator dot;
        ss << dots.size() << "\n";
        for(dot = dots.begin(); dot != dots.end(); dot++)
        {
            // adding dot coordinates
            ss << (*dot).toObject().value("x").toDouble() << " "
               << (*dot).toObject().value("y").toDouble() << "\n";
        }

        // rewinding the stream
        ss.seek(0);

        // adding paper with stream as input
        m_llah.AddPaper(ss);

        // marker added
        TimeLoggerLog("Marker %d loaded", marker_in_map.key());
    }
}

void UchiyaMarkerDetector::extractMarkers()
{
    // list of detected markers
    visible *papers = m_llah.GetVisiblePaper();

    // forget all detected markers
    // this removes point correspondences
    markers.undetect();

    // for detected papers
    for(visible::iterator itpa=(*papers).begin(); itpa!=(*papers).end(); itpa++)
    {
        // id of the marker in Uchiya library
        int id = (*itpa)->id - 1;

        // marker in the storage
        Marker* m = markers.getPointer(id);

        // no marker in the database
        // this happens if camera saw a marker
        // not from this activity
        if(m == NULL) continue;

        // homography matrix from Uchiya library
        // top-left 3x3 submatrix is used
        // warning: matrix uses mirrored y axis
        QMatrix4x4 homography = MyMatConverter::convert3x3((*itpa)->H);

        // Basic idea:
        // 1. Need: correspondences world <-> image
        // 2. Homography * Uchiya Coordinates (0-600 x 0-600) = Image coordinates of corners
        // 3. Obtaining world coordinates of marker corners from MarkerStorage
        // 4. Adding pairs (world corner, image corner) to Marker as a correspondence

        // get marker size in mm
        double marker_size = m->getSizeMM();

        // fill in the marker corners in the actual sheet (mm)
        // corners (in order): top-left, bottom-left, top-right, bottom-right
        // positioning (mm)    (0, 0)    (0, s)       (s, 0)     (s, s)
        QVector3D marker_tl = m->getPositionMM();
        QVector3D marker_bl = marker_tl;
        QVector3D marker_tr = marker_tl;
        QVector3D marker_br = marker_tl;

        marker_bl += QVector3D(0          , marker_size, 0);
        marker_tr += QVector3D(marker_size, 0          , 0);
        marker_br += QVector3D(marker_size, marker_size, 0);

        // marker coordinate system (marker has size 600)
        QVector3D marker_uchiya_tl_aeffine = QVector3D(0  , 600, 0);
        QVector3D marker_uchiya_bl_aeffine = QVector3D(0  , 0  , 0);
        QVector3D marker_uchiya_tr_aeffine = QVector3D(600, 600, 0);
        QVector3D marker_uchiya_br_aeffine = QVector3D(600, 0  , 0);

        // add all of the points to a vector
        QVector<QVector3D> marker_corners;
        marker_corners.append(marker_tl);
        marker_corners.append(marker_bl);
        marker_corners.append(marker_tr);
        marker_corners.append(marker_br);

        // all of the marker corners corresponding to real world corners
        QVector<QVector3D> marker_uchiya_affine_corners;
        marker_uchiya_affine_corners.push_back(marker_uchiya_tl_aeffine);
        marker_uchiya_affine_corners.push_back(marker_uchiya_bl_aeffine);
        marker_uchiya_affine_corners.push_back(marker_uchiya_tr_aeffine);
        marker_uchiya_affine_corners.push_back(marker_uchiya_br_aeffine);

        // length should be equal since number of corners is the same
        Q_ASSERT(marker_uchiya_affine_corners.size() == marker_corners.size());

        Q_ASSERT(marker_uchiya_affine_corners.size() == 4);

        // colors for the corners
        QVector<QVector3D> colors;
        colors.append(QVector3D(255, 0, 0));
        colors.append(QVector3D(0, 255, 0));
        colors.append(QVector3D(0, 0, 255));
        colors.append(QVector3D(255, 255, 255));

        // go through the vector, compute image point and
        // add a correspondence
        for(int i = 0; i < marker_corners.size(); i++)
        {
            // obtain current marker point in the world coordinates
            QVector3D world_marker_point = marker_corners[i];

            // get uchiya marker coordinates point to 2D form (ignore z)
            QVector4D uchiya_marker_point_affine = marker_uchiya_affine_corners[i];
            uchiya_marker_point_affine.setZ(1);

            // obtain marker image point using homography matrix
            QVector4D image_marker_point_affine = homography.map(uchiya_marker_point_affine);

            // check if the z dimension is nonzero
            // otherwise, perspective division is impossible
            // (point is certainly invisible)
            Q_ASSERT(image_marker_point_affine.z() != 0);

            // convert image point from affine to 2D form
            QVector2D image_marker_point = QVector2D(image_marker_point_affine.x() / image_marker_point_affine.z(),
                                                     image_marker_point_affine.y() / image_marker_point_affine.z());

            // Uchiya library mirrors y axis
            image_marker_point.setY(m_img.h - image_marker_point.y());

            // draw the corner with its own color
            QVector3D color = colors[i];
            m_img.Circle(image_marker_point.x(), image_marker_point.y(), 5, 1, color.x(), color.y(), color.z());

            // add the calculated correspondence
            m->addCorrespondence(world_marker_point, image_marker_point);
        }
    }
}

void UchiyaMarkerDetector::drawPreview() {
    // show image
    m_llah.DrawPts(m_img);

    extractMarkers();
}

void UchiyaMarkerDetector::preparePreview()
{
    IplImage dst = *(IplImage*) m_img;
    // second parameter disables data copying
    cv::Mat dst2mat = cv::cvarrToMat(&dst, false);
    output_buffer = QtOcv::mat2Image_shared(dst2mat);
}

void UchiyaMarkerDetector::prepareInput()
{
    TimeLoggerLog("%s", "Detecting blobs");
    blob_detector.detectBlobs(input_buffer, max_dots);

    TimeLoggerLog("%s", "Drawing blobs");
    QImage blobs = blob_detector.drawBlobs();

    TimeLoggerLog("%s", "Copying input");
    cv::Mat src2mat = QtOcv::image2Mat_shared(blobs);
    static IplImage src2mat2ipl;
    src2mat2ipl = (IplImage) src2mat;
    m_camimg.m_img = &src2mat2ipl;

    TimeLoggerLog("%s", "Copying output");
    static cv::Mat src2mat1;
    src2mat1 = QtOcv::image2Mat(output_buffer_background, CV_8UC3, QtOcv::MCO_RGB);
    static IplImage src2mat2ipl1;
    src2mat2ipl1 = src2mat1;
    m_img.m_img = &src2mat2ipl1;
}

void UchiyaMarkerDetector::process()
{
    TimeLoggerProfile("%s", "Start marker detection");

    // if the image is invalid, no need for marker detection
    // this happens at the start of the application
    if(input_buffer.height() * input_buffer.width() <= 0)
    {
        TimeLoggerLog("%s", "Empty image");
        return;
    }

    // initialize marker detection pipeline
    // when we know the camera image size
    if(!is_initialized)
    {
        TimeLoggerLog("%s", "Initializing UchiyaMarkerDetector");
        initialize(input_buffer.height(), input_buffer.width());
    }

    Q_ASSERT(is_initialized);

    TimeLoggerLog("%s", "Preparing input");
    // putting camera src image to Uchiya pipeline
    prepareInput();

    TimeLoggerLog("%s", "Extracting blobs");

    // passing detected blobs to the library
    m_llah.Extract(m_camimg);

    TimeLoggerLog("%s", "Setting points");

    m_llah.SetPts();
    //m_llah.SetPts(blob_detector.getBlobs());
    m_llah.CoordinateTransform(static_cast<double>(m_camimg.h));

    TimeLoggerLog("%s", "Tracking");

    m_llah.RetrievebyTracking();
    m_llah.FindPaper(4);

    TimeLoggerLog("%s", "Matching");

    m_llah.RetrievebyMatching();
    m_llah.FindPaper(8);

    // creating preview image

    TimeLoggerLog("%s", "Creating preview");

    drawPreview();

    // obtaining Uchiya image dst and returning it
    preparePreview();

    TimeLoggerProfile("%s", "End marker detection");

    // tell the parent to recompute projection matrix
    emit markersUpdated();
}
