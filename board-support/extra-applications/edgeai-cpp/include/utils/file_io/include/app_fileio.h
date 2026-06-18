/*
 *
 * Copyright (c) 2023 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef APP_FILEIO_H_
#define APP_FILEIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * \defgroup group_vision_apps_utils_fileio File IO APIs
 *
 * \brief This section contains APIs for file operations from remote cores
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/** \brief Max CPUs that are participating in the logging */
#define APP_FILEIO_MAX_CPUS        (16u)

/** \brief CPU name to use as prefix while logging */
#define APP_FILEIO_MAX_CPU_NAME    (16u)

/** \brief Size of memory used for logging by one CPU */
#define APP_FILEIO_PER_CPU_MEM_SIZE   (200*1024u)

#define APP_TRACE_FILE_NAME_SIZE (512u)

#define APP_FILE_MODE_SIZE (4u)

#define MAX_NUM_FILES (20U)

/** \brief Shared memory structure used for instruction communication across remote cores and MPU */
typedef struct 
{
    int32_t      offset;    /**< Offset for file operation */
    int32_t      size;      /**< Size of data elements read/write in file operation; mimics size of fread/fwrite */
    int32_t      count;     /**< Number of data elements to read/write in file operation; mimics nmemb of fread/fwrite */
    int32_t      opCode;    /**< Indicates the type of file io operation to be performed as specified in TI_FILEIO_OPCODE enum */
    char         fileName[APP_TRACE_FILE_NAME_SIZE]; /**< Absolute path of the file on which file operations to be performed */
    char         mode[APP_FILE_MODE_SIZE];           /**< Mode for operning file (e.g. 'r', 'wb' 'a', etc.) */
    int32_t      rsv;       /**< Reserved to make structure 64b aligned */
    uint64_t     fid;       /**< File descriptor id of opened file */
} OpStruct_t;


/** \brief Shared memory structure used for file io by a specific CPU
 *
 *  This state of the fileio instructions, user can ignore it.
 */
typedef struct {
    volatile  int32_t fileCtr;             /**< Indicates index in the fileArray to use for file operation */
    volatile  int32_t writeIndicatorFlag;  /**< Set to 1 to indicate remote core write complete, ARM sets it to 0 indicate read is complete*/
    volatile  int32_t retValue;            /**< Status value returned by ARM to remote cores */
    volatile  int32_t rsv1;                /**< Reserved to make structure 64b aligned */
    volatile  uint8_t fileio_cpu_name[APP_FILEIO_MAX_CPU_NAME]; /**< Init by writer to CPU name, used by reader to add a prefix when writing to console device */
    volatile  uint8_t fileio_mem[APP_FILEIO_PER_CPU_MEM_SIZE];  /**< memory address into which logs are written by this CPU */
    volatile  OpStruct_t opStruct;         /**< Struct containing instructions to be communicated across cores */
    volatile  uint64_t fp[MAX_NUM_FILES];  /**< File descriptor object array for open files */
} app_fileio_cpu_shared_mem_t;


/**< \brief Shared memory structure for all CPUs, used by reader and writer CPUs
 *
 * User application MUST map this to the same physical address across all CPUs.
 * For non-coherent CPUs, this MUST map to a non-cache region
 * */
typedef struct {
    app_fileio_cpu_shared_mem_t cpu_shared_mem[APP_FILEIO_MAX_CPUS]; /**< CPU specific shared memory structure */
} app_fileio_shared_mem_t;

/** \brief Init parameters to use for appFileIOInit()
 *
 * */
typedef struct {
    app_fileio_shared_mem_t *shared_mem;       /**< Shared memory to use for file read/write, all CPUs must point to the same shared memory. This is physical address in case of linux */
    uint32_t self_cpu_index;                /**< Index into shared memory area for self CPU to use to use when writing. Two CPUs must not use the same CPU index */
    char     self_cpu_name[APP_FILEIO_MAX_CPU_NAME]; /**< self CPU name */
    uint32_t fileio_rd_poll_interval_in_msecs; /**< polling interval for fileio reader in msecs */
    uint32_t fileio_rd_max_cpus;               /**< Maximum CPUs that fileio into the shared memory */
    uint32_t fileio_rd_cpu_enable[APP_FILEIO_MAX_CPUS]; /**< 1: enable doing FIle IO operations for core, else disable */
} app_fileio_init_prm_t;

/** \brief Initialize app_fileio_init_prm_t with default parameters
 *         always call this function before calling appFileIOInit
 *
 * It is recommended to call this API before calling appFileIOInit().
 * User should override init parameters after calling this API.
 *
 * \param prms [in] Init parameters
 */
void appFileIOInitPrmSetDefault(app_fileio_init_prm_t *prms);

/** \brief Init File IO reader and fileio writer,
 *
 * \param prms [in] Init parameters.
 *
 * \return 0 on success, else failure.
 */
int32_t appFileIORdInit(app_fileio_init_prm_t *prms);

/** \brief Init File IO reader and fileio writer,
 *
 * \param prms [in] Init parameters.
 *
 * \return 0 on success, else failure.
 */
int32_t appFileIOWrInit(app_fileio_init_prm_t *prms);

/** \brief De-init fileio reader and fileio writer
 *
 * \return 0 on success, else failure.
 */
int32_t appFileIORdDeInit();

/** \brief De-init fileio reader and fileio writer
 *
 * \return 0 on success, else failure.
 */
int32_t appFileIOWrDeInit();

/** \brief Fwrite function call from remote cores
 *
 * \param filename [in] Absolute path of file name.
 * \param ptr [in] Ptr from which to write data to file.
 * \param size [in] Size of data element to write.
 * \param count [in] Number of elements to write
 *
 * \return 0 on success, else failure.
 */
int32_t appWriteBinToFile(const char * filename, void * ptr, int32_t size, int32_t count);

/** \brief Fread function call from remote cores
 *
 * \param filename [in] Absolute path of file name.
 * \param ptr [in] Ptr to which to read data from file.
 * \param size [in] Size of data element to read.
 * \param count [in] Number of elements to read
 *
 * \return 0 on success, else failure.
 */
int32_t appReadBinFromFile(const char * filename, void * ptr, int32_t size, int32_t count);

/** \brief Utility function to get file name from file absolute path
 *
 * \param path [in] Absolute path to file.
 *
 * \return Pointer to file name
 */
char * getFileNameFromPath(char* path);

/** \brief Utility function to get directory name from file absolute path
 *
 * \param path [in] Absolute path to file.
 */
void getDirFromPath(char* path);
/* @} */

#ifdef __cplusplus
}
#endif

#endif

