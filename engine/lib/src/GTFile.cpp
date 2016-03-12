#include "GTFile.h"
#include <fstream>
#include "Debug.h"
#include "GTString.h"
#include "Application.h"
#include "contrib/minizip/unzip.h"

#ifdef WINPHONE
#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>

#define WIN_STORAGE 0

template<class T>
static void wait_for_async(T ^A)
{
    while(A->Status == Windows::Foundation::AsyncStatus::Started)
    {
        //Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
    }

    Windows::Foundation::AsyncStatus S = A->Status;
}

#endif

namespace Galaxy3D
{
#if WIN_STORAGE
    bool GTFile::Exist(const std::string &path)
    {
        using namespace Windows::Storage;
        using namespace Concurrency;

        std::vector<char> ret;

        GTString path_full(path);
		path_full = path_full.Replace("/", "\\");

        wchar_t buffer[MAX_PATH];
        int wsize = MultiByteToWideChar(CP_ACP, 0, path_full.str.c_str(), path_full.str.size(), buffer, MAX_PATH);
        buffer[wsize] = 0;
        auto wpath = ref new Platform::String(buffer);

        try
        {
            auto file_async = StorageFile::GetFileFromPathAsync(wpath);
            wait_for_async(file_async);
			auto file = file_async->GetResults();
            return true;
        }
        catch(Platform::InvalidArgumentException ^e)
        {
            return false;
        }
        catch(Platform::COMException ^e)
        {
            if(e->HResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                Debug::Log("GTFile::Exist Platform::COMException ERROR_FILE_NOT_FOUND");
                return false;
            }
            else
            {
                throw(e);
            }
        }

        return false;
    }

    void *GTFile::ReadAllBytes(const std::string &path, int *size)
    {
        using namespace Windows::Storage;
        using namespace Concurrency;

        void *ret = NULL;

        GTString path_full(path);
		path_full = path_full.Replace("/", "\\");

		wchar_t buffer[MAX_PATH];
		int wsize = MultiByteToWideChar(CP_ACP, 0, path_full.str.c_str(), path_full.str.size(), buffer, MAX_PATH);
		buffer[wsize] = 0;
		auto wpath = ref new Platform::String(buffer);

        try
        {
			auto file_async = StorageFile::GetFileFromPathAsync(wpath);
			wait_for_async(file_async);
			auto file = file_async->GetResults();

            auto open_async = file->OpenReadAsync();
            wait_for_async(open_async);
            auto stream = open_async->GetResults();

            int bufferSize = (int) stream->Size;
            auto fileBuffer = ref new Streams::Buffer(bufferSize);
            auto read_async = stream->ReadAsync(fileBuffer, bufferSize, Streams::InputStreamOptions::None);
            wait_for_async(read_async);

            auto fileData = ref new Platform::Array<byte>(fileBuffer->Length);
            Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(fileData);

            *size = fileData->Length;
            ret = malloc(*size);
            memcpy(ret, fileData->Data, *size);

            return ret;
        }
        catch(Platform::InvalidArgumentException ^e)
        {
        }
        catch(Platform::COMException ^e)
        {
            if(e->HResult != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                throw(e);
            }
        }

        Debug::Log("File %s does not exist!!!", path.c_str());

        return ret;
    }

	void GTFile::WriteAllBytes(const std::string &path, void *data, int size)
	{
		using namespace Windows::Storage;

		GTString path_local = path.substr(Application::GetSavePath().length());
		auto splits = path_local.Split("/", true);

		auto folder = ApplicationData::Current->LocalFolder;
		for(size_t i=0; i<splits.size(); i++)
		{
			wchar_t buffer[MAX_PATH];
			int wsize = MultiByteToWideChar(CP_ACP, 0, splits[i].str.c_str(), splits[i].str.size(), buffer, MAX_PATH);
			buffer[wsize] = 0;
			auto wname = ref new Platform::String(buffer);

			if(i == splits.size() - 1)
			{
				// create or open file
				auto file_async = folder->CreateFileAsync(wname, CreationCollisionOption::ReplaceExisting);
				wait_for_async(file_async);
				auto file = file_async->GetResults();

				auto array = ref new Platform::Array<unsigned char>(size);
				memcpy(array->Data, data, size);

				auto write_async = FileIO::WriteBytesAsync(file, array);
				wait_for_async(write_async);
			}
			else
			{
				// create or open folder
				auto folder_async = folder->CreateFolderAsync(wname, CreationCollisionOption::OpenIfExists);
				wait_for_async(folder_async);
				folder = folder_async->GetResults();
			}
		}
	}
#else
	bool GTFile::Exist(const std::string &path)
    {
        std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
        if(is)
        {
            is.close();
            return true;
        }

        return false;
    }

