#include "mupeSdCardWeb.h"
#include "mupeSdCardNvs.h"
#include "mupeWeb.h"
#include "mupeMdnsNtp.h"
#include "esp_http_server.h"

const char *sdCard_uri_txt = "SdCard init";
static const char *TAG = "mupeSdCardWeb";
#define STARTS_WITH(string_to_check, prefix) (strncmp(string_to_check, prefix, (strlen(prefix))))



esp_err_t sdCard_get_handler(httpd_req_t *req) {
	extern const unsigned char sdCard_index_start[] asm("_binary_sdCard_html_start");
	extern const unsigned char sdCard_index_end[] asm("_binary_sdCard_html_end");
	const size_t sdCard_index_size = (sdCard_index_end - sdCard_index_start);
	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char*) sdCard_index_start, sdCard_index_size);
	return ESP_OK;
}
esp_err_t root_sdCard_post_handler(httpd_req_t *req) {

	ESP_LOGI(TAG, "root_post_handler req->uri=[%s]", req->uri);
	ESP_LOGI(TAG, "root_post_handler content length %d", req->content_len);

	return sdCard_get_handler(req);
}



esp_err_t sdCard_get_cfg(httpd_req_t *req) {
	ESP_LOGI(TAG, "modbus_get_cfg %s ", req->uri);

	char value[50];
	httpd_resp_set_type(req, "text/html");

	httpd_resp_send_chunk(req, value, strlen(value));
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

esp_err_t root_sdCard_get_handler(httpd_req_t *req) {
	ESP_LOGI(TAG, "HTTP req %s ", req->uri);

	return sdCard_get_handler(req);;
}

httpd_uri_t sdCard_uri_get = { .uri = "/sdcard/*", .method = HTTP_GET,
		.handler = root_sdCard_get_handler };

httpd_uri_t sdCard_uri_post = { .uri = "/sdcard/*", .method = HTTP_POST,
		.handler = root_sdCard_post_handler, .user_ctx = NULL };

void mupeSdCardWebInit(void) {
	ESP_LOGI(TAG, "mupeSdCardWebInit");
	addHttpd_uri(&sdCard_uri_post, sdCard_uri_txt);
	addHttpd_uri(&sdCard_uri_get, sdCard_uri_txt);
}
