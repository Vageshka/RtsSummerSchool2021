#include "opencv2/opencv.hpp"
#include "unistd.h"
#include "json.hpp"
#include "mosquitto.h"

using namespace cv;
using json = nlohmann::json;

#define mqtt_host "localhost"
#define mqtt_port 1883

struct mosquitto *mosquit;

void to_json(json &j, int num, int x1, int y1)
{
    j = json{
        { "obj_num", num },
        { "vector1", {
            { "x", x1 },
            { "y", y1 }
            }
        }
    };
}

Point rect_center(Rect r)
{
    return Point(r.x+r.width/2,r.y+r.height/2);
}

void draw_trajs(Mat &im, std::vector<Point> *traj)
{
    for(int i=0; i < 2; i++){
        std::vector<Point> &t = traj[i];
        int n = t.size() - 1;
        for(int j=0; j < n; j++)
            line(im, t[j], t[j+1],Scalar(15,255,255), 2);
    }
}

void filter(std::vector<Point> &filtered_traj, std::vector<Point> &last_3_unfiltered, Point new_point){
    if(last_3_unfiltered.size() == 3){
        Point sum = new_point;
        for(int i=0; i<3; i++)
            sum +=last_3_unfiltered[i];
        filtered_traj.push_back(sum/4);
        last_3_unfiltered.erase(last_3_unfiltered.begin());
        last_3_unfiltered.push_back(new_point);
    }
    else{
        filtered_traj.push_back(new_point);
        last_3_unfiltered.push_back(new_point);
    }
}

void recogniseStickersByThreshold(Mat image,std::vector<Rect> &rects, 
                                    std::vector<Point> *traj, std::vector<Point> *last_3) {
    Mat image_hsv;
    std::vector< std::vector<Point> > contours;
    Mat tmp_img(image.size(),CV_8U);
    cvtColor(image, image_hsv, COLOR_BGR2HSV );
    Scalar min_max_scals[4] = { Scalar(12,50,210), Scalar(16,255,255),
                            Scalar(25,0,100), Scalar(60,80,255) };
    for(int i = 0; i < 4; i+=2){
        inRange(image_hsv,
                min_max_scals[i],
                min_max_scals[i+1],
                tmp_img);
        dilate(tmp_img,tmp_img,Mat(),Point(-1,-1),3);
        erode(tmp_img,tmp_img,Mat(),Point(-1,-1),1);
        findContours(tmp_img,contours,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        if(contours.size()){
            Rect max_br = boundingRect(contours[0]);
            for(uint i=0; i < contours.size(); i++){
                Rect br = boundingRect(contours[i]);
                if(br.area() > max_br.area()) max_br = br;
            }
            if(max_br.area() > 1000) {
                rects.push_back(max_br);
                filter(traj[i/2], last_3[i/2], rect_center(max_br));
                // traj[i/2].push_back(rect_center(max_br));
                // filter(traj[i/2]);
                json j;
                to_json(j, i/2, traj[i/2].back().x, traj[i/2].back().y);  
                std::stringstream ss;
                std::string str;
                ss << j.dump();
                ss >> str;
                mosquitto_publish(mosquit, NULL, "test", str.size(), str.c_str(), 0, false); 
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int connection;
    mosquit = mosquitto_new("test", true, NULL);    
    connection = mosquitto_connect(mosquit, mqtt_host, mqtt_port, 60);
    if (connection != 0)
    {
        printf(" Error\n");
        mosquitto_destroy(mosquit);
        return -1;
    }
    VideoCapture cap("video.MOV"); 
    int frame_counter;
    std::vector<Point> trajectory[2];
    std::vector<Point> last_3[2];
    if ( !cap.isOpened() )
        return -1;
    double fps = cap.get(CV_CAP_PROP_FPS); 
    std::cout << "Frame per seconds : " << fps << std::endl;
    namedWindow("MyVideo", CV_WINDOW_AUTOSIZE);
    while(1) {
        Mat frame;
        bool bSuccess = cap.read(frame); 
        if (!bSuccess) {
            std::cout << "Cannot read the frame from video file" << std::endl;
            break;
        }
        frame_counter++;
        std::vector<Rect> rects;
        // resize(frame,frame,Size(800,600));
        recogniseStickersByThreshold(frame, rects, trajectory, last_3);
        for(uint i=0; i < rects.size(); i++)
            rectangle(frame, rects[i], Scalar(0,250,0), 3);
        draw_trajs(frame, trajectory);
        imshow("MyVideo", frame);
        // sleep(1);
        if(waitKey(30) == 27) {
            break;
        }
    }
    mosquitto_disconnect(mosquit);
    mosquitto_destroy(mosquit);
    return 0;
}
