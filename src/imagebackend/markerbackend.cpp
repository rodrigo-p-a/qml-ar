#include "markerbackend.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "QtOpenCV/cvmatandqimage.h"

using namespace cv;

MarkerBackEnd::MarkerBackEnd(QQuickImageProvider *image_provider, MarkerDetector* marker_detector) : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    provider = image_provider;
    detector = marker_detector;
}

QPixmap MarkerBackEnd::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{ Q_UNUSED(id)
    // obtain input image from camera
    QPixmap input = provider->requestPixmap("raw", size, requestedSize);

    // send input to marker detector
    detector->setInput(input.toImage());

    // detect markers
    detector->process();

    // return the preview
    return QPixmap::fromImage(detector->getPreview());
}

MarkerBackEnd::~MarkerBackEnd()
{
}