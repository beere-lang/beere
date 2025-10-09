#include <windows.h>

int read_file(char* buff, const unsigned int buff_size, const char* path)
{
	HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	
	if (file == INVALID_HANDLE_VALUE)
	{
		return 1;
	}
	
	if (ReadFile(file, buff, buff_size - 1, NULL, NULL) == 0)
	{
		return 1;
	}
	
	buff[buff_size - 1] = '\0';

	return 0;
}