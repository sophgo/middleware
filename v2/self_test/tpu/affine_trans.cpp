/* A simple example */
#include <ctime>
#include <iostream>
#include "opencv2/opencv.hpp"

#include <cvi_comm_gdc.h>

using namespace std;
using namespace cv;

using cv::Mat;
using std::string;
using std::vector;

Mat tformfwd(const Mat &trans, const Mat &uv) {
	Mat uv_h = Mat::ones(uv.rows, 3, CV_64FC1);
	Mat xv_h = uv_h * trans;

	uv.copyTo(uv_h(cv::Rect(0, 0, 2, uv.rows)));
	return xv_h(cv::Rect(0, 0, 2, uv.rows));
}

Mat find_none_flectives_similarity(const Mat &uv, const Mat &xy) {
	Mat A = Mat::zeros(2 * xy.rows, 4, CV_64FC1);
	Mat b = Mat::zeros(2 * xy.rows, 1, CV_64FC1);
	Mat x = Mat::zeros(4, 1, CV_64FC1);

	xy(cv::Rect(0, 0, 1, xy.rows)).copyTo(A(cv::Rect(0, 0, 1, xy.rows)));  // x
	xy(cv::Rect(1, 0, 1, xy.rows)).copyTo(A(cv::Rect(1, 0, 1, xy.rows)));  // y
	A(cv::Rect(2, 0, 1, xy.rows)).setTo(1.);

	xy(cv::Rect(1, 0, 1, xy.rows)).copyTo(A(cv::Rect(0, xy.rows, 1, xy.rows)));    // y
	(xy(cv::Rect(0, 0, 1, xy.rows))).copyTo(A(cv::Rect(1, xy.rows, 1, xy.rows)));  //-x
	A(cv::Rect(1, xy.rows, 1, xy.rows)) *= -1;
	A(cv::Rect(3, xy.rows, 1, xy.rows)).setTo(1.);

	uv(cv::Rect(0, 0, 1, uv.rows)).copyTo(b(cv::Rect(0, 0, 1, uv.rows)));
	uv(cv::Rect(1, 0, 1, uv.rows)).copyTo(b(cv::Rect(0, uv.rows, 1, uv.rows)));

	cv::solve(A, b, x, cv::DECOMP_SVD);
	Mat trans_inv = (cv::Mat_<double>(3, 3) <<
			x.at<double>(0), -x.at<double>(1), 0,
			x.at<double>(1), x.at<double>(0), 0,
			x.at<double>(2), x.at<double>(3), 1);
	Mat trans = trans_inv.inv(cv::DECOMP_SVD);

	trans.at<double>(0, 2) = 0;
	trans.at<double>(1, 2) = 0;
	trans.at<double>(2, 2) = 1;

	return trans;
}

Mat find_similarity(const Mat &uv, const Mat &xy) {
	Mat trans1 = find_none_flectives_similarity(uv, xy);
	Mat xy_reflect = xy;

	xy_reflect(cv::Rect(0, 0, 1, xy.rows)) *= -1;

	Mat trans2r = find_none_flectives_similarity(uv, xy_reflect);
	Mat reflect = (cv::Mat_<double>(3, 3) << -1, 0, 0, 0, 1, 0, 0, 0, 1);
	Mat trans2 = trans2r * reflect;
	Mat xy1 = tformfwd(trans1, uv);
	double norm1 = cv::norm(xy1 - xy);
	Mat xy2 = tformfwd(trans2, uv);
	double norm2 = cv::norm(xy2 - xy);
	Mat trans = (norm1 < norm2) ? trans1 : trans2;

	return trans;
}

Mat get_similarity_transform(const vector<cv::Point2f> &src_pts,
	const vector<cv::Point2f> &dest_pts, bool reflective) {
	Mat src((int)src_pts.size(), 2, CV_32FC1, (void *)(&src_pts[0].x));
	src.convertTo(src, CV_64FC1);

	Mat dst((int)dest_pts.size(), 2, CV_32FC1, (void *)(&dest_pts[0].x));
	dst.convertTo(dst, CV_64FC1);

	Mat trans = reflective ? find_similarity(src, dst) : find_none_flectives_similarity(src, dst);
	return trans(cv::Rect(0, 0, 2, trans.rows)).t();
}

extern "C" void find_bounding_box(const POINT2F_S *face_info, POINT2F_S *face_box);

void find_bounding_box(const POINT2F_S *face_landmark, POINT2F_S *face_box)
{
	int i = 0;

	vector<cv::Point2f> detect_points;
	for (int j = 0; j < 5; ++j) {
		cv::Point2f e;
		e.x = face_landmark[j].x;
		e.y = face_landmark[j].y;
		detect_points.emplace_back(e);
	}

	vector<cv::Point2f> reference_points;
	vector<cv::Point2f> search_points;
	reference_points = { {38.29459953, 51.69630051},
			     {73.53179932, 51.50139999},
			     {56.02519989, 71.73660278},
			     {41.54930115, 92.3655014},
			     {70.72990036, 92.20410156}};
	search_points = { {0.0, 0.0},
			  {111.0, 0.0},
			  {0.0, 111.0},
			  {111.0, 111.0}};

	// find face-rect in image
	Mat tfm = get_similarity_transform(detect_points, reference_points, true);

	double t, a, b, c;

	/*
	 * wrapaffine() transforms the image use the matrix below:
	 *   Sx * M00 + Sy * M01 + M02 = Dx
	 *   Sx * M10 + Sy * M11 + M12 = Dy
	 *
	 * We want to get Src point by Dst point and matrix as below:
	 *   Sx = Dx * a + Dx * b + c
	 *
	 * 1) if angle is almost 0, which means cos@ = 0 and M01/M10 = 0
	 *  Sx * M00 = Dx - M02
	 *  a = 1/M00
	 *  b = 0
	 *  c = -M02/M00
	 * 2) angle > 1
	 *  Sx = ((Dx - M02) / M01 - (Dy - M12) / M11) / (M00/M01 - M10/M11)
	 *  t = (M00/M01 - M10/M11)
	 *  a = 1/M01/t
	 *  b = -1/M11/t
	 *  c = (-M02/M01 + M12/M11) / t
	 *
	 */
	if (abs(tfm.at<double>(0, 1)) <  0.0017452) {
		a = 1 / tfm.at<double>(0, 0);
		b = 0;
		c = -tfm.at<double>(0, 2) / tfm.at<double>(0, 0);
	} else {
		t = (tfm.at<double>(0, 0) / tfm.at<double>(0, 1) - tfm.at<double>(1, 0) / tfm.at<double>(1, 1));
		a = 1 / tfm.at<double>(0, 1) / t;
		b = -1 / tfm.at<double>(1, 1) / t;
		c = (-tfm.at<double>(0, 2) / tfm.at<double>(0, 1) + tfm.at<double>(1, 2) / tfm.at<double>(1, 1))/t;
	}

	//cout << "box pos: { ";
	for (auto &e : search_points) {
		face_box[i].x = e.x * a + e.y * b + c;
		face_box[i].y = (e.y - tfm.at<double>(1, 2)
			      - face_box[i].x * tfm.at<double>(1, 0)) / tfm.at<double>(1, 1);
		++i;

		//cout << "{.x = " << face_box[i].x << ", .y = " << face_box[i].y << "}, ";
	}
	//cout << "}," << endl;
}

