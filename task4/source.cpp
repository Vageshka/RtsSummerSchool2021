#include "opencv2/opencv.hpp"
#include "unistd.h"

using namespace cv;

void recogniseStickersByThreshold(Mat image,std::vector<Rect> &rects) {
    Mat image_hsv;
    std::vector< std::vector<Point> > contours;
    Mat tmp_img(image.size(),CV_8U);
    cvtColor(image, image_hsv, COLOR_BGR2HSV ); // Преобразуем в hsv
    Scalar min_max_scals[4] = { Scalar(12,50,210), Scalar(16,255,255),
                            Scalar(25,0,100), Scalar(60,80,255) };
    for(int i = 0; i < 4; i+=2){
        // Выделение подходящих по цвету областей. Цвет задается константой :)
        inRange(image_hsv,
                min_max_scals[i],
                min_max_scals[i+1],
                tmp_img);
        // "Замазать" огрехи в при выделении по цвету
        dilate(tmp_img,tmp_img,Mat(),Point(-1,-1),3);
        erode(tmp_img,tmp_img,Mat(),Point(-1,-1),1);
        //Выделение непрерывных областей
        findContours(tmp_img,contours,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        if(contours.size()){
            Rect max_br = boundingRect(contours[0]);
            for(uint i=0; i < contours.size(); i++){
                Rect br = boundingRect(contours[i]);
                if(br.area() > 400 && br.area() > max_br.area()) max_br = br;
            }
            rects.push_back(max_br);
        }
    }
}

int main(int argc, char* argv[]) {
    VideoCapture cap("video.MOV"); // open the video file for reading
    // VideoCapture cap(0);
    if ( !cap.isOpened() )
        return -1;
    //cap.set(CV_CAP_PROP_POS_MSEC, 300); //start the video at 300ms
    double fps = cap.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
    std::cout << "Frame per seconds : " << fps << std::endl;
    namedWindow("MyVideo", CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
    while(1) {
        Mat frame;
        bool bSuccess = cap.read(frame); // read a new frame from video
        if (!bSuccess) {
            std::cout << "Cannot read the frame from video file" << std::endl;
            break;
        }
        std::vector<Rect> rects;
        // resize(frame,frame,Size(1280,720));
        recogniseStickersByThreshold(frame, rects);
        for(uint i=0; i < rects.size(); i++)
            rectangle(frame, rects[i], Scalar(0,250,0),2);
        imshow("MyVideo", frame);
        if(waitKey(30) == 27) {
            break;
        }
    }
}