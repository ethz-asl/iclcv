#ifdef WIN32
#ifdef WITH_SONYIIDC

#include "iclSonyFwGrabber.h"

namespace icl {

	SonyFwGrabber::SonyFwGrabber(void) {
		//init();
	}

	bool SonyFwGrabber::init() {
		char* cameraID[10];

		IIDC_FORMATINFO *formatinfo;
		IIDC_FRAMERATEINFO *rateinfo;
		IIDC_FEATUREINFO *featureInfo;
		IIDC_FEATURE feature;

		int nRetCode = 0;
		int i,j;
		long lRet, lFormatCnt;
		long **ppFormatIndexList;
		bool bRet;

		//---- Establish camera connection ----
		bRet = iidc_init();

		if (!bRet)
		{
			cout << "FATAL error during init!" << endl;
			return false;
		}
		else
		{
			//---- Check for number of available cameras ----
			m_lNumCameras = iidc_getcameracount();
			cout << "Number of cameras found: " << m_lNumCameras << endl;
			ppFormatIndexList = new long*[m_lNumCameras];
			formatinfo = new IIDC_FORMATINFO[m_lNumCameras];
			rateinfo = new IIDC_FRAMERATEINFO[m_lNumCameras];
			featureInfo = new IIDC_FEATUREINFO[m_lNumCameras];

			//---- Setup ----
			for (i=0;i<m_lNumCameras;i++)
			{
				//---- Read/ build unique camera id ----
				cameraID[i] = new char[100];
				GetCamAllString(i, cameraID[i]);
				//cout << "Unique ID for camera " << i << ": " << cameraID[i] << endl;

				//---- Open camera ----
				m_hCamera[i] = iidc_open(i);
				if (!m_hCamera[i] )
					cout << "Get camera handle of camera " << i << " FAILED!" << endl;

				//---- How many formats are supported ----
				lFormatCnt = iidc_getformatcount( m_hCamera[i] );
				cout << "Number of supported formats for camera " << i << ": " << lFormatCnt << endl;

				//---- Read the supported formats from camera ----
				ppFormatIndexList[i] = new long[lFormatCnt];
				lRet = iidc_getformatidlist( m_hCamera[i], ppFormatIndexList[i], sizeof(long)*lFormatCnt );

				if (!lRet)
				{
					printf("Cannot read the supported formats from camera\n");
					exit(1);
				}

				for(int k=0;k<lFormatCnt;k++)
					lRet = iidc_getformatinfo(m_hCamera[i],
					&formatinfo[i],
					sizeof(IIDC_FORMATINFO),
					ppFormatIndexList[i][k]);

				//---- SET FRAMERATE (has to be set non-zero, and before iidc_setformat is called!) ----
				iidc_setframerate(m_hCamera[i], 15.0);

				//---- Set image format ----
				bRet = iidc_setformat(m_hCamera[i], ppFormatIndexList[i][0]);
				
				long lCurFormat = iidc_currentformat(m_hCamera[i]);
				cout << "color coding: " << GET_COLORCODING(lCurFormat);
				//cout << iidc_setformat(m_hCamera[i], IIDCID_COLORCODING_RGB8) << endl;

				switch(GET_COLORCODING(lCurFormat)) {
					case IIDCID_COLORCODING_MONO8:
						cout << "  MONO8" << endl;
						break;
					case IIDCID_COLORCODING_YUV411:
						cout << "  YUV411"<< endl;
						break;
					case IIDCID_COLORCODING_YUV422:
						cout << "  YUV422"<< endl;
						break;
					case IIDCID_COLORCODING_YUV444:
						cout << "  YUV444"<< endl;
						break;
					case IIDCID_COLORCODING_RGB8:
						cout << "  RGB8"<< endl;
						break;
					case IIDCID_COLORCODING_MONO16:
						cout << "  MONO16"<< endl;
						break;
					case IIDCID_COLORCODING_RGB16:
						cout << "  RGB8"<< endl;
						break;
					case IIDCID_COLORCODING_RAW8:
						cout << "  RAW8"<< endl;
						break;
					case IIDCID_COLORCODING_RAW16:
						cout << "  RAW16"<< endl;
						break;
				}

				if (!bRet)
				{
					cout << "FATAL error - could not set FORMAT" << endl;
					return false;
				}

				//---- Get image width/ height  ----
				cout << "Image width : " << formatinfo[i].width << endl;
				cout << "Image height: " << formatinfo[i].height << endl;

				m_iWidth = (int) formatinfo[i].width;
				m_iHeight = (int) formatinfo[i].height;

				//---- Get/ Set framerate info ----
				lRet = iidc_getframerateinfo(m_hCamera[i],
					&rateinfo[i],
					sizeof(IIDC_FRAMERATEINFO),
					ppFormatIndexList[i][0] );

				//---- Get/ Set feature info ----
				lRet = iidc_currentfeatureinfo(m_hCamera[i], &featureInfo[i], sizeof(IIDC_FEATUREINFO) );
			}

			m_pppImgBuffer = (BYTE***) new BYTE**[m_lNumCameras];

			for (i=0;i<m_lNumCameras;i++)
			{
				//if (iidc_getstatus(m_hCamera[i]) == IIDC_STATUS_UNSTABLE)
				//	cout << "status unstable, call preparecapture" << endl;
				//---- Prepare capture (IIDC_STATUS_UNSTABLE) ----
				bRet = iidc_preparecapture( m_hCamera[i]);

				if( bRet == FALSE ) 
				{
					cout << "FATAL error - Prepare capture failed" << endl;
					cout << "error code: ";
					printf("%x\r\n", iidc_getlasterror(iidc_handletoindex(m_hCamera[i])));
					Sleep(5000);
					exit(1);
				}

				//---- Attach image buffer (IIDC_STATUS_STABLE) ----
				long lBufSz;
				lBufSz = iidc_currentdataframebytes( m_hCamera[i] );

				m_pppImgBuffer[i] = new BYTE*[1];
				m_pppImgBuffer[i][0] = new BYTE[lBufSz];
				lRet = iidc_attachbuffer( m_hCamera[i], (void**)m_pppImgBuffer[i], sizeof(LPBYTE) );

				//---- Start sequential grab ----
				bRet = iidc_capture( m_hCamera[i], TRUE);

				cout << "Set feature values for CAMERA " << i << endl;
				cout << "--------------------------------" << endl;

				//---- Gain feature ----
				memset( (void*)&feature, 0, sizeof(IIDC_FEATURE) );
				feature.feature_index = IIDCID_FEATURE_GAIN;
				iidc_currentfeature( m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				feature.value = (int)SONY_GAIN;
				iidc_setfeature(m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				cout << "GAIN feature value: " << (int)feature.value << endl;

				//---- Shutter ----
				memset( (void*)&feature, 0, sizeof(IIDC_FEATURE) );
				feature.feature_index = IIDCID_FEATURE_SHUTTER;
				feature.flags = iidcfeature_value;
				iidc_currentfeature( m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				feature.value = (float)SONY_SHUTTER;
				iidc_setfeature(m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				cout << "Shutter       : " << (float)feature.value << endl;

				//---- Get color feature overview ----
				//---- White balance U----
				memset( (void*)&feature, 0, sizeof(IIDC_FEATURE) );
				feature.feature_index = IIDCID_FEATURE_WHITE_BALANCE;
				iidc_currentfeature( m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				feature.value2 = (int)SONY_WHITEBALANCE_U;
				iidc_setfeature(m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				cout << "White Balance U: " << (int)feature.value2 << endl;

				//---- White balance V----
				memset( (void*)&feature, 0, sizeof(IIDC_FEATURE) );
				feature.feature_index = IIDCID_FEATURE_WHITE_BALANCE;
				feature.flags = iidcfeature_value;
				iidc_currentfeature( m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				feature.value = (int)SONY_WHITEBALANCE_V;
				iidc_setfeature(m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				cout << "White Balance V: " << (int)feature.value << endl;

				//---- HUE ----
				memset( (void*)&feature, 0, sizeof(IIDC_FEATURE) );
				feature.feature_index = IIDCID_FEATURE_HUE;
				feature.flags = iidcfeature_value;
				iidc_currentfeature( m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				feature.value = (int)SONY_HUE;
				iidc_setfeature(m_hCamera[i], &feature, sizeof(IIDC_FEATURE) );
				cout << "Hue            : " << (int)feature.value << endl;
				cout << endl;
			}  
		}

		return true;

	}

	SonyFwGrabber::~SonyFwGrabber(void) {
		//---- Destroy all camera handle ----
		iidc_uninit();
	}

	std::string SonyFwGrabber::getValue(const std::string &name) {
		//TODO return values for all cameras or get num of camera as argument!
		IIDC_FEATURE feature;
		if(name == "size"){
			return translateSize(Size(m_iWidth, m_iHeight));
		}else if(name == "gain"){
			memset( (void*)&feature, 0, sizeof(IIDC_FEATURE) );
			feature.feature_index = IIDCID_FEATURE_GAIN;
			iidc_currentfeature( m_hCamera[0], &feature, sizeof(IIDC_FEATURE) );
			feature.value = (int)SONY_GAIN;
			char buf[20];
			sprintf(buf,"%d",int(feature.value));
			return buf;
		}else if(name == "white balance red" || name == "white balance blue" || name == "white balance mode"){
			if(name == "white balance red"){ 
			}else if(name == "white balance blue"){}
			return "TODO";
		}else if(name == "format"){
			return "TODO";
		}else if(name == "shutter speed"){
			memset( (void*)&feature, 0, sizeof(IIDC_FEATURE) );
			feature.feature_index = IIDCID_FEATURE_SHUTTER;
			feature.flags = iidcfeature_value;
			iidc_currentfeature( m_hCamera[0], &feature, sizeof(IIDC_FEATURE) );
			feature.value = (float)SONY_SHUTTER;
			char buf[20];
			sprintf(buf,"%d",int(feature.value));
			return buf;
		}
		return "undefined";
	}

	void SonyFwGrabber::setProperty(const std::string &property, const std::string &value) {
		if(property == "size"){
			Size newSize = translateSize(value);
			//setGrabbingSize(newSize);
		}else if(property == "format"){
			/*if(value != "YUV 4-2-0 planar"){
				ERROR_LOG("invalid format \"" << value <<"\"");
			}*/
		}else if(property == "gain"){
			int val = atoi(value.c_str());
			//setGain(clip(val,0,65535));
		}else if(property == "save user settings"){
			//saveUserSettings(); // value is ignored
		}else if(property == "restore user settings"){
			//restoreUserSettings();
		}else if(property == "white balance mode"){
			/*int mode = -1; // unknown default
			if      (value == "indoor") mode = 0;
			else if (value == "outdoor") mode = 1;
			else if (value == "fl-tube") mode = 2;
			else if (value == "auto") mode = 4;
			if (mode != -1) {
				setWhiteBalance(mode);
			}else{
				ERROR_LOG("unknown white balance mode \"" << value << "\"");
			}*/
		}else if(property == "white balance"){
			/*vector<double> vec = Grabber::translateDoubleVec (value);
			if (vec.size() != 2) {
				ERROR_LOG("two white balance values required (red + blue)");
			} else {
				setWhiteBalance((int)vec[0], (int)vec[1]);
			}*/
		}else if(property == "white balance red"){
			int val = atoi(value.c_str());
			//setWhiteBalance(val,-1);
		}else if(property == "white balance blue"){
			int val = atoi(value.c_str());
			//setWhiteBalance(-1,val);
		}else if(property == "shutter speed"){
			int val = atoi(value.c_str());
			//setShutterSpeed(val);
		}else{
			ERROR_LOG("nothing known about a property " << property ); 
		}
	}

	std::vector<std::string> SonyFwGrabber::getPropertyList() {
		std::vector<std::string> v;
		v.push_back("size");
		v.push_back("format");
		v.push_back("gain");
		v.push_back("save user settings");
		v.push_back("restore user settings");
		v.push_back("white balance mode");
		v.push_back("white balance red");
		v.push_back("white balance blue");
		v.push_back("shutter speed");
		return v;
	}

	const ImgBase* SonyFwGrabber::grab(ImgBase **poDst) {
		ImgBase* img = new Img8u(Size(m_iWidth, m_iHeight), icl::formatGray);

		//---- Initialize variables ----
		IIDC_WAIT waitparam;
		bool bRetLeft, bRetRight;
		void *vImgBufLeft, *vImgBufRight;

		printf("get image(s) ... ");

		waitparam.event = IIDCID_EVENT_FRAMEEND;
		waitparam.timeout = 1000;

		//---- Get image from buffer ----
		bRetLeft = iidc_wait( m_hCamera[0], &waitparam, sizeof(IIDC_WAIT) );
		bRetRight = iidc_wait( m_hCamera[1], &waitparam, sizeof(IIDC_WAIT) );

		if( bRetLeft && bRetRight )
		{
			vImgBufLeft = (LPBYTE) iidc_lockdata( m_hCamera[0], -1 );
			vImgBufRight = (LPBYTE) iidc_lockdata( m_hCamera[1], -1 );
			if(vImgBufLeft && vImgBufRight)
			{
				memcpy((icl8u*)(img->getDataPtr(0)),m_pppImgBuffer[0][0],(m_iWidth*m_iHeight)*sizeof(unsigned char));
				//memcpy((icl8u*)(img->getDataPtr(1)),m_pppImgBuffer[1][0],(m_iWidth*m_iHeight)*sizeof(unsigned char));

				//---- remove data lock ----
				iidc_unlockdata( m_hCamera[0] );
				iidc_unlockdata( m_hCamera[1] );
			}
		}

		printf("done\n");
		return img;
	}

	void SonyFwGrabber::GetStereoImage(ImgBase **poDstLeft, ImgBase **poDstRight) {
		
		//---- Initialize variables ----
		IIDC_WAIT waitparam;
		bool bRetLeft, bRetRight;
		void *vImgBufLeft, *vImgBufRight;

		printf("get images ... ");

		waitparam.event = IIDCID_EVENT_FRAMEEND;
		waitparam.timeout = 1000;

		//---- Get image from buffer ----
		bRetLeft = iidc_wait( m_hCamera[0], &waitparam, sizeof(IIDC_WAIT) );
		bRetRight = iidc_wait( m_hCamera[1], &waitparam, sizeof(IIDC_WAIT) );

		if( bRetLeft && bRetRight )
		{
			vImgBufLeft = (LPBYTE) iidc_lockdata( m_hCamera[0], -1 );
			vImgBufRight = (LPBYTE) iidc_lockdata( m_hCamera[1], -1 );
			if(vImgBufLeft && vImgBufRight)
			{
				memcpy((icl8u*)((*poDstLeft)->getDataPtr(0)),m_pppImgBuffer[0][0],(m_iWidth*m_iHeight)*sizeof(unsigned char));
				memcpy((icl8u*)((*poDstRight)->getDataPtr(0)),m_pppImgBuffer[1][0],(m_iWidth*m_iHeight)*sizeof(unsigned char));

				//---- remove data lock ----
				iidc_unlockdata( m_hCamera[0] );
				iidc_unlockdata( m_hCamera[1] );
			}
		}

		printf("done\n");

		//flipping image
		ImgBase* tmp = new Img8u(Size(m_iWidth, m_iHeight), icl::formatGray);
		flippedCopy(axisBoth, *poDstLeft, &tmp);
		tmp->deepCopy(poDstLeft);
		flippedCopy(axisBoth, *poDstRight, &tmp);
		tmp->deepCopy(poDstRight);

	}

	void SonyFwGrabber::GetCamAllString(long camIndex, char *strCamera) {
		char* pBuf1, *pBuf2, *pBuf3;
		long	lRet = iidc_getstring( camIndex, IIDCID_STRING_VENDOR, NULL, 0 );
		pBuf1 = new char[lRet+1];
		memset( pBuf1, NULL, lRet+1);
		lRet = iidc_getstring( camIndex, IIDCID_STRING_VENDOR, pBuf1, lRet );
		if( !lRet )
		{
			delete [] pBuf1;
			return;
		}

		lRet = iidc_getstring( camIndex, IIDCID_STRING_MODEL, NULL, 0 );
		pBuf2 = new char[lRet+1];
		memset( pBuf2, NULL, lRet+1);
		lRet = iidc_getstring( camIndex, IIDCID_STRING_MODEL, pBuf2, lRet );
		if( !lRet )
		{
			delete [] pBuf2;
			return;
		}

		lRet = iidc_getstring( camIndex, IIDCID_STRING_CAMERAID, NULL, 0 );
		pBuf3 = new char[lRet+1];
		memset( pBuf3, NULL, lRet+1);
		lRet = iidc_getstring( camIndex, IIDCID_STRING_CAMERAID, pBuf3, lRet );
		if( !lRet )
		{
			delete [] pBuf3;
			return;
		}

		sprintf(strCamera,"%s %s %s",pBuf1,pBuf2,pBuf3);

		delete [] pBuf1;
		delete [] pBuf2;
		delete [] pBuf3;
	}

}

#endif //SONYIIDC
#endif //WIN32
