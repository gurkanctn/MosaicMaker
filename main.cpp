#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <filesystem>
#include <iostream>
#include <string>

#include "MosaicUtils.h"

class MosaicMaker : public olc::PixelGameEngine
{
public:
	MosaicMaker() {		sAppName = "Mosaic Maker v0.3a - Gurkan Cetin, 2022";	}

public:
	
	struct PhotoData {
		int index;
		float brightness;
		RGB rgb;
		olc::Sprite* thumbnail;
		bool operator<(const PhotoData& a) const {
			return brightness < a.brightness;
		}
	};
	std::vector<PhotoData> PD;

	olc::Sprite* sprDummy;
	olc::Sprite* sprTemporary;
	olc::Sprite* sprTargetOriginal;
	olc::Sprite* sprTarget;
	std::vector<olc::Sprite*> sprPhotos;

	void DrawFrame(olc::Sprite* f, int x, int y, int scale){
		int nFrameWidth = f->width / scale;
		int nFrameHeight = f->height / scale;
		for (int i = 0; i < nFrameWidth; i++)
			for (int j = 0; j < nFrameHeight; j++)
			{
				olc::Pixel p = f->GetPixel(i * scale, j * scale);
				Draw(x + i, y + j, p);
			}
	}

	float calcFrameBrightness(olc::Sprite* spr){
		int nFrameWidth = spr->width;
		int nFrameHeight = spr->height;
		float brightnessSum = 0.0f;

		for (int i = 0; i < nFrameWidth; i++)
			for (int j = 0; j < nFrameHeight; j++)
			{
				olc::Pixel p = spr->GetPixel(j, i);
				brightnessSum += (p.r + p.g + p.b) / 3.0f;
			}
		return brightnessSum / (nFrameHeight * nFrameWidth);
	}

	RGB calcFrameRGB(olc::Sprite* spr) {
		int nFrameWidth = spr->width;
		int nFrameHeight = spr->height;
		RGB FrameRGB = {0.0, 0.0, 0.0};
		long tempR = 0;
		long tempG = 0;
		long tempB = 0;
		for (int i = 0; i < nFrameWidth; i++){
			for (int j = 0; j < nFrameHeight; j++)
			{
				olc::Pixel p = spr->GetPixel(j, i);
				tempR += p.r;
				tempG += p.g;
				tempB += p.b;
			}
		}
		tempR /= (nFrameHeight * nFrameWidth);
		tempG /= (nFrameHeight * nFrameWidth);
		tempB /= (nFrameHeight * nFrameWidth);
		FrameRGB.r = (unsigned char)tempR;
		FrameRGB.g = (unsigned char)tempG;
		FrameRGB.b = (unsigned char)tempB;
		return FrameRGB;
	}


	void WriteResultToFile() {
		olc::Sprite* pScreenPtr = GetDrawTarget();
		try {
			stbi_write_png("output.png", pScreenPtr->width, pScreenPtr->height, 4, pScreenPtr->GetData(), pScreenPtr->width * 4);
		}
		catch (...) {
			std::cout << "ERROR WRITING OUTPUT, TRY SCREEN CAPTURING!" << std::endl;
		}
	}

	bool OnUserCreate() override {

		try {
				sprTargetOriginal = new olc::Sprite(targetFileName);
				sprTarget = new olc::Sprite(sprTargetOriginal->width *2, sprTargetOriginal->height *2);
				//olc::GFX2D::Transform2D t;
				//t.Scale(2, 2);
				SetDrawTarget(sprTarget);
				DrawSprite(0, 0, sprTargetOriginal, 2, 0);

				delete sprTargetOriginal;
		}
		catch (...) {
			std::cout << "Couldn't open your file! Correct it or die!" << std::endl;
			return false;
		}

		XGridCount = sprTarget->width / XGridSize;
		YGridCount = sprTarget->height / YGridSize;
		
		nFilesToProcess = sFileNames.size();
		std::cout << "Starting to load all photos, this may take a while. Please standby..." << std::endl;
		
		for (int i = 0; i < nFilesToProcess; i++) {
			sprTemporary = new olc::Sprite(sFileNames[i]);
			int scale = sprTemporary->width / XGridSize;
			sprDummy = new olc::Sprite(XGridSize, YGridSize);
			SetDrawTarget(sprDummy);
			DrawFrame(sprTemporary, 0, 0, scale);
			sprPhotos.push_back(sprDummy);
			delete sprTemporary;
			displayProgressBar(progress);
			progress += static_cast<float>(1) / nFilesToProcess;
		}
		std::cout << std::endl;
		std::cout << "Finished loading all files. Let's Go!" << std::endl;
		return true;
	}
	void GenerateTargetMosaic() {
		for (int i = 0; i < nFilesToProcess; i++) {
			PhotoData tempPD;
			tempPD.index = i;
			tempPD.brightness = calcFrameBrightness(sprPhotos[i]);
			tempPD.rgb = calcFrameRGB(sprPhotos[i]);
			tempPD.thumbnail = sprPhotos[i];
			PD.push_back(tempPD);
		}

		float errBrightness = 256;

		//create Brightness Lookup Table
		for (int i = 0; i < 255; i++) {
			float errBrightness = 256;

			for (int j = 0; j < nFilesToProcess; j++) {
				if (std::abs(PD[j].brightness - i) < errBrightness)
				{
					brightnessLookup[i] = j;
					errBrightness = std::abs(PD[j].brightness - i);
				}
			}
		}

		//create Color Lookup Table (3D matrix)
		// fill up a cube of red,green,blue arrays with best matching photo (j of nFilesToProcess)
		RGB dummyRGB{0,0,0};
		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				for (int k = 0; k < 32; k++) {
					float errColor = 1000; //check

					for (int n = 0; n < nFilesToProcess; n++) {
						dummyRGB.r = i*8;
						dummyRGB.g = j*8;
						dummyRGB.b = k*8;
						double ColDistance = ColourDistance(PD[n].rgb, dummyRGB);
						if (ColDistance < errColor)
						{
							LookupRGB[i][j][k] = n;
							errColor = ColDistance;
						}
					}
				}
			}
		}


		//Generate TARGET PHOTO brightness map.
		if (false) { // for brightness
			for (int m = 0; m < XGridCount; m++) {
				for (int n = 0; n < YGridCount; n++) {
					sprDummy = new olc::Sprite(XGridSize, YGridSize);
					SetDrawTarget(sprDummy);
					DrawPartialSprite(0, 0, sprTarget, m * XGridSize, n * YGridSize, XGridSize, YGridSize, 1, 0);
					TargetBrightness[m][n] = calcFrameBrightness(sprDummy);
				}
			}
		}

		for (int m = 0; m < XGridCount; m++) {
			for (int n = 0; n < YGridCount; n++) {
				sprDummy = new olc::Sprite(XGridSize, YGridSize);
				SetDrawTarget(sprDummy);
				DrawPartialSprite(0, 0, sprTarget, m * XGridSize, n * YGridSize, XGridSize, YGridSize, 1, 0);
				TargetRGB[m][n] = calcFrameRGB(sprDummy);
			}
		}
		

		std::cout << "Loading Complete!" << std::endl;
		std::cout << sprPhotos.size() << " photos in Memory!" << std::endl;

	}

	bool OnUserUpdate(float fElapsedTime) override {
		fGlobalTime += fElapsedTime;

		if (GetKey(olc::Key::ESCAPE).bReleased) return false;
		
		if (!bTargetAlreadyDisplayed) {
			SetDrawTarget(nullptr);
			SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels
			DrawSprite(0, 0, sprTarget, 1, 0);
			bTargetAlreadyDisplayed = true;
			std::cout << "Target Photo displayed." << std::endl;
			return true;
		}

		if ((!bTargetMosaicGenerated) && (fGlobalTime>1.0f)) {
			GenerateTargetMosaic();
			bTargetMosaicGenerated = true;
			std::cout << "Target Mosaic integration complete." << std::endl;
			return true;
		}
			
		if (!bWrittenToFile) {
			SetDrawTarget(sprTarget);
			Clear(olc::BLACK);
			SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels
			for (int m = 0; m < XGridCount; m++) {
				for (int n = 0; n < YGridCount; n++) {
					//DrawSprite(m * XGridSize, n * YGridSize, sprPhotos[brightnessLookup[int(TargetBrightness[m][n])]]);
					unsigned char r = (TargetRGB[m][n].r)/8;
					unsigned char g = (TargetRGB[m][n].g)/8;
					unsigned char b = (TargetRGB[m][n].b)/8;
					int nFile = LookupRGB[r][g][b];
//					std::cout << "r: " << (int)r <<" g:"<< (int)g <<" b:" << (int)b << " nFile:" << nFile << std::endl;
					DrawSprite(m * XGridSize, n * YGridSize, sprPhotos[nFile]);
				}
			}
			WriteResultToFile();
			bWrittenToFile = true;
			std::cout << "Target Mosaic written to file." << std::endl;
			std::cout << "Press ESC to quit Mosaic Maker." << std::endl;
			return true;
		}

		if (GetKey(olc::Key::TAB).bReleased) bShowTarget = !bShowTarget;

		SetDrawTarget(nullptr);
		DrawSprite(0, 0, sprTarget);

		return true;
	}
};

int main()
{
	std::cout << "Mosaic Maker v0.3a. G.Çetin.\n";
	std::cout << "Will generate a Mosaic photo from 'Photos' folder.\n";
	try {
		GenerateFileList();
	}
	catch (...) {
		std::cout << "\x1B[41mERROR: Could not load 'Photos', make sure your photos are in 'Photos' folder!\n";
		return 1;
	}

	//SET and LOAD TARGET PHOTO

	std::cout << std::endl << "ENTER TARGET PHOTO FILENAME. Make sure it is under 'photos' folder." << std::endl;
	std::cin >> targetFileName;
	targetFileName = "photos/" + targetFileName;
	std::cout << targetFileName << std::endl;

	MosaicMaker mosaicmaker;

	if (mosaicmaker.Construct(1800, 1000, 1, 1)) {
		mosaicmaker.Start();
	}
	return 0;
}