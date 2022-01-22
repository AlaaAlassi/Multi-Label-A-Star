#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "CellData.hpp"
#include "Warehouse.hpp"
#include "Robot.hpp"
#include "Graphics.hpp"
#include <vector>
#include <thread>
#include <future>

using namespace cv;

const int MAX_MONITOR_LENGTH = 1080;
const int MAX_MONITOR_WIDTH = 1920;

int main(int argc, char **argv)
{
    // get visulize the warehouse
    Warehouse warehouse = Warehouse();
    double aspectRatio = 0.7;
    int windowWidth = int(aspectRatio * MAX_MONITOR_WIDTH);
    int windowLength = int(aspectRatio * MAX_MONITOR_LENGTH);
    Graphics viewer = Graphics(windowLength, windowWidth, warehouse._map);

    // construct dummy robots
    auto rob1 = std::make_shared<Robot>(1,warehouse._map._cells[0]->cartesianPosition,warehouse._map.getCellSize()*0.5);
    auto rob2 = std::make_shared<Robot>(2,warehouse._map._cells[0]->cartesianPosition,warehouse._map.getCellSize()*0.5);
    viewer._robots.push_back(rob1);
    viewer._robots.push_back(rob2);
    viewer.loadBackgroundImg();

    // run viewer thread
    std::thread simulationThread(&Graphics::run, &viewer);

    //create a path

    int pathSize = 10;
    for(int j=0;j<pathSize;j++){
        rob1->_path.push_back(warehouse._map._cells[j]);
    }

    Cartesian2DPoint goal;
    Cartesian2DPoint goal2;
    int counter = 0;
    bool forward = false;
    for (int i = 0; i < warehouse._map.getNumberOfRows();i++)
    {
        forward = !forward;
        for (int j = 0; j < warehouse._map.getNumberOfColumns(); j++)
        {
            goal = warehouse._map.getCell(i, counter)->cartesianPosition;
            goal2 = warehouse._map.getCell(i, counter)->cartesianPosition;
            std::future<bool> ftr = std::async(std::launch::async, &Robot::trackGoalPosition, rob1, goal);
            std::future<bool> ftr2 = std::async(std::launch::async, &Robot::trackGoalPosition, rob2, goal2);
            ftr.get();
            ftr2.get();
            if (forward && j!=(warehouse._map.getNumberOfColumns()-1))
            {
                counter=j+1;
            }
            else if(!forward && j!=(warehouse._map.getNumberOfColumns()-1))
            {
                counter = warehouse._map.getNumberOfColumns()-1-j;
            }
        }
    }
    std::cout << "Goal reached, distance error: " << rob1->distanceToPoint(goal) << std::endl;
    //wait for the user to press any key:
    simulationThread.join();
    waitKey(0);
    return 0;
}