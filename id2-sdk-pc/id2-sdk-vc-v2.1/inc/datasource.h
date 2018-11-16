/**
 * Copyright (C) 2017 The YunOS IoT Project. All rights reserved.
 */

#ifndef _DATA_SOURCE_H_
#define _DATA_SOURCE_H_

/**
 * Make sure we can call this stuff from C++.
 */

#include "basetype.h"
#include "errCode.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t ds_stat_t;

typedef enum _ds_dev_prov_stat_t
{
    DEV_PROV_STAT_SUCCESS       = 0,
    DEV_PROV_STAT_ERROR_GENERIC = -1,
    /* other stats */
} ds_dev_prov_stat_t;

typedef struct _ds_cfg_t
{
    uint32_t port_num;      /* maximum device port numeber for synchronous provisioning */
    uint8_t lic_path[256];  /* ID2 license file path */
} ds_cfg_t;

typedef void* ds_dev_t;

/*
* Data source Initialization
*
* cfg[in]: data source configurations
*/
__declspec(dllexport) ds_stat_t ds_init(ds_cfg_t *cfg);

/*
* Create device handle
*
* dev[out]: device handle
*/
__declspec(dllexport) ds_stat_t ds_create_dev(ds_dev_t* dev);

/*
* Get record number
*
* dev[in]:  device handle
* num[out]: record number
*/
__declspec(dllexport) ds_stat_t ds_get_rec_num(ds_dev_t dev, uint32_t *num);
/*
 * 1. call ds_get_rec with rec == NULL to get size
 * 2. rec = malloc(size);
 * 3. call ds_get_rec again
 */
__declspec(dllexport)  ds_stat_t ds_get_rec(ds_dev_t dev, uint32_t rec_idx, void *rec, uint32_t *size);

/*
* Set prov stat into data source
*
* dev[in]:       device handle
* prov_stat[in]: prov status
*/
__declspec(dllexport)  ds_stat_t ds_set_dev_prov_stat(ds_dev_t dev, ds_dev_prov_stat_t prov_stat);

/*
* Destroy device handle
*
* dev[in]: device handle
*/
__declspec(dllexport)  void ds_destroy_dev(ds_dev_t dev);

/*
* Data source cleanup
*
*/
__declspec(dllexport)  void ds_cleanup(void);

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif

#endif /* _DATA_SOURCE_H_ */
