#include <mupeDataBase.h>
#include <mupeSdCard.h>
#include <stdio.h>
#include <string.h>
#include "mupeSdCard.h"
#include <sys/stat.h>
#include "esp_log.h"
#include "mupeClientMqtt.h"
#include "mqtt_client.h"
#include "mupeMdnsNtp.h"
#include "mupeSdCardNvs.h"

static const char *TAG = "mupeDataBase";

void mqttDataBaseStore(char *topic, char *data, size_t sl, char *paramter) {

	struct stat st = { 0 };

	char fileName[strlen(topic) + strlen(MOUNT_POINT) + 9 + strlen(paramter)];
	strcpy(fileName, MOUNT_POINT);
	strcat(fileName, "/db");
	strcat(fileName, "/");
	strcat(fileName, topic);

	char path[sizeof(fileName)];
	int ofset = 1;
	do {
		if (strchr(&fileName[ofset], '/') != NULL) {
			ofset = strchr(&fileName[ofset], '/') - fileName;
			strncpy(path, fileName, ofset);
			path[ofset] = '\0';
		} else {
			strcpy(path, fileName);
		}
		if (stat(path, &st) == -1) {
			printf("%s %i \n", path, mkdir(path, 0700));
			printf("--\n");
		}
		ofset++;
	} while (strlen(path) != strlen(fileName));
	strcat(fileName, "/");
	strcat(fileName, paramter);
	strcat(fileName, ".dat");
	FILE *f = fopen(fileName, "ab");

	ESP_LOGI(TAG, "%s %i", fileName, f==NULL);
	if (strstr(data, paramter) != NULL) {
		char *pos1 = strstr(data, paramter);
		char *pos2 = strstr(pos1, ":");
		double d = atof(&pos2[1]);
		pos1 = strstr(data, "\"ts\":");
		pos2 = strstr(pos1, ":");
		double t = atof(&pos2[1]);
		fwrite(&t, sizeof(t), 1, f);
		fwrite(&d, sizeof(d), 1, f);
		ESP_LOGI(TAG, "store Data %s %f,%f", fileName, t, d);
	}
	fclose(f);
}

size_t findPos(FILE *fp, double utime) {
	MupeDbDtata mupeDbDtata;
	fseek(fp, 0L, SEEK_END);
	size_t f_s = ftell(fp);
	size_t h;
	h = f_s;
	h = h / 2;
	h = h - h % sizeof(mupeDbDtata);
	f_s = f_s / 2;
	f_s = f_s - f_s % sizeof(mupeDbDtata);
	fseek(fp, f_s, SEEK_SET);
	fread(&mupeDbDtata, 1, sizeof(mupeDbDtata), fp);
	while (h > sizeof(mupeDbDtata)) {
		h = h / 2;
		h = h - h % sizeof(mupeDbDtata);
		if (utime < mupeDbDtata.ts) {
			f_s = f_s - h;
		} else {
			f_s = f_s + h;
		}
		fseek(fp, f_s, SEEK_SET);
		fread(&mupeDbDtata, 1, sizeof(mupeDbDtata), fp);
	}
	ESP_LOGI(TAG, "DB POS %f %u", utime, f_s);
	return f_s;
}

size_t mqttDataBaseGetSize(double from, double to, char *topic, char *paramter) {

	char path[strlen(MOUNT_POINT) + strlen(topic) + strlen(paramter) + 8];
	strcpy(path, MOUNT_POINT);
	strcat(path, "/db/");
	strcat(path, topic);
	strcat(path, "/");
	strcat(path, paramter);
	strcat(path, ".dat");
	FILE *fp = fopen(path, "r");
	ESP_LOGI(TAG, "---%s ", path);
	size_t p_from = findPos(fp, from);
	size_t p_to = findPos(fp, to);
	fclose(fp);
	ESP_LOGI(TAG, "-mqttDataBaseGetSize--%s %i", path,
			(p_to - p_from) / sizeof(MupeDbDtata));
	return (p_to - p_from) / sizeof(MupeDbDtata);
}

void mqttDataBaseGetData(double from, double to, char *topic, char *paramter,
		MupeDbDtata *mupeDbDtata) {

	char path[strlen(MOUNT_POINT) + strlen(topic) + strlen(paramter) + 8];
	strcpy(path, MOUNT_POINT);
	strcat(path, "/db/");
	strcat(path, topic);
	strcat(path, "/");
	strcat(path, paramter);
	strcat(path, ".dat");
	FILE *fp = fopen(path, "r");
	ESP_LOGI(TAG, "---%s ", path);
	size_t p_from = findPos(fp, from);
	size_t p_to = findPos(fp, to);
	fseek(fp, p_from, SEEK_SET);
	fread(mupeDbDtata, sizeof(MupeDbDtata),
			(p_to - p_from) / sizeof(MupeDbDtata), fp);

	fclose(fp);

}

static void mqtt_event_handlerDB(void *handler_args, esp_event_base_t base,
		int32_t event_id, void *event_data) {
	//  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%l", base, event_id);
	esp_mqtt_event_handle_t event = event_data;
	//esp_mqtt_client_handle_t client = event->client;

	switch ((esp_mqtt_event_id_t) event_id) {
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		break;

	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_DATA:
		ESP_LOGI(TAG, "MQTT_EVENT_DATA");

		printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
		printf("DATA=%.*s\r\n", event->data_len, event->data);
		if (event->data_len > 0) {
			char topic[event->topic_len + 1];
			strncpy(topic, event->topic, event->topic_len);
			topic[event->topic_len] = '\0';
			DatabaseNvs databaseNvs;
			uint8_t pos=0;
			while  (databaseCheckTopic(topic, &databaseNvs,pos)==0) {
				mqttDataBaseStore(topic, event->data, event->data_len,
						databaseNvs.parameter);


				pos++;
			}

		}
		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGI(TAG, "MQTT_EVENT_ERROR");

		break;
	default:
		ESP_LOGI(TAG, "Other event id:%d", event->event_id);
		break;
	}
}

void mupeDataBaseInit() {
	esp_mqtt_client_handle_t client = get_esp_mqtt_client_handle_t();
	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
			mqtt_event_handlerDB,
			NULL);

}

