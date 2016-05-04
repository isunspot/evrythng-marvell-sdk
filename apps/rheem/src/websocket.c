/*
 *  Copyright (C) 2008-2016, Marvell International Ltd.
 *  All Rights Reserved.
 */
#include <httpc.h>
#include <httpd.h>
#include <websockets.h>
#include <lwip/api.h>
#include <base64.h>

#define STACK_SIZE (1024*7)

#ifndef CONFIG_ENABLE_HTTPC_SECURE
#error WMSDK should compiled with CONFIG_ENABLE_HTTPC_SECURE defined
#endif

static os_thread_stack_define(test_thread_stack, STACK_SIZE);
static os_thread_t test_thread_handle;

const char* property = "raw";
const char* thng_id = "Uhcyy9s4kNpStCCpApm3qs8h";
const char* device_api_key = "mtpJlwhoX009jnNJdMfq8gfvzHipFK7FiPr6hfx40Xb1cHZfIlyEPoIjofcDAD0l3PHjsuTWlJPyiqYR";
const char* base_url = "wss://ws.evrythng.com:443";
const char* resource_format = "/thngs/%s/properties/%s";
const char* data_format = "[{\"value\":\"%s\"}]";

static ws_context_t ws;
static http_session_t hS;

static int create_websocket()
{
    int ret = WM_SUCCESS;

    size_t resource_len = strlen(resource_format) + 
        strlen(thng_id) +
        strlen(property);

    char* resource = os_mem_alloc(resource_len);
    snprintf(resource, resource_len, resource_format, thng_id, property);

	int status;
	httpc_cfg_t httpc_cfg = { 0 };
    httpc_cfg.flags |= TLS_ENABLE;
	httpc_cfg.retry_cnt = DEFAULT_RETRY_COUNT;
	status = http_open_session(&hS, base_url, &httpc_cfg);
	if (status != WM_SUCCESS) {
        ret = -WM_FAIL;
        goto create_exit;
	}
	wmprintf("HTTP connection established to %s\r\n", base_url);

	http_req_t req;
	req.type = HTTP_GET;
	req.resource = resource;
	req.version = HTTP_VER_1_1;
	status = http_prepare_req(hS, &req, STANDARD_HDR_FLAGS);
	if (status != WM_SUCCESS) {
		http_close_session(&hS);
        ret = -WM_FAIL;
        goto create_exit;
	}
    
    http_add_header(hS, &req, "authorization", device_api_key);

	status = ws_upgrade_socket(&hS, &req, "chat", &ws);
	if (status != WM_SUCCESS) {
		http_close_session(&hS);
        ret = -WM_FAIL;
        goto create_exit;
	}

create_exit:
    os_mem_free(resource);
    if (ret == WM_SUCCESS)
        wmprintf("Socket upgraded to websocket\r\n");
	return ret;
}

/*
 * This function sends data to the server
 */
static int send_data(void *arg)
{
	int ret;
	ws_frame_t f;
	char *data = (char *) arg;
	f.fin = 1;
	f.opcode = WS_TEXT_FRAME;
	f.data = (uint8_t *)data;
	f.data_len = strlen(data);
	wmprintf("Sending data: %s\r\n", data);
	ret = ws_frame_send(&ws, &f);
	if (ret <= 0) {
		return -WM_FAIL;
	} else
		return WM_SUCCESS;
}


/*
 * This function reads data from the server
 */
static int receive_data()
{
	ws_frame_t f;
	if (ws_frame_recv(&ws, &f)) {
		wmprintf("Received data: %s\r\n", f.data);
		http_close_session(&hS);
		wmprintf("Connection closed\r\n");
		return WM_SUCCESS;
	} else {
		wmprintf("Error in read\r\n");
		http_close_session(&hS);
		ws_close_ctx(&ws);
		wmprintf("Connection closed\r\n");
		return -WM_FAIL;
	}
}


/* It calls the send and receive data */
static void websocket_test(void *arg)
{
	if (create_websocket() != WM_SUCCESS) {
		wmprintf("Error in connecting to cloud\r\n");
        goto test_exit;
	}

	if (send_data(arg) == WM_SUCCESS) {
		receive_data();
    }
	else
		wmprintf("Error in sending data\r\n");

test_exit:
    os_mem_free(arg);
	test_thread_handle = NULL;
	os_thread_delete(NULL);
}

/*
 * Cli handle for websocket-test cli
 */
void cmd_websocket_handle(int argc, char **argv)
{
    int i;
    unsigned char bin[0xff];
    for (i = 0; i < sizeof bin; i++) bin[i] = i;
    char* data;
    size_t data_len;

	if (argc < 2) {
        //encode bin to base64 and send as payload if no args provided
        int base64_len = base64encode_len(sizeof bin);
        char* base64_str = os_mem_alloc(base64_len);
        if (WM_SUCCESS != base64encode(bin, sizeof bin, base64_str, &base64_len)) {
            wmprintf("failed to base64 encode binary data\r\n");
            os_mem_free(base64_str);
            return;
        }
        data_len = strlen(data_format) + strlen(base64_str);
        data = os_mem_alloc(data_len);
        snprintf(data, data_len, data_format, base64_str);
	} else {
        //send provided arg as a payload
        data_len = strlen(data_format) + strlen(argv[1]);
        data = os_mem_alloc(data_len);
        snprintf(data, data_len, data_format, argv[1]);
	}

    //free data buf in thread
    os_thread_create(&test_thread_handle,
            "Websocket-Test", websocket_test, (void *) data,
            &test_thread_stack, OS_PRIO_3);
}
