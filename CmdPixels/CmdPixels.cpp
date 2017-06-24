#include <iostream>
#include <windows.h>
#include <iomanip>

using namespace std;

#define DEFAULT_TEXT_COLOR		8

#define FG_RGBA16(r, g, b, a)	(((a) << 3) | ((r) << 2) | ((g) << 1) | (b))
#define FG_RGBF(r, g, b)		FG_RGBA16((r) > 0.3333f ? 1 : 0, (g) > 0.3333f ? 1 : 0, (b) > 0.3333f ? 1 : 0, max(max(r, g), b) > 0.666f ? 1 : 0)

#define BG_RGBA16(r, g, b, a)	(((a) << 7) | ((r) << 6) | ((g) << 5) | ((b) << 4))
#define BG_RGBF(r, g, b)		BG_RGBA16((r) > 0.3333f ? 1 : 0, (g) > 0.3333f ? 1 : 0, (b) > 0.3333f ? 1 : 0, max(max(r, g), b) > 0.6666f ? 1 : 0)

struct setcolor
{
	uint16_t m_color;
	static HANDLE m_console_handle;

	setcolor(uint16_t c) :
		m_color(c) {}
};

HANDLE setcolor::m_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

basic_ostream<char> &operator<<(basic_ostream<char> &s, const setcolor &ref)
{
	SetConsoleTextAttribute(ref.m_console_handle, ref.m_color);
	
	return s;
}

void setNextPixel(float r, float g, float b)
{
	cout << setcolor(FG_RGBF(r, g, b) | BG_RGBF(r, g, b)) << "  ";
}

void setFontSize(int FontSize)
{
	CONSOLE_FONT_INFOEX info = { 0 };
	info.cbSize = sizeof(info);
	info.dwFontSize.Y = FontSize; // leave X as zero
	info.FontWeight = FW_NORMAL;
	wcscpy_s(info.FaceName, L"Lucida Console");
	
	SetCurrentConsoleFontEx(setcolor::m_console_handle, false, &info);
}

// Load a ppm file into memory
bool loadTexture(const char *filename, uint8_t *rgba_data, uint32_t &width, uint32_t &height, uint32_t &rowPitch)
{
	FILE *fPtr;
	fopen_s(&fPtr, filename, "rb");
	char header[256], *cPtr, *tmp;

	if (!fPtr)
		return false;

	cPtr = fgets(header, 256, fPtr); // P6
	if (cPtr == nullptr || strncmp(header, "P6\n", 3))
	{
		fclose(fPtr);
		return false;
	}

	do {
		cPtr = fgets(header, 256, fPtr);
		if (cPtr == nullptr)
		{
			fclose(fPtr);
			return false;
		}
	} while (!strncmp(header, "#", 1));

	sscanf_s(header, "%u %u", &width, &height);
	if (rgba_data == nullptr)
	{
		fclose(fPtr);
		return true;
	}
	tmp = fgets(header, 256, fPtr); // Format
	(void)tmp;
	if (cPtr == nullptr || strncmp(header, "255\n", 3))
	{
		fclose(fPtr);
		return false;
	}

	rowPitch = (width * 8 * sizeof(uint8_t[4]) + 7) / 8;

	for (auto y = 0u; y < height; ++y)
	{
		uint8_t *rowPtr = rgba_data;
		for (auto x = 0u; x < width; ++x)
		{
			size_t s = fread(rowPtr, 3, 1, fPtr);
			(void)s;
			rowPtr[3] = UINT8_MAX; // Alpha of 1
			rowPtr += 4;
		}
		rgba_data += rowPitch;
	}
	fclose(fPtr);

	return true;
}

int main(int argc, char **argv)
{
	char filename[256] = "RenderingX.ppm";
	if (argc > 1) strncpy_s(filename, argv[1], sizeof(filename));

	setFontSize(1);

	uint32_t width, height, rowPitch;
	loadTexture(filename, nullptr, width, height, rowPitch);

	uint8_t *data = new uint8_t[width * height * 4];
	loadTexture(filename, data, width, height, rowPitch);

	char setWidth[256], setHeight[256];
	sprintf_s(setWidth, "mode con cols=%u", width * 2);
	sprintf_s(setHeight, "mode con lines=%u", height + 1);

	system(setWidth);
	system(setHeight);

	auto rowPtr = data;
	for (auto y = 0u; y < height; ++y)
	{
		auto ptr = rowPtr;
		for (auto x = 0u; x < width; ++x)
		{
			float r = ptr[0] / 255.0f;
			float g = ptr[1] / 255.0f;
			float b = ptr[2] / 255.0f;
			ptr += 4;
			
			setNextPixel(r, g, b);
		}
		rowPtr += rowPitch;
		cout << endl;
	}

	delete [] data;

	cout << setcolor(DEFAULT_TEXT_COLOR);

	system("pause");

	return 0;
}
