
#include "connector.hpp"

#define VENDOR_ID  0x1B4F  // Change to your device's VID
#define PRODUCT_ID 0x9206  // Change to your device's PID
#define PACKET_SIZE 64     // Adjust based on your device's buffer size
#define TIMEOUT_MS 5000    // 5-second timeout for response

typedef int (__cdecl *hid_init_t)();
typedef hid_device* (__cdecl *hid_open_t)(unsigned short, unsigned short, const wchar_t*);
typedef int (__cdecl *hid_write_t)(hid_device*, const unsigned char*, size_t);
typedef int (__cdecl *hid_read_timeout_t)(hid_device*, unsigned char*, size_t, int);
typedef void (__cdecl *hid_close_t)(hid_device*);
typedef int (__cdecl *hid_exit_t)();

int initConn() {
    // Load HIDAPI dynamically
    HMODULE hHidApi = LoadLibraryA("./plugin/hidapi/hidapi.dll");
    if (!hHidApi) {
        std::cerr << "Failed to load hidapi.dll" << std::endl;
        return 1;
    }

    // Get function pointers
    hid_init_t hid_init = (hid_init_t)GetProcAddress(hHidApi, "hid_init");
    hid_open_t hid_open = (hid_open_t)GetProcAddress(hHidApi, "hid_open");
    hid_write_t hid_write = (hid_write_t)GetProcAddress(hHidApi, "hid_write");
    hid_read_timeout_t hid_read_timeout = (hid_read_timeout_t)GetProcAddress(hHidApi, "hid_read_timeout");
    hid_close_t hid_close = (hid_close_t)GetProcAddress(hHidApi, "hid_close");
    hid_exit_t hid_exit = (hid_exit_t)GetProcAddress(hHidApi, "hid_exit");

    if (!hid_init || !hid_open || !hid_write || !hid_read_timeout || !hid_close || !hid_exit) {
        std::cerr << "Failed to load functions from DLL" << std::endl;
        FreeLibrary(hHidApi);
        return 1;
    }

    // Initialize HIDAPI
    if (hid_init()) {
        std::cerr << "HIDAPI initialization failed" << std::endl;
        FreeLibrary(hHidApi);
        return 1;
    }

    // Open the HID device
    hid_device *handle = hid_open(VENDOR_ID, PRODUCT_ID, nullptr);
    if (!handle) {
        std::cerr << "Failed to open HID device" << std::endl;
        hid_exit();
        FreeLibrary(hHidApi);
        return 1;
    }

    // Prepare handshake message {0x01, 0x02, 0x03}
    unsigned char data[PACKET_SIZE] = {0};  // Zero-filled buffer
    data[1] = 0x69;
    data[2] = 0x01;
    data[3] = 0x02;
    data[4] = 0x03;

    // Send handshake request
    int res = hid_write(handle, data, PACKET_SIZE);
    if (res < 0) {
        std::cerr << "Failed to send handshake: " << res << std::endl;
        hid_close(handle);
        hid_exit();
        FreeLibrary(hHidApi);
        return 1;
    }
    std::cout << "Handshake sent: {0x69, 0x01, 0x02, 0x03}" << std::endl;

    // Read response (with timeout)
    unsigned char response[PACKET_SIZE] = {0};  // Buffer for response
    res = hid_read_timeout(handle, response, PACKET_SIZE, TIMEOUT_MS);
    
    if (res < 0) {
        std::cerr << "Failed to receive response" << std::endl;
    } else if (res == 0) {
        std::cerr << "Timeout waiting for response" << std::endl;
    } else {
        std::cout << "Received response: ";
        for (int i = 0; i < res; i++) {  // Skip response[0] (report ID if applicable)
            std::cout << "0x" << std::hex << (int)response[i] << " ";
        }
        std::cout << std::endl;
        if (response[0] == 0x01)
        {
            std::cout << "Connection success" << std::endl;
        }else
        {
            std::cout << "Connection failed" << std::endl;
        }
    }

    // Cleanup
    hid_close(handle);
    hid_exit();
    FreeLibrary(hHidApi);
    return 0;
}

int main() {
    int result = initConn();
    if (result != 0) {
        std::cerr << "Connection initialization failed with error code: " << result << std::endl;
        return result;
    }
    std::cout << "Connection initialized successfully." << std::endl;
    return 0;
}
