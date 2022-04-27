#pragma once
#include <filesystem>

std::string getFileNameExtension(std::string s) {
	size_t i = s.rfind('.', s.length());
	std::string ext = "";
	if (i != std::string::npos) {
		ext = s.substr(i + 1, s.length() - i);
	}
	return ext;
}

std::string targetFileName;

typedef struct {
	unsigned char r, g, b;
} RGB;

double ColourDistance(RGB e1, RGB e2)
{
	long rmean = ((long)e1.r + (long)e2.r) / 2;
	long r = (long)e1.r - (long)e2.r;
	long g = (long)e1.g - (long)e2.g;
	long b = (long)e1.b - (long)e2.b;
	return sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));
}


int nPhotos = 0;
int nFilesToProcess = 0;
int nPhotoCounter = 0;
int nProblems = 0;
std::vector<std::string> sFileNames;

int XGridSize = 40;
int YGridSize = 32;
int XGridCount = 5;		//will be recalculated depending on Target photo size
int YGridCount = 5;		//will be recalculated depending on Target photo size

int brightnessLookup[255];
int LookupRGB[32][32][32];


float TargetBrightness[400][400];		//400x400 should be enough for everyone (kidding)
RGB TargetRGB[400][400];		//400x400 should be enough for everyone (kidding)

float fGlobalTime = 0.0f;
bool bShowTarget = false;
bool bTargetAlreadyDisplayed = false;
bool bTargetMosaicGenerated = false;
bool bWrittenToFile = false;

void GenerateFileList() {
	std::filesystem::path path = std::filesystem::current_path() / "photos";
	//check if folder "photos" exists
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		//check if extension is PNG or JPG and whether filename is valid
		try {
			std::cout << entry.path() << std::endl;
			std::string s = entry.path().string();
			std::string ext = getFileNameExtension(s);
			std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
			if (ext == "PNG" || ext == "JPG" || ext == "JPEG") {
				nPhotos++;
				sFileNames.push_back(s);
			};
		}
		catch (...) {
			std::cout << "ERROR IN FILENAME. DISCARDING." << std::endl;
			nProblems++;
		}
	}
	std::cout << nPhotos << " photos found!..." << std::endl;
	if (nProblems > 0) std::cout << nProblems << " files ignored..." << std::endl;

}

float progress = 0.0f;

void displayProgressBar(float progress) {
	//display Progress bar
	int barWidth = 70;
	std::cout << "[";
	int pos = barWidth * progress;
	for (int j = 0; j < barWidth; ++j) {
		if (j < pos) std::cout << "=";
		else if (j == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0) << "%\r";
	std::cout.flush();
}