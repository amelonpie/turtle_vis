/*********************************************************************
* Compiler:         gcc 4.6.3
*
* Company:          Institute for Cognitive Systems
*                   Technical University of Munich
*
* Author:           Emmanuel Dean (dean@tum.de)
*                   Karinne Ramirez (karinne.ramirez@tum.de)
*
* Compatibility:    Ubuntu 12.04 64bit (ros hydro)
*
* Software Version: V0.1
*
* Created:          01.06.2015
*
* Comment:          turtle connection and visualization (Sensor and Signals)
*
********************************************************************/


/*********************************************************************
* STD INCLUDES
********************************************************************/
#include <iostream>
#include <fstream>
#include <pthread.h>


/*********************************************************************
* ROS INCLUDES
********************************************************************/
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <ros/console.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include <tf_conversions/tf_eigen.h>

/*********************************************************************
 * CUSTOM CLASS
 * ******************************************************************/
#include <turtle_vis/myClass/TurtleClass.h>


int main( int argc, char** argv )
{

    ros::init(argc, argv, "turtle_control",ros::init_options::AnonymousName);

    ROS_INFO_STREAM("**Publishing turtle control..");

    ros::NodeHandle n;
    ros::Rate r(60);



    //ADVERTISE THE SERVICE
    turtleSpace::TurtleClass turtleF;
    ros::ServiceServer service=n.advertiseService("TurtlePose",
                                                  &turtleSpace::TurtleClass::getDPose,
                                                  &turtleF);
    //CALL SERVICE FROM TERMINAL//
    //    rosservice call /TurtlePose '{p: [0.5, 0.0, 3.0]}'
    // rosservice call /TurtlePose "[ 0.0, 0.0, 0.0]"

    //    rosservice call /TurtlePose "{p: {x: 1.5, y: 1.0, theta: 0.0}}"
    //DON'T FORGET TO SOURCE THE WORKSPACE BEFORE CALLING THE SERVICE

    //ADVERTIZE THE TOPIC
    ros::Publisher pub=n.advertise<turtle_vis::DesiredPose>("turtle_control",100);

    ros::Time ti, tf;
    ti=ros::Time::now();

    //Proportional Gain
    Matrix3d Kp;

    double p_g=0.0;

    ros::param::get("/turtle_gains/p_gain",p_g); //TODO: p_gain -> also include ; also solves the above todo?

    //Proportional Gain
    Kp<<p_g,0  ,0,
            0  ,p_g,0,
            0  ,0  ,p_g;

    ROS_INFO_STREAM("Kp= \n"<<Kp);

    Vector3d turtlePose,turtlePose_old,turtleVel;
    Vector3d error;
    double dt;

    //INITIALIZE THE TURTLE POSE
    turtlePose<<1,0,0;
    turtlePose_old=turtlePose;
    turtleVel<<0,0,0;

    //DESIRED POSE
    Vector3d turtlePose_desired_local;
    ////#>>>>TODO: INITIALIZE THE DESIRED POSE VARIABLE OF THE CLASS TURTLE
    turtleF.SetLocalDesiredPose(turtlePose); //TODO: which class variable? there is none? create one + setter?
    turtlePose_desired_local=turtlePose;


    //CREATE A DESIREDPOSE MSG VARIABLE
    turtle_vis::DesiredPose msg; //#>>>>TODO:DEFINE THE MSG TYPE

    while(ros::ok())
    {
        tf=ros::Time::now();

        dt=tf.toSec()-ti.toSec();

        ////#>>>>TODO: Get Desired Pose from the class variable
        turtlePose_desired_local=turtleF.getLocalDesiredPose(); //TODO: correct one selected?

        //CONTROL
        ////#>>>>TODO:COMPUTE THE ERROR BETWEEN CURRENT POSE AND DESIRED
        error=turtlePose_desired_local - turtlePose_old; //TODO: why error = pose_old??
        // COMPUTE THE INCREMENTS
        turtleVel=error/dt;

        turtlePose+=-Kp*turtleVel;

        //Publish Data
        ////#>>>>TODO:SET THE MSG VARIABLE WITH THE NEW TURTLE POSE
        msg.y=turtlePose[1];
        msg.theta=turtlePose[2];
        msg.x=turtlePose[0];

        pub.publish(msg);

        //SET THE HISTORY
        turtlePose_old=turtlePose;
        ti=tf;

        //ROS::SPIN IS IMPORTANT TO UPDATE ALL THE SERVICES AND TOPICS
        ros::spinOnce();

        r.sleep();
    }

    return 0;
}


