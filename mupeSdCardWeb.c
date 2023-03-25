#include "mupeSdCardWeb.h"
#include "mupeSdCardNvs.h"
#include "mupeWeb.h"
#include "mupeMdnsNtp.h"
#include "esp_http_server.h"
#include "mupeSdCard.h"
#include "mupeDataBase.h"

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
	char *buf = malloc(req->content_len + 1);
	uint16_t from = 0;
	uint16_t to = 0;

	size_t off = 0;
	while (off < req->content_len) {
		/* Read data received in the request */
		int ret = httpd_req_recv(req, buf + off, req->content_len - off);
		if (ret <= 0) {
			if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
				httpd_resp_send_408(req);
			}
			free(buf);
			return ESP_FAIL;
		}
		off += ret;
		ESP_LOGI(TAG, "root_post_handler recv length %d", ret);
	}
	buf[off] = '\0';
	ESP_LOGI(TAG, "root_post_handler buf=[%s]", buf);

	uint32_t sdCardNvs = 0;
	DatabaseNvs databaseNvs;
	char value[20];
	if (find_value("miso=", buf, value) > 0) {
		sdCardNvs = sdCardNvs + (atoi(value) << 24);
	}
	if (find_value("mosi=", buf, value) > 0) {
		sdCardNvs = sdCardNvs + (atoi(value) << 16);
	}
	if (find_value("clk=", buf, value) > 0) {
		sdCardNvs = sdCardNvs + (atoi(value) << 8);
	}
	if (find_value("cs=", buf, value) > 0) {
		sdCardNvs = sdCardNvs + atoi(value);
		ESP_LOGI(TAG, "sdCardNvs %lu", sdCardNvs);
		sDCardSet(sdCardNvs);
	}
	if (find_value("topic=", buf, value) > 0) {
		char search[] = "%2F";
		char replace[] = "/";
		stringReplace(search, replace, value);
		strcpy(databaseNvs.topic, value);
	}
	if (find_value("parameter=", buf, value) > 0) {
		databaseNvs.id = getNowMs() / 1000;
		strcpy(databaseNvs.parameter, value);
		databaseNvsSet(&databaseNvs);
	}
	if (find_value("from=", buf, value) > 0) {
		from = atoi(value);
	}
	if (find_value("to=", buf, value) > 0) {
		to = atoi(value);
	}
	if (find_value("dataget=", buf, value) > 0) {
		char search[] = "%2F";
		char replace[] = "/";
		stringReplace(search, replace, value);
		double _from = getNowMinus(from);
		double _to = getNowMinus(to);

		char topic[40];
		strcpy(topic,value);
		topic[strrchr(topic,'/')-topic]=0;
		ESP_LOGI(TAG, "topic %s ", topic);
		ESP_LOGI(TAG, "parameter %s ", strrchr(value,'/')+1);

		size_t size = mqttDataBaseGetSize(_from, _to, topic,
				strrchr(value,'/')+1);
		MupeDbDtata *mupeDbDtata;
		mupeDbDtata=malloc(size*sizeof(MupeDbDtata));
		mqttDataBaseGetData(_from, _to, topic,
				strrchr(value,'/')+1,mupeDbDtata);
		httpd_resp_set_type(req, "text/plain");
		char value[70];
	//	sprintf(value, "'Datum','Wert'");
	//	httpd_resp_send_chunk(req, value, strlen(value));
		for (size_t i = 0; i < size; ++i) {
			sprintf(value, "%s%f,%f",i==0?"":"\n", mupeDbDtata[i].ts, mupeDbDtata[i].data);
			httpd_resp_send_chunk(req, value, strlen(value));
		}
		httpd_resp_send_chunk(req, NULL, 0);
		free(mupeDbDtata);
		free(buf);
		return ESP_OK;
	}
	free(buf);
	return sdCard_get_handler(req);
}

esp_err_t sdCard_get_list(httpd_req_t *req) {
	ESP_LOGI(TAG, "modbus_get_cfg %s ", req->uri);

	httpd_resp_set_type(req, "text/html");

	char value[70];
	strcpy(value, "<html> <body><table style=\"width:50%\"> <tr>");
	httpd_resp_send_chunk(req, value, strlen(value));
	strcpy(value, "<th>Topic</th><th>Parameter</th><th>del</th>");
	httpd_resp_send_chunk(req, value, strlen(value));

	sendDatabaseCfg(req);

	strcpy(value, "</tr></table></body></html>");

	httpd_resp_send_chunk(req, value, strlen(value));
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}
esp_err_t sdCard_get_ldata(httpd_req_t *req) {
	ESP_LOGI(TAG, "modbus_get_cfg %s ", req->uri);

	httpd_resp_set_type(req, "text/html");

	char value[70];
	strcpy(value, "<html> <body><table style=\"width:50%\"> <tr>");
	httpd_resp_send_chunk(req, value, strlen(value));
	strcpy(value,
			"<th>Data From</th><th>from</th><th>to</th><th>Get</th></tr>");
	httpd_resp_send_chunk(req, value, strlen(value));

	sendDatabaseData(req);

	strcpy(value, "</tr></table></body></html>");

	httpd_resp_send_chunk(req, value, strlen(value));
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

esp_err_t sdCard_get_cfg(httpd_req_t *req) {
	ESP_LOGI(TAG, "modbus_get_cfg %s ", req->uri);

	char value[50];
	httpd_resp_set_type(req, "text/html");
	uint32_t sdCardNvs;
	sdCardNvs = sDCardGet();
	sprintf(value, "%lu\n%lu\n%lu\n%lu", (sdCardNvs >> 24) & 0xff,
			sdCardNvs >> 16 & 0xff, sdCardNvs >> 8 & 0xff, sdCardNvs & 0xff);
	ESP_LOGI(TAG, "sdCard_get_cfg%s ", value);
	httpd_resp_send_chunk(req, value, strlen(value));
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

esp_err_t root_sdCard_get_handler(httpd_req_t *req) {
	ESP_LOGI(TAG, "HTTP req %s ", req->uri);
	if (STARTS_WITH(req->uri, "/sdcard/cfg") == 0) {
		return sdCard_get_cfg(req);
	} else if (STARTS_WITH(req->uri, "/sdcard/list") == 0) {
		return sdCard_get_list(req);
	} else if (STARTS_WITH(req->uri, "/sdcard/ldata") == 0) {
		return sdCard_get_ldata(req);
	} else if (STARTS_WITH(req->uri, "/sdcard/del") == 0) {
		databaseNvsDel((char*) &(req->uri[12]));
	} else {
		char path[HTTPD_MAX_URI_LEN];
		strcpy(path, MOUNT_POINT);
		strcat(path, "/webs/");
		strcat(path, &(req->uri[8]));
		ESP_LOGI(TAG, "Path %s ", path);
		httpd_resp_set_type(req, "text/html");
		char value[500];
		FILE *fp;
		if ((fp = fopen(path, "r")) == NULL) {
			return sdCard_get_handler(req);
		}
		size_t size = 0;
		do {
			size = fread(value, 1, sizeof(value), fp);
			httpd_resp_send_chunk(req, value, size);

		} while (size == sizeof(value));
		fclose(fp);
		httpd_resp_send_chunk(req, NULL, 0);
		return ESP_OK;

	}
	return sdCard_get_handler(req);
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
