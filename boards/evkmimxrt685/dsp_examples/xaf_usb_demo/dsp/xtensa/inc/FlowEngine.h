#ifndef _FLOW_ENGINE_
#define _FLOW_ENGINE_
/*
 * Copyright (C) 2019 Tymphany
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

 /**
  * @file FlowEngine.h
  *
  * @brief Flow Engine C public api
  *
  */

  /**
   \mainpage Flow Engine C public api
   * FlowEngine.h provides several public interface to construct audio dsp processing capabilities for various processors. <br>
   *
   * A typical usage of Flow Engine C api is as follows: <br>
   * FlowEngine_Mempool_Create() -> FlowEngine_create() -> FlowEngine_prepare()->[FlowEngine_query_cmd()] to create signal flow -> FlowEngine_process16() or FlowEngine_process32() to process audio
   *
   * 1.The minimum requirement is to call FlowEngine_create() to create a Flow Engine instance. <br>
   * @attention In Cortex-M processors, user need to allocate a static int8 array and call FlowEngine_Mempool_Create() to create a memory pool before create a Flow Engine instance.
   *
   * 2.After creating a instance, call FlowEngine_prepare() to allocate every resources that Flow Engine needed.  <br><br>
   * 
   * 3.To create a signal flow that could apply on the processor, it will need to use internal message system by calling a bunch of FlowEngine_query_cmd() to do this.
	   Please refer to "Flow Engine on RT1060 training.pptx" for more details. <br><br>

   * 4.Finally, place process function like FlowEgnine_process16() or FlowEngine_process32() to audio ISR or processing task in the system. <br><br>
   * 
   * 5.If there is any chance to release resource use by Flow Engine, call FlowEngine_destroy() <br><br>

  **/

#include "DllExport.h"



#include <stdbool.h>
typedef struct _FlowEngine FlowEngine;

/**
* @brief Create FlowEngine's memory pool
* @param [in] engine	pointer to Flow Engine instance
* @param [in] buf	buffer pointer
* @param [in] buf_len	buffer size in bytes
*
* @details Call this before calling FloeEngine_Create, this will provide heap allocation pool for all the Flow Engine apis.
* A single pool can provide for multiple engine instances
*
*/
FLOW_API void FlowEngine_Mempool_Create(void* buf, long buf_len);

/**
* @brief Create the FlowEngine instance.
* @param [in] name			instance name
* @param [in] configPath	specify the config file path(.flw), if no config file specify NULL
* @param [in] inChCount		input channel counts
* @param [in] outChCount	output channel counts
* @return					pointer to Flow Engine instance
*
* @details Creates the FlowEngine instance. All other APIs will fail until you call this.
* For the configPath, if the system don't support file system use NULL as argument. And create the signal flow by message
*/


FLOW_API FlowEngine* FlowEngine_create(const char* name, const char* configPath, int inCount, int outCount);


/**
* @brief Prepare the Flow Engine
* @param [in] sampleRate	sample rate in Hz
* @param [in] frameSize		sample counts to process per frame
* @return	
*
* @details This will create internal processing audio buffers by framesize, and set the default processing sample rate
* 
*/

FLOW_API int FlowEngine_prepare(FlowEngine* engine, float sampleRate, int frameSize);

/**
* @brief update Flow Engine's sample rate and framesize
* @param [in] engine	pointer to Flow Engine instance
* @param [in] sampleRate	sample rate in Hz
* @param [in] frameSize		sample counts to process per frame
*
* @details sometimes the system may change sample rate or framesize, remember to call FlowEngine_release before you do this.
* @attention This also needs to use FlowEngine_set_cmd to re-create the layout. <br>
* 
*/

FLOW_API void FlowEngine_Update_SampleRate_and_FrameSize(FlowEngine* engine, float sampleRate, int frameSize);



/**
* @brief Destroy the FlowEngine instance.
* @param [in] engine	pointer to Flow Engine instance
*
* @details This destroys the FlowEngine instance. You can later call FlowEngine_create() to create a new
* instance.
*/

FLOW_API void FlowEngine_destroy(FlowEngine* engine);

/**
* @brief Release current Audio Objects resources that own by Flow Engine
* @param [in] engine	pointer to Flow Engine instance
*
* @details This will delete the all of the audio objects own by Flow Engine, to create new signal layout you must <br>
* use FlowEngine_query_cmd to send relative message to create and wire the audio objets<br>
*/
FLOW_API void FlowEngine_release(FlowEngine* engine);

#ifdef COMM_HAS_TCP
/**
* @brief Construct a FlowEngine message server for socket based communication
* @param [in] engine	pointer to Flow Engine instance
* @return						0 (success) or -1 (failed)
*
* @details The FlowEngine server accepts multiple client connection, note that the default port of the server is 54010.
* Please refer to the Flow message protocol for more information
*
*/

FLOW_API int FlowEngine_create_tcp_server(FlowEngine* engine, int port);

/**
* @brief Check if message server is running
* @param [in] engine	pointer to Flow Engine instance
* @return						true (running) or false (not running)
*
* @details Helper function to check if Flow Engine's message server is running
*
*/
FLOW_API bool FlowEngine_is_server_running(FlowEngine* engine);

#endif

#ifdef COMM_HAS_UART

/**
* @brief Construct a FlowEngine message server for uart based communication
* @param [in] engine	pointer to Flow Engine instance
* @return						0 (success) or -1 (failed)
*
* @details 
*
*/

