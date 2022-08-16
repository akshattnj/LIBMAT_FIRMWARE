#define FORMAT_SPIFFS_IF_FAILED true

#include <FS.h>
#include <SPI.h>
#include <SPIFFS.h>

class FileManager
{
public:
    bool cardConnected = false;
    /**
     * @brief Set up SPIFFS connection
     *
     */
    void connectSDCard()
    {
        if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
        {
            ESP_LOGE("Telemetry", "SPIFFS mount failed");
            cardConnected = false;
            return;
        }
        ESP_LOGI("Telemetry", "SPIFFS mount Success");
        cardConnected = true;
    }

    /**
     * @brief Write file to path
     *
     * @param path Path to write the file to
     * @param message Content to be written into file
     * @return true on success, false on fail
     */
    bool writeFile(const char *path, const char *message)
    {
        if (!cardConnected)
        {
            ESP_LOGE("File Management", "SPIFFS not connected");
            return false;
        }

        ESP_LOGI("File Management", "Writing file: %s\n", path);

        File file = SPIFFS.open(path, FILE_WRITE);
        if (!file)
        {
            ESP_LOGE("File Management", "Failed to open file for writing");
            return false;
        }
        if (file.print(message))
        {
            file.close();
            ESP_LOGI("File Management", "File written");
            return true;
        }
        else
        {
            file.close();
            ESP_LOGE("File Management", "Write failed");
            return false;
        }
    }

    /**
     * @brief Appends given message to end of file
     *
     * @param path Path of file
     * @param message Content to write
     * @return true on success, false on fail
     */
    bool appendFile(const char *path, const char *message)
    {
        if (!cardConnected)
        {
            ESP_LOGE("File Management", "SPIFFS not connected");
            return false;
        }

        ESP_LOGI("File Management", "Appending to file: %s\n", path);

        File file = SPIFFS.open(path, FILE_APPEND);
        if (!file)
        {
            ESP_LOGE("File Management", "Failed to open file for appending");
            return false;
        }
        if (file.print(message))
        {
            file.close();
            ESP_LOGI("File Management", "Message appended");
            return true;
        }
        else
        {
            file.close();
            ESP_LOGE("File Management", "Append failed");
            return false;
        }
    }

    /**
     * @brief Reads file in given path and stores data in provided char* array
     *
     * @param path Path of file to read
     * @param data Pointer to char array to store data in
     * @param dataLen Maximum size of char array for storing data
     * @return Size of file. 0 if reading failed
     */
    int readFile(const char *path, char *data, uint16_t dataLen)
    {
        if (!cardConnected)
        {
            ESP_LOGE("File Management", "SPIFFS not connected");
            return 0;
        }
        File file = SPIFFS.open(path, FILE_READ);
        if (!file)
        {
            ESP_LOGE("File Management", "Failed to open file for writing");
            return 0;
        }
        uint16_t fileSize = file.size();
        if (dataLen < fileSize + 1)
        {
            ESP_LOGE("File Management", "Storage variable too small to store data");
            return 0;
        }
        file.read((uint8_t *)data, (size_t)dataLen);
        data[fileSize] = '\0';
        return fileSize;
    }

    /**
     * @brief Delete file as per path provided
     *
     * @param path Path of file to be deleted
     * @return True if delete success. Else fail
     */
    bool deleteFile(const char *path)
    {
        if (!cardConnected)
        {
            ESP_LOGE("File Management", "SPIFFS not connected");
            return false;
        }
        ESP_LOGI("File Management", "Deleting file %s", path);
        if (SPIFFS.remove(path))
        {
            ESP_LOGI("File Management", "Delete Success");
            return true;
        }
        else
        {
            ESP_LOGE("File Management", "Delete failed");
            return false;
        }
    }

private:
};