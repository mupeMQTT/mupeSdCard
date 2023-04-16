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

DatabaseNvs* databaseNvsGet(DatabaseNvs *databaseNvs) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id, "%llu", databaseNvs->id);
	size_t strSize = 0;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_blob(my_handle, id, NULL, &strSize);
	nvs_get_blob(my_handle, id, databaseNvs, &strSize);
	nvs_close(my_handle);
	return databaseNvs;
}

uint8_t databaseCheckTopic(char *topic, DatabaseNvs *databaseNvsret,
		uint8_t pos) {
	nvs_iterator_t it = NULL;
	esp_err_t res = nvs_entry_find(NVS_DEFAULT_PART_NAME, NAMESPACE_NAME,
			NVS_TYPE_BLOB, &it);
	uint8_t posx = 0;
	while (res == ESP_OK) {
		nvs_entry_info_t info;
		nvs_entry_info(it, &info);
		DatabaseNvs databaseNvs;
		databaseNvs.id = atoll(info.key);
		databaseNvsGet(&databaseNvs);
		if (strcmp(databaseNvs.topic, topic) == 0) {
			if (posx == pos) {
				databaseNvsret->id = databaseNvs.id;
				strcpy(databaseNvsret->parameter, databaseNvs.parameter);
				strcpy(databaseNvsret->topic, databaseNvs.topic);
				strcpy(databaseNvsret->unixTime, databaseNvs.unixTime);
				return 0;
			}
			posx++;
		}

		res = nvs_entry_next(&it);
	}
	nvs_release_iterator(it);
	return 1;

}

void sendDatabaseCfg(httpd_req_t *req) {

	nvs_iterator_t it = NULL;
	char value[150];
	esp_err_t res = nvs_entry_find(NVS_DEFAULT_PART_NAME, NAMESPACE_NAME,
			NVS_TYPE_BLOB, &it);
	while (res == ESP_OK) {
		nvs_entry_info_t info;
		nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
		DatabaseNvs databaseNvs;
		databaseNvs.id = atoll(info.key);
		databaseNvsGet(&databaseNvs);
		sprintf(value, "<tr><th>%s</th><th>%s</th><th>%s</th>", databaseNvs.topic, databaseNvs.unixTime,
				databaseNvs.parameter);
		httpd_resp_send_chunk(req, value, strlen(value));
		sprintf(value,
				"<th><button onclick=\"onClickDel(%llu)\">del</button></th></tr>",
				databaseNvs.id);
		httpd_resp_send_chunk(req, value, strlen(value));

		res = nvs_entry_next(&it);
	}
	nvs_release_iterator(it);

}
void sendDatabaseCfgCSV(httpd_req_t *req) {

	nvs_iterator_t it = NULL;
	char value[95];
	esp_err_t res = nvs_entry_find(NVS_DEFAULT_PART_NAME, NAMESPACE_NAME,
			NVS_TYPE_BLOB, &it);
	int lauf=0;
	while (res == ESP_OK) {
		if (lauf!=0){
			sprintf(value, "%s","\n");
			httpd_resp_send_chunk(req, value, strlen(value));
		}
		nvs_entry_info_t info;
		nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
		DatabaseNvs databaseNvs;
		databaseNvs.id = atoll(info.key);
		databaseNvsGet(&databaseNvs);
		sprintf(value, "%s,%s,chart_div%i", databaseNvs.topic,databaseNvs.parameter,lauf);
		httpd_resp_send_chunk(req, value, strlen(value));
		res = nvs_entry_next(&it);
		lauf++;
	}
	httpd_resp_send_chunk(req, NULL, 0);
	nvs_release_iterator(it);

}
void sendDatabaseData(httpd_req_t *req) {

	nvs_iterator_t it = NULL;
	char value[200];
	esp_err_t res = nvs_entry_find(NVS_DEFAULT_PART_NAME, NAMESPACE_NAME,
			NVS_TYPE_BLOB, &it);
	while (res == ESP_OK) {
		nvs_entry_info_t info;
		nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
		DatabaseNvs databaseNvs;
		databaseNvs.id = atoll(info.key);
		databaseNvsGet(&databaseNvs);
		sprintf(value, "<th  colspan=\"4\"><form action=\"	 \" method=\"post\"><div class=\"container\">");
		httpd_resp_send_chunk(req, value, strlen(value));
		sprintf(value, "<input type=\"text\" placeholder=\"dataget\" name=\"dataget\" id=\"dataget\" value=\"%s/%s\" size=\"30\" readonly>", databaseNvs.topic,databaseNvs.parameter);
		httpd_resp_send_chunk(req, value, strlen(value));
		sprintf(value, "<input type=\"number\" placeholder=\"from\" name=\"from\" id=\"from\" value=\"24\" size=\"10\">");
		httpd_resp_send_chunk(req, value, strlen(value));
		sprintf(value, "<input type=\"number\" placeholder=\"to\" name=\"to\" id=\"to\" value=\"0\" size=\"10\">");
		httpd_resp_send_chunk(req, value, strlen(value));
		sprintf(value,"<button type=\"submit\">Get</button></div></form></th></tr>");
		httpd_resp_send_chunk(req, value, strlen(value));

		res = nvs_entry_next(&it);
	}
	nvs_release_iterator(it);

}
void databaseNvsSet(DatabaseNvs *databaseNvs) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id, "%llu", databaseNvs->id);
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_set_blob(my_handle, id, databaseNvs, sizeof(DatabaseNvs));
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

void databaseNvsDel(char *id) {
	nvs_handle_t my_handle;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_erase_key(my_handle, id);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}
