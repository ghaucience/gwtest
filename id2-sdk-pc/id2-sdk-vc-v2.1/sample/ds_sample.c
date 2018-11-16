/**
 * Copyright (C) 2017 The YunOS IoT Project. All rights reserved.
 */

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <process.h>
#include "datasource.h"

typedef struct _tee_id2_prov_head_t {
    uint32_t magic;   /* 0x764f7250 */
    uint32_t version; /* 0x00030000 */
    uint32_t rsvd[4]; /* 0x00000000 */
    uint32_t rec_num;
    uint8_t recs[];
} tee_id2_prov_head_t;

typedef struct _ds_thrd_ctx_t {
    uint32_t num;
    int ret;
} ds_thrd_ctx_t;

static int __stdcall WriteDataToFile(char *file_name, uint8_t *data, uint32_t len)
{
    FILE *fp = NULL;

    if (file_name == NULL || data == NULL || len == 0) {
        printf("bad input args - file_name: %s len: %d\n", file_name, len);
        return -1;
    }

    errno = 0;
    fp = fopen(file_name, "wb");
    if (fp == NULL) {
        printf("fopen file(%s) fail(%d)\n", file_name, errno);
        return -1;
    }

    fwrite(data, len, 1, fp);

    fclose(fp);

    return 0;
}

unsigned int __stdcall thread_handle_func(void *arg)
{
    int ret = -1;
    ds_dev_t dev = NULL;
    uint32_t rec_idx;
    uint32_t rec_num;
    uint32_t rec_size;
    uint32_t prov_len = 0;
    uint32_t prov_offset = 0;
    ds_thrd_ctx_t *thrd_ctx;
    uint8_t file_name[64];
    uint8_t *rec_data = NULL;
    tee_id2_prov_head_t *prov_head = NULL;
    ds_dev_prov_stat_t prov_stat = DEV_PROV_STAT_SUCCESS;

    if (arg == NULL) {
        printf("no arg!\n");
        return -1;
    } else {
        thrd_ctx = (ds_thrd_ctx_t *)arg;
    }

    ret = ds_create_dev(&dev);
    if(ret != DS_STATUS_OK) {
        printf("ds_create_dev error.\n");
        goto bail;
    }

    if(ds_get_rec_num(dev, &rec_num) != DS_STATUS_OK) {
        printf("ds_create_dev error.\n");
        goto bail;
    }

    prov_len = sizeof(tee_id2_prov_head_t);
    for (rec_idx = 0; rec_idx < rec_num; rec_idx++) {
        ret = ds_get_rec(dev, rec_idx, NULL, &rec_size);
        if (ret != DS_STATUS_OK) {
            printf("ds_get_rec size fail(%d)\n",  ret);
            goto bail;
        } else {
            prov_len += rec_size;
        }
    }

    prov_head = (tee_id2_prov_head_t *)malloc(prov_len);
    if (prov_head == NULL) {
        printf("malloc(%d) fail.\n", prov_len);
        goto bail;
    } else {
        memset(prov_head, 0x0, prov_len);
        prov_head->magic = 0x764f7250;
        prov_head->version = 0x00020000;
        prov_head->rec_num = rec_num;
    }

    for (rec_idx = 0; rec_idx < rec_num; rec_idx++) {
        /* get rec size first*/
        ret = ds_get_rec(dev, rec_idx, NULL, &rec_size);
        if (ret != DS_STATUS_OK) {
            printf("ds_get_rec(idx: %d) size error (%d)\n", rec_idx, ret);
            goto bail;
        }

        rec_data = malloc(rec_size);
        if (rec_data == NULL) {
            printf("malloc(%d) fail\n", rec_size);
            goto bail;
        }

        ret = ds_get_rec(dev, rec_idx, rec_data, &rec_size);
        if (ret != DS_STATUS_OK) {
            printf("ds_get_rec(idx: %d) error (%d)\n", rec_idx, ret);
            goto bail;
        } else {
            memcpy(prov_head->recs + prov_offset, rec_data, rec_size);
            prov_offset += rec_size;
            free(rec_data);
            rec_data = NULL;
        }
    }

    memset(file_name, 0x0, 64);
    sprintf(file_name, "bin/teed_com%d.bin", thrd_ctx->num);

    if (-1 == _access("bin", 0)) {
        mkdir("bin");
    }

    ret = WriteDataToFile(file_name, (uint8_t *)prov_head, prov_len);
    if (ret < 0) {
        goto bail;
    }

bail:
    if (rec_data) {
        free(rec_data);
    }
    if (prov_head) {
        free(prov_head);
    }

    if (ret != DS_STATUS_OK) {
        prov_stat = DEV_PROV_STAT_ERROR_GENERIC;
    }
    if (ds_set_dev_prov_stat(dev, prov_stat) != DS_STATUS_OK) {
        printf("ds_set_dev_prov_stat(%d) fail\n", prov_stat);
    }

    ds_destroy_dev(dev);

    thrd_ctx->ret = ret;

    return ret;
}

#define threadCont 1

int main(int argc, char * argv[])
{
    HANDLE  handle[threadCont] = {NULL};
    int ret = -1;
    int n = 0;
    uint32_t i = 0;
    uint32_t loop_time = 1;
    ds_cfg_t cfg;
    ds_thrd_ctx_t *ctx[threadCont] = {NULL};

    cfg.port_num = threadCont;
    strcpy(cfg.lic_path, "licenseConfig.ini");
    ret = ds_init(&cfg);
    if (ret != DS_STATUS_OK) {
        printf("ds_init error.\n");
        goto bail;
    }

    for (i = 0; i < loop_time; i++) {
        printf("data source test loop_time: %d\n", i);

        for (n = 0; n < threadCont; n++) {
            ctx[n] = malloc(sizeof(ds_thrd_ctx_t ));
            if (ctx[n] == NULL) {
                printf("malloc(%d) fail\n",  sizeof(ds_thrd_ctx_t));
                goto bail;
            } else {
                ctx[n]->num = n;
                ctx[n]->ret = 0;
            }

            handle[n] = (HANDLE)_beginthreadex(NULL, 0, thread_handle_func, ctx[n], 0, NULL);
        }

        WaitForMultipleObjects(threadCont, handle, TRUE, INFINITE);
        for (n = 0; n < threadCont; n++) {
            if (ctx[n]->ret < 0) {
                printf("thread %d fail\n", n);
                goto bail;
	    }

            free(ctx[n]);
            CloseHandle(handle[n]);
            ctx[n] = NULL;
            handle[n] = NULL;
        }
    }

bail:
    for (n = 0; n < threadCont; n++) {
        if (ctx[n]) {
            free(ctx[n]);
        }
        if (handle[n]) {
            CloseHandle(handle[n]);
        }
    }

    ds_cleanup();

    return 0;
}
