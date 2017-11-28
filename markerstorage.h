#ifndef MARKERSTORAGE_H
#define MARKERSTORAGE_H

#include <QMap>
#include <QIODevice>
#include "marker.h"

/*
 * This class stores the set of used markers
 * loads them from a file and outputs
 * the coordinates of each of them
 * on a request
 */

/*
 * Json config format example:
 * {
 *   "markers": [
 *     {
 *       "name": "blue one at the top",
 *       "id": 5,
 *       "x_mm": 1.4,
 *       "y_mm": 111
 *     },
 *     {
 *       "name": "red one in the middle",
 *       "id": 8,
 *       "x_mm": 122.1,
 *       "y_mm": 100
 *     }
 *   ]
 * }
 *
 */

class MarkerStorage
{
private:
    // stored markers
    QMap<int, Marker> markers;
public:
    // initialize empty storage
    MarkerStorage();

    // fill with data from a file/other QIODevice
    void populate(QIODevice &input);

    // fill with data from file
    void populateFromFile(QString filename);

    // fill with data from the string
    void populate(QString data);

    // obtain a marker
    // calls qFatal on missing key
    Marker get(int marker_id);

};

#endif // MARKERSTORAGE_H