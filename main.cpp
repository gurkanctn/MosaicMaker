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
	MosaicMaker() {		sAppName = "Mosaic Maker v0.1a - Gurkan Cetin, 2022";	}

public:
	
	struct PhotoData {
		int index;
		float brightness;
		olc::Sprite* thumbnail;
		bool operator<(const PhotoData& a) const {
			return brightness < a.brightness;
		}
	};
	std::vector<PhotoData> PD;

	olc::Sprite* sprDummy;
	olc::Sprite* sprTemporary;
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
		printf("Mosaic Maker v0.2a. G.Çetin.\n");

		GenerateFileList();
		
		//SET and LOAD TARGET PHOTO
		std::string targetFileName;
		std::cout << "ENTER TARGET PHOTO FILENAME. Make sure it is under 'photos' folder." << std::endl;
		std::cin >> targetFileName;
		targetFileName = "photos/" + targetFileName;
		std::cout << targetFileName << std::endl;

		try {
				sprTarget = new olc::Sprite(targetFileName);
		}
		catch (...) {
			std::cout << "Couldn't open your file, Correct it or die!" << std::endl;
			return false;
		}

		XGridCount = sprTarget->width / XGridSize;
		YGridCount = sprTarget->height / YGridSize;

		int nFilesToProcess = sFileNames.size();
		std::cout << "Starting to load all photos, this may take a while. Please standby..." << std::endl;

		for (int i = 0; i < nFilesToProcess; i++) {
			sprTemporary = new olc::Sprite(sFileNames[i]);
			int scale = sprTemporary->width / XGridSize;
			sprDummy = new olc::Sprite(XGridSize, YGridSize);
			SetDrawTarget(sprDummy);
			DrawFrame(sprTemporary, 0, 0, scale);
			sprPhotos.push_back(sprDummy);
			delete sprTemporary;
		}
		std::cout << "Finished loading all files. Let's Go!" << std::endl;

		for (int i = 0; i < nFilesToProcess; i++) {
			PhotoData tempPD;
			tempPD.index = i;
			tempPD.brightness = calcFrameBrightness(sprPhotos[i]);
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

		//TARGET PHOTO
		for (int m = 0; m < XGridCount; m++) {
			for (int n = 0; n < YGridCount; n++) {
				sprDummy = new olc::Sprite(XGridSize, YGridSize);
				SetDrawTarget(sprDummy);
				DrawPartialSprite(0, 0, sprTarget, m * XGridSize, n * YGridSize, XGridSize, YGridSize, 1, 0);
				TargetBrightness[m][n] = calcFrameBrightness(sprDummy);
			}
		}

		std::cout << "Loading Complete!" << std::endl;
		std::cout << sprPhotos.size() << " photos in Memory!" << std::endl;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		SetDrawTarget(nullptr);
		Clear(olc::BLACK);
		SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels
		if (GetKey(olc::Key::ESCAPE).bReleased) return false;

		for (int m = 0; m < XGridCount; m++) {
			for (int n = 0; n < YGridCount; n++) {
				DrawSprite(m * XGridSize, n * YGridSize, sprPhotos[brightnessLookup[int(TargetBrightness[m][n])]]);
			}
		}
		
		if (GetKey(olc::Key::TAB).bReleased) bShowTarget = !bShowTarget;
		if (bShowTarget) DrawSprite(0, 0, sprTarget);
		
		if (!bWrittenToFile) {
			WriteResultToFile();
			bWrittenToFile = true;
		}
		
		SetDrawTarget(nullptr);

		return true;
	}
};

int main()
{
	MosaicMaker mosaicmaker;

	if (mosaicmaker.Construct(1800, 1000, 1, 1)) {
		mosaicmaker.Start();
	}
	return 0;
}