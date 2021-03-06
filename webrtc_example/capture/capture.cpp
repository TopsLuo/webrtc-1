// capture.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "VideoCaptureCallback.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/video_capture/video_capture_defines.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "rtc_base/logging.h"

void PrintCapability(webrtc::VideoCaptureCapability& capability);

//using namespace webrtc;
int main(int  argc, char *argv[])
{
	// 摄像头操作
	webrtc::VideoCaptureModule::DeviceInfo* pDev = webrtc::VideoCaptureFactory::CreateDeviceInfo();
	if (nullptr == pDev) {
		RTC_LOG(INFO) << "CreateDeviceInfo Failed";
		return -1;
	}
	RTC_LOG(INFO) << "NumberOfDevices : " << pDev->NumberOfDevices();

	uint32_t size = 256;
	char szDeviceName[256] = { '\0' };
	char szDeviceUniqueID[256*4] = { '\0' };
	char szProductUniqueID[256] = { '\0' };
	pDev->GetDeviceName(0, szDeviceName, size,
		szDeviceUniqueID, size*4, szProductUniqueID, size);
	RTC_LOG(INFO) << "szDeviceName : " << szDeviceName;
	RTC_LOG(INFO) << "szDeviceUniqueID : " << szDeviceUniqueID;
	RTC_LOG(INFO) << "szProductUniqueID : " << szProductUniqueID;


	// 获取摄像头的能力集数量
	uint32_t num = pDev->NumberOfCapabilities(szDeviceUniqueID);
	// 输出webrtc::VideoCaptureCapability信息
	for (int i = 0; i < num; i++) {
		webrtc::VideoCaptureCapability capability;
		uint32_t ret = pDev->GetCapability(szDeviceUniqueID, i, capability);
		if (ret != 0) {
			RTC_LOG(LS_ERROR) << "GetCapability ret : " << ret;
			return -1;
		}
		PrintCapability(capability);
	}



	// 通过指定摄像头的DeviceUniqueID创建VideoCaptureModule对象
	rtc::scoped_refptr<webrtc::VideoCaptureModule> pVCM = webrtc::VideoCaptureFactory::Create(szDeviceUniqueID);
	if (pVCM.get() == nullptr) {
		RTC_LOG(LS_ERROR) << "szDeviceUniqueID : " << szDeviceUniqueID;
		return -1;
	}


	//  第一种Capability采集视频
	webrtc::VideoCaptureCapability capability;
	uint32_t ret = pDev->GetCapability(szDeviceUniqueID, 0, capability);

	std::shared_ptr<VideoCaptureCallback> callback(new VideoCaptureCallback());
	callback->Init(capability.width, capability.height);


	// 音频设备操作
	rtc::scoped_refptr<webrtc::AudioDeviceModule> pADM = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kPlatformDefaultAudio);
	if (pADM.get() == nullptr) {
		RTC_LOG(LS_ERROR) << "AudioDeviceModule::Create Failed : ";
		return -1;
	}

	pADM->Init();  // 注：需要调用否则都是报错
	pADM->RegisterAudioCallback(callback.get());

	num = pADM->RecordingDevices();
	RTC_LOG(INFO) << "RecordingDevices : " << num;
	if (num > 0) {
		char name[webrtc::kAdmMaxDeviceNameSize] = { '\0' };
		char guid[webrtc::kAdmMaxGuidSize] = { '\0' };
		int32_t ret = pADM->RecordingDeviceName(0, name, guid);
		RTC_LOG(INFO) << "RecordingDevices name : " << name << " guid : " << guid << " ret : " << ret;
	}

	pADM->SetPlayoutDevice(0);
	pADM->SetRecordingDevice(0);
	pADM->StartPlayout();
	pADM->StartRecording();

	pVCM->RegisterCaptureDataCallback(callback.get());
	ret = pVCM->StartCapture(capability);
	if (ret != 0) {
		RTC_LOG(LS_ERROR) << "StartCapture failed ";
		PrintCapability(capability);
	}
	system("pause");
	pVCM->StopCapture();
	pVCM->DeRegisterCaptureDataCallback();
    return 0;
}


void PrintCapability(webrtc::VideoCaptureCapability& capability) {
	RTC_LOG(INFO) << "VideoCaptureCapability width : " << capability.width
		<< " height : " << capability.height << " maxFPS : " << capability.maxFPS
		<<" videoType : " << static_cast<int>(capability.videoType);
}

