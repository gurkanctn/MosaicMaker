#pragma once
std::string getFileNameExtension(std::string s) {
	size_t i = s.rfind('.', s.length());
	std::string ext = "";
	if (i != std::string::npos) {
		ext = s.substr(i + 1, s.length() - i);
	}
	return ext;
}

int nPhotos = 0;
int nPhotoCounter = 0;
int nProblems = 0;
std::vector<std::string> sFileNames;

int XGridSize = 20;
int YGridSize = 16;
int XGridCount = 5;
int YGridCount = 5;

int brightnessLookup[255];

float TargetBrightness[100][100];		//100x100 should be enough for everyone (kidding)
float fTargetPos = 900.0f;				//this should be calculated from the Target image

bool bShowTarget = false;

void GenerateFileList() {
	std::filesystem::path path = std::filesystem::current_path() / "photos";
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