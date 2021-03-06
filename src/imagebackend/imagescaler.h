#ifndef IMAGESCALER_H
#define IMAGESCALER_H

#include <QImage>
#include <QtConcurrent>
#include "imageproviderasync.h"

/*
 * This class scales images
 */

class ImageScaler : public ImageProviderAsync
{ Q_OBJECT
public slots:
    void setInput(QImage source);
    void handleResult();
private:
    QImage buffer;

    QFutureWatcher<QImage> watcher;

    int target_width;

    QImage input_buffer;
    bool input_buffer_nonempty;
public:
    ImageScaler(int target_width);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QImage scale(QImage source);
    ImageScaler(const ImageScaler &that);
    ImageScaler();
};

#endif // IMAGESCALER_H
