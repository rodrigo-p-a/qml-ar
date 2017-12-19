#include "posecamerapnp.h"
#include <QtCore>
#include <QMatrix4x4>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <QtOpenCV/cvmatandqimage.h>

Pose CameraPoseEstimatorCorrespondences::estimate(CalibratedCamera *camera, WorldImageCorrespondences *correspondences)
{
    Q_ASSERT(camera != NULL);
    Q_ASSERT(correspondences != NULL);
    Q_ASSERT(correspondences->size() > 0);

    // number of points in correspondences
    int points_n = correspondences->size();

    // matrix for world points
    cv::Mat cv_world_points(points_n, 3, CV_64F);

    // matrix for image points
    cv::Mat cv_image_points(points_n, 2, CV_64F);

    // filling matrices from correspondences
    for(int i = 0; i < points_n; i++)
    {
        cv_world_points.at<double>(i, 0) = correspondences->getWorldPoint(i).x();
        cv_world_points.at<double>(i, 1) = correspondences->getWorldPoint(i).y();
        cv_world_points.at<double>(i, 2) = correspondences->getWorldPoint(i).z();

        cv_image_points.at<double>(i, 0) = correspondences->getImagePoint(i).x();
        cv_image_points.at<double>(i, 1) = correspondences->getImagePoint(i).y();
    }

    // camera matrix
    cv::Mat cv_camera_matrix(3, 3, CV_64F);

    // filling camera matrix
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            cv_camera_matrix.at<double>(i, j) = camera->getMatrix()(i, j);

    // rotation vector
    cv::Mat rvec;

    // translation vector
    cv::Mat tvec;

    // solving PnP task
    bool res = cv::solvePnP(cv_world_points, cv_image_points, cv_camera_matrix, cv::Mat(), rvec, tvec, false, CV_ITERATIVE);

    // if failed, return invalid pose
    if(!res) return Pose();

    // translation vector (Qt)
    QVector3D tvec_qt;
    for(int i = 0; i < 3; i++)
        tvec_qt[i] = tvec.at<double>(i, 0);

    // rotation vector (Qt)
    QVector4D rvec_qt;
    for(int i = 0; i < 4; i++)
        rvec_qt[i] = rvec.at<double>(i, 0);

    // mirroring translation
    tvec_qt = -tvec_qt;

    // returning pose
    return Pose(tvec_qt, rvec_qt);
}
