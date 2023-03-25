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

#ifndef MUPESDCARDNVS_H
#define MUPESDCARDNVS_H
#include "esp_system.h"
#include "esp_http_server.h"

typedef struct DatabaseNvs{
	uint64_t id;
  char topic[50];
  char parameter[20];
} DatabaseNvs;

void mupeSdCardNvsInit(void);
uint32_t sDCardGet(void);
void sDCardSet(uint32_t SdCardNvs);
void sendDatabaseCfg(httpd_req_t *req);
void sendDatabaseData(httpd_req_t *req);
void databaseNvsSet(DatabaseNvs *databaseNvs);
void databaseNvsDel(char *id);
uint8_t databaseCheckTopic(char *topic, DatabaseNvs *databaseNvsret,uint8_t pos);
#endif