    void *GTFile::ReadAllBytes(const std::string &path, int *size)
    {
        void *ret = NULL;

        std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
        if(is)
        {
            is.seekg(0, std::ios::end);
            int file_size = (int) is.tellg();
            is.seekg(0, std::ios::beg);

            if(file_size > 0)
            {
                ret = malloc(file_size);
                is.read((char *) ret, file_size);
                *size = file_size;
            }
            is.close();
        }
        else
        {
            Debug::Log("File %s does not exist!!!", path.c_str());
        }

        return ret;
    }

	void GTFile::WriteAllBytes(const std::string &path, void *data, int size)
    {
		auto folder = path.substr(0, path.rfind("/"));
		CreateFolder(folder);

        std::ofstream os(path.c_str(), std::ios::out | std::ios::binary);
        if(os)
        {
            os.write((const char *) data, size);
            os.close();
        }
    }
#endif

    void GTFile::ReadAllText(const std::string &path, std::string &str)
    {
        int size;
        void *buffer = ReadAllBytes(path, &size);
        if(buffer != NULL)
        {
            str.resize(size);
            memcpy(&str[0], buffer, size);
            free(buffer);
        }
    }

#ifdef WINPHONE
	void GTFile::CreateFolder(const std::string &path)
	{
		using namespace Windows::Storage;

		GTString path_local = path.substr(Application::GetSavePath().length());
		auto splits = path_local.Split("/", true);

		auto folder = ApplicationData::Current->LocalFolder;
		for(size_t i=0; i<splits.size(); i++)
		{
			wchar_t buffer[MAX_PATH];
			int wsize = MultiByteToWideChar(CP_ACP, 0, splits[i].str.c_str(), splits[i].str.size(), buffer, MAX_PATH);
			buffer[wsize] = 0;
			auto wname = ref new Platform::String(buffer);

			// create or open folder
			auto folder_async = folder->CreateFolderAsync(wname, CreationCollisionOption::OpenIfExists);
			wait_for_async(folder_async);
			folder = folder_async->GetResults();
		}
	}
#endif

	static void unzip_file(unzFile file, const std::string &path)
	{
		auto folder = path.substr(0, path.rfind("/"));
		GTFile::CreateFolder(folder);

		int result = unzOpenCurrentFilePassword(file, 0);

		int buffer_size = 8192;
		char *buffer = (char *) calloc(1, buffer_size);

        std::ofstream os(path.c_str(), std::ios::out | std::ios::binary);

		do
		{
			result = unzReadCurrentFile(file, buffer, buffer_size);
			if(result > 0)
			{
				if(os)
				{
					os.write(buffer, result);
				}
			}
		}while(result > 0);

		if(os)
        {
            os.close();
        }
		
		free(buffer);

		unzCloseCurrentFile(file);
	}

	void GTFile::Unzip(const std::string &zip_path, const std::string &source, const std::string &dest, bool directory)
	{
		unzFile file = unzOpen64(zip_path.c_str());

		if(file != NULL)
		{
			unz_file_info64 file_info;
			char filename_inzip[256];

			int result = unzGoToFirstFile(file);
			while(result == UNZ_OK)
			{
				result = unzGetCurrentFileInfo64(file, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
				if(result != UNZ_OK)
				{
					break;
				}

				if(directory)
				{
					GTString filename(filename_inzip);
					if(filename.StartsWith(source))
					{
						std::string dest_filename = dest + filename.str.substr(source.size());
						unzip_file(file, dest_filename);
					}
				}
				else
				{
					if(source == filename_inzip)
					{
						unzip_file(file, dest);
						break;
					}
				}

				result = unzGoToNextFile(file);
			}

			unzClose(file);
		}
	}
}

#ifdef WINPC
#include <Windows.h>

void Galaxy3D::GTFile::CreateFolder(const std::string &path)
{
	GTString path_local = path.substr(Application::GetSavePath().length());
	auto splits = path_local.Split("/", true);

	auto folder = Application::GetSavePath();
	for(size_t i=0; i<splits.size(); i++)
	{
		folder = folder + "/" + splits[i].str;

		CreateDirectoryA(folder.c_str(), NULL);
	}
}
#endif

#if defined(ANDROID) || defined(IOS)
#include <sys/types.h>
#include <sys/stat.h>

void Galaxy3D::GTFile::CreateFolder(const std::string &path)
{
	GTString path_local = path.substr(Application::GetSavePath().length());
	auto splits = path_local.Split("/", true);

	auto folder = Application::GetSavePath();
	for(size_t i=0; i<splits.size(); i++)
	{
		folder = folder + "/" + splits[i].str;

		mkdir(folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
}
#endif