// Copyright Peter Müller mupe
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "nvs_flash.h"
#include "mupeSdCardNvs.h"

#define NAMESPACE_NAME "SdCardcfg"

void mupeSdCardNvsInit(void) {
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
}
uint32_t sDCardGet(void) {
	uint32_t SdCardNvs;
	nvs_handle_t my_handle;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_u32(my_handle, "SdCardNvs", &SdCardNvs);
	nvs_commit(my_handle);
	nvs_close(my_handle);
	return SdCardNvs;
}
void sDCardSet(uint32_t SdCardNvs) {

	nvs_handle_t my_handle;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_set_u32(my_handle, "SdCardNvs", SdCardNvs);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

