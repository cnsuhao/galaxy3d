#include "GTFile.h"
#include <fstream>
#include "Debug.h"
#include "GTString.h"
#include "Application.h"

#ifdef WINPHONE
#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>
#endif

#ifdef WINPHONE
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
#ifdef WINPHONE
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
#endif

#ifdef WINPHONE
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
}