#define DEBUG 1
#define OVR_BUILD_DEBUG

#include "OVR.h"

int main(int argc, char ** argv) {

    using namespace OVR;

    OVR::System::Init();

    Ptr<DeviceManager> pManager = 0;
    Ptr<HMDDevice>     pHMD = 0;
    Ptr<SensorDevice>  pSensor = 0;
    SensorFusion       FusionResult;


    // *** Initialization - Create the first available HMD Device
    pManager = *DeviceManager::Create();
    pHMD     = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();
    if (!pHMD)
        return 0;
    pSensor  = *pHMD->GetSensor();

    // Get DisplayDeviceName, ScreenWidth/Height, etc..
    HMDInfo hmdInfo;
    pHMD->GetDeviceInfo(&hmdInfo);

    if (pSensor)
        FusionResult.AttachToSensor(pSensor);

    // *** Per Frame
    // Get orientation quaternion to control view
    Quatf q = FusionResult.GetOrientation();

    // Create a matrix from quaternion,
    // where elements [0][0] through [3][3] contain rotation.
    Matrix4f bodyFrameMatrix(q);

    // Get Euler angles from quaternion, in specified axis rotation order.
    float yaw, pitch, roll;
    q.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);

    // *** Shutdown
    pSensor.Clear();
    pHMD.Clear();
    pManager.Clear();

    OVR::System::Destroy();
    return 0;
}
