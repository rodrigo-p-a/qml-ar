#include "blobdetector.h"
#include "opencv2/imgproc.hpp"
#include "QtOpenCV/cvmatandqimage.h"
#include "opencv2/features2d.hpp"
#include "qvideoframehelpers.h"
#include "timelogger.h"
#include <QVector>
#include <QVector2D>

BlobDetector::BlobDetector(const BlobDetector& detector) : BlobDetector()
{
    this->parameters = detector.parameters;
    this->max_blobs = detector.max_blobs;
}

BlobDetector::BlobDetector(int max_blobs) : BlobDetector()
{
    this->max_blobs = max_blobs;
}

BlobDetector::BlobDetector()
{
    // set blob detector parameters
    parameters.minThreshold = 50;
    parameters.maxThreshold = 200;
    parameters.thresholdStep = 10;
    parameters.filterByArea = true;
    parameters.minArea = 10;
    parameters.filterByCircularity = true;
    parameters.minCircularity = .5;
    parameters.filterByConvexity = true;
    parameters.minConvexity = .9;
    parameters.filterByInertia = true;
    parameters.minInertiaRatio = .5;

    // create blob detector
    detector = cv::SimpleBlobDetector::create(parameters);

    // init blur size parameter
    blur_size.height = 4;
    blur_size.width = 4;

    // no blobs now
    is_initialized = false;

    max_blobs = 50;

    buffer_is_nonempty = false;

    this->last_output = QVideoFrameHelpers::empty();

    connect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));
}

void BlobDetector::handleFinished()
{
    QPair<QVector<QVector2D>, QImage> result = watcher.result();
    last_output = result.second;

    // output drawn blobs
    emit imageAvailable(last_output);

    // pass the vector with keypoints
    emit blobsUpdated(result.first);

    // schedule another run if neccessary
    if(buffer_is_nonempty)
    {
        buffer_is_nonempty = false;
        setInput(input_buffer);
    }
}



QImage BlobDetector::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{ Q_UNUSED(id) Q_UNUSED(size) Q_UNUSED(requestedSize)
    return last_output;
}

void BlobDetector::setInput(QImage img)
{
    input_buffer = img;

    if(!watcher.isRunning())
    {
        QFuture<QPair<QVector<QVector2D>, QImage>> future = QtConcurrent::run(*this, &BlobDetector::getAndDraw, img);
        watcher.setFuture(future);
    }
    else buffer_is_nonempty = true;
}

QImage BlobDetector::drawBlobs()
{
    Q_ASSERT(is_initialized);

    // store the result in mat
    if(result.rows != last_input.height() || result.cols != last_input.width())
        result = cv::Mat(last_input.height(), last_input.width(), CV_8UC3, cv::Scalar(255, 255, 255));
    else result.setTo(cv::Scalar(255, 255, 255));

    // go through all blobs
    std::vector<cv::KeyPoint>::iterator it;
    int i = 0;
    for(it = keypoints.begin(); it != keypoints.end(); it++, i++)
    {
        // draw a white circle on the place of a blob
        cv::KeyPoint kp = (*it);

        // preserving blob color
        QColor color = last_input.pixel(kp.pt.x, kp.pt.y);
        int r, g, b;
        color.getRgb(&r, &g, &b);

        // drawing circle
        cv::circle(result, kp.pt, 2, cv::Scalar(b, g, r), -1);
    }

    last_output = QtOcv::mat2Image(result);

    // return mat -> qt image
    return last_output;
}

std::vector<cv::KeyPoint> BlobDetector::getBlobs()
{
    Q_ASSERT(is_initialized);
    return keypoints;
}

QPair<QVector<QVector2D>, QImage> BlobDetector::getAndDraw(QImage img)
{
    TimeLoggerLog("%s", "[ANALYZE] Begin BlobDetector");
    QVector<QVector2D> blobs = detectBlobs(img);
    QImage drawn = drawBlobs();

    QPair<QVector<QVector2D>, QImage> result = qMakePair(blobs, drawn);

    TimeLoggerLog("%s", "[ANALYZE] End BlobDetector");

    return result;
}

QVector<QVector2D> BlobDetector::detectBlobs(QImage source)
{
    TimeLoggerLog("%s", "Saving input");
    // setting last input image
    last_input = source;

    // get data qimage -> mat
    cv::Mat source_cv = QtOcv::image2Mat_shared(source);

    TimeLoggerLog("%s", "Converting to grayscale");

    // color -> grayscale
    cv::Mat source_cv_gray;
    cv::cvtColor(source_cv, source_cv_gray, cv::COLOR_RGB2GRAY);

    TimeLoggerLog("%s", "Blurring");

    // blur the image
    cv::Mat blurred = source_cv_gray;
    cv::blur(source_cv_gray, blurred, blur_size);

    TimeLoggerLog("%s", "Detecting keypoints");

    // detect blobs
    keypoints.clear();
    detector->detect(blurred, keypoints);

    // removing extra blobs
    if(max_blobs > 0 && max_blobs < (int) keypoints.size())
        keypoints.resize(max_blobs);

    // creating qt vector with keypoints
    QVector<QVector2D> blobs_qt;
    for(unsigned i = 0; i < keypoints.size(); i++)
        blobs_qt.append(QVector2D(keypoints[i].pt.x, keypoints[i].pt.y));

    // now the object is initialized
    is_initialized = true;

    return blobs_qt;
}