FLOW_API int FlowEngine_create_uart_server(FlowEngine* engine, const int baudrate);

/**
* @brief Check if message server is running
* @param [in] engine	pointer to Flow Engine instance
* @return	true (running) or false (not running)
*
* @details Helper function to check if Flow Engine's message server is running
*
*/

FLOW_API bool FlowEngine_is_server_running(FlowEngine* engine);

/**
* @brief send cmd through uart server
* @param [in] engine	pointer to Flow Engine instance
* @param [in] cmd string command to send
*
* @details
*
*/

FLOW_API void FlowEngine_uart_server_set_cmd(FlowEngine* engine, const char* cmd);

#endif



/**
 * @brief Send command to Flow engine and get its response
 * @param [in] engine	pointer to Flow Engine instance
 * @param [in] cmd command in string type
 * @return response in string type
 *
 * @details Detail of Command list please refer to the FlowDsp_message_commands.xlsx
 * @attention This function is not thread safe
 */

FLOW_API const char* FlowEngine_query_cmd(FlowEngine* engine, const char* cmd);


/**
 * @brief Get pointer to engine's input buffer 
 * @param [in] engine	pointer to Flow Engine instance
 * @param [in] ch_num	channel index of the input buffer
 * @return	pointer to buffer in float
 *
 * @details Get the pointer to engine's input buffer
 * @attention the data type only support float
 */
FLOW_API float* FlowEngine_get_in_buf(FlowEngine* engine, int ch_num);

/**
 * @brief Get pointer to engine's output buffer
 * @param [in] engine	pointer to Flow Engine instance
 * @param [in] ch_num	channel index of the output buffer
 * @return	pointer to buffer in float 
 *
 * @details Get the pointer to engine's output buffer
 * @attention the data type only support float
 */

FLOW_API float* FlowEngine_get_out_buf(FlowEngine* engine, int ch_num);


/**
* @brief Process audio samples
* @param [in] engine	pointer to Flow Engine instance
* @param [in]  in		samples to send to Flow Engine
* @param [out] out		where to write processed samples
* @return						0 (success) or -1 (failed)
*
* @details Processes the input samples through the configuration file to the output samples
* You can use FlowEngine_get_in_buf and FlowEngine_get_out_buf to point to the input and 
* output buffer in your audio system, the data type is float
*
*/
void FlowEngine_Process_Simple(FlowEngine* engine, int frameSize);

/**
* @brief Process audio samples in float
* @param [in] engine	pointer to Flow Engine instance
* @param [in]  in		samples to send to Flow Engine
* @param [out] out		where to write processed samples
* @return						0 (success) or -1 (failed)
*
* @details Processes the input samples through the configuration file to the output samples <br>
* @attention Audio Data should placed using double pointer first dimension is channel, second dimension is samples <br>
* e.g <br>
* in[1]: pointer to input buffer for 2nd channel <br>
* out[2][3]: 4th sample of 3rd output channel <br>
* out[7]: pointer to output buffer for 8th channel <br>
* 
*/

FLOW_API void FlowEngine_Process(FlowEngine* engine, const float** in, float** out, int frameSize);

/**
* @brief Process audio samples
* @param [in] engine	pointer to Flow Engine instance
* @param [in]  in		samples to send to Flow Engine
* @param [out] out		where to write processed samples
* @return						0 (success) or -1 (failed)
*
* @details Processes the input samples through the configuration file to the output samples
* @attention Audio Data should place in interleaved format
* <table>
* <tr><th>IN1 <th>IN2 <th>IN... <th>IN1 <th>IN2 <th>IN... <th>IN1 <th>IN2 <th>...
* </table>
*/

FLOW_API void FlowEngine_ProcessInt16(FlowEngine* engine, const int16_t* in, int16_t* out);


/**
* @brief Process audio samples
* @param [in] engine	pointer to Flow Engine instance
* @param [in]  in		samples to send to Flow Engine
* @param [out] out		where to write processed samples
* @return						0 (success) or -1 (failed)
*
* @details Processes the input samples through the configuration file to the output samples
* @attention Audio Data should place in interleaved format
* <table>
* <tr><th>IN1 <th>IN2 <th>IN... <th>IN1 <th>IN2 <th>IN... <th>IN1 <th>IN2 <th>...
* </table>
*/

FLOW_API void FlowEngine_ProcessInt32(FlowEngine* engine, const int32_t* in, int32_t* out);


/**
* @brief Low priority process for data communication between each audio objects
* @param [in] engine	pointer to Flow Engine instance
* @param [in]  in		pointer to input data
* @param [out] out		pointer to output data
* @return						0 (success) or -1 (failed)
*
* @details This is low priority process function for audio objects with control port (orange node in Flow Studio)
* @attention Don't call this function in high priority audio ISR, place it into lower priority task or ISR

*/

FLOW_API void FlowEngine_ControlProcess(FlowEngine* engine, const int16_t* in, int16_t* out);

/**
 * @brief Get number of memory usage in memory pool
 * @param [in] engine	pointer to Flow Engine instance
 * @return Space in Bytes
 */

FLOW_API long FlowEngine_GetHeapUsage(FlowEngine* engine);

/**
 * @brief Get number of free space can be used
 * @param [in] engine	pointer to Flow Engine instance
 * @return Space in Bytes
 */
FLOW_API long FlowEngine_GetHeapFreeSpace(FlowEngine* engine);



#endif
