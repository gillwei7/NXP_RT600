var a00062 =
[
    [ "usart_config_t", "a00062.html#a00391", [
      [ "baudRate_Bps", "a00062.html#a5d2631bc772901b4114b01770f9bb337", null ],
      [ "parityMode", "a00062.html#aff1d7e368b35ae89db8aa393207e7ccc", null ],
      [ "stopBitCount", "a00062.html#aeb0b8a38f5d7e0def2aa1b079643682f", null ],
      [ "bitCountPerChar", "a00062.html#ab964b3fbce4b824beff770a138fd4b6e", null ],
      [ "loopback", "a00062.html#a9892d7a138f2245bc9b7fe4e6c1652fb", null ],
      [ "enableRx", "a00062.html#a8b9813693fdfd2116e0d6019ea39b41d", null ],
      [ "enableTx", "a00062.html#a2c3b5faf44be34a152232b9dfaaf064d", null ],
      [ "enableContinuousSCLK", "a00062.html#a8d618d37f6867480bd0e1fcd9e5b50a5", null ],
      [ "enableMode32k", "a00062.html#aed39bca3e2fca1cb8a548e7f7a5cd650", null ],
      [ "enableHardwareFlowControl", "a00062.html#ac34e16abe408d6e9daa37741420f7ec0", null ],
      [ "txWatermark", "a00062.html#af7968e954beeaf72f99ebd1243e6e7f5", null ],
      [ "rxWatermark", "a00062.html#a29a5d5a30658b044a09ad86373dae217", null ],
      [ "syncMode", "a00062.html#ad5debb2aa90fc8fa732edd1ab4bdbbea", null ],
      [ "clockPolarity", "a00062.html#ac808e4908f1d6f9ca78fec3d0b8f118a", null ]
    ] ],
    [ "usart_transfer_t", "a00062.html#a00393", [
      [ "data", "a00062.html#acad21ced979ce0c2af98f0b1837863c9", null ],
      [ "rxData", "a00062.html#a44bd38d7d89b7e185ac9bb49bd7214ea", null ],
      [ "txData", "a00062.html#ad081b93c86f16dedd57dfc105a71e7fa", null ],
      [ "dataSize", "a00062.html#a605c0a56df2815ffb3461aaaf116752a", null ]
    ] ],
    [ "usart_handle_t", "a00062.html#a00274", [
      [ "txData", "a00062.html#a2e0eada6edeef87ca5609381a495caa0", null ],
      [ "txDataSize", "a00062.html#aafb090dd1831c9e83df2d60cf3cd25f0", null ],
      [ "txDataSizeAll", "a00062.html#add7cbfa096ebb8bfe5084c9940d0a9d8", null ],
      [ "rxData", "a00062.html#a7fcedbe0ea6e380084a05ba1bb06e38d", null ],
      [ "rxDataSize", "a00062.html#a94a60ffa711d28e3c9151c9990a9145c", null ],
      [ "rxDataSizeAll", "a00062.html#a35109a019b73257d24021ad79d951e64", null ],
      [ "rxRingBuffer", "a00062.html#a2cac1523ac58da64d4076c6155309d2f", null ],
      [ "rxRingBufferSize", "a00062.html#ae1c5e6a8ccdd56a678e8dda316e43845", null ],
      [ "rxRingBufferHead", "a00062.html#afb838e91205d1aa1a2b064591bc11c4e", null ],
      [ "rxRingBufferTail", "a00062.html#a197ce470fd9b42e53a867ef84eb2b6dd", null ],
      [ "callback", "a00062.html#a50fd5afc23c86d872ee2a1d46bd4145e", null ],
      [ "userData", "a00062.html#adbecb8574e5d62ee38761a3b4c30e4c3", null ],
      [ "txState", "a00062.html#a30a2856c0e736ad39fe44c015bd54ca4", null ],
      [ "rxState", "a00062.html#a7f621935f46f5f1bdd10ca755e1b51a7", null ],
      [ "txWatermark", "a00062.html#a2239cb0b41a90b9bb9a3853600386190", null ],
      [ "rxWatermark", "a00062.html#a24428e55f6a2e2c3778f80e3c0cbe47d", null ]
    ] ],
    [ "FSL_USART_DRIVER_VERSION", "a00062.html#gad737c0eee502276b7615e83ec3a79be4", null ],
    [ "UART_RETRY_TIMES", "a00062.html#gac0f88b67f77fd05f186a5ec940c340bd", null ],
    [ "usart_transfer_callback_t", "a00062.html#ga9688f27725349ed0dd7a37c9a75eccc0", null ],
    [ "flexcomm_usart_irq_handler_t", "a00062.html#ga7bfd3f835fa5bd9faa9590687077e125", [
      [ "kStatus_USART_TxBusy", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a78157ec0658118f9205521c03da99093", null ],
      [ "kStatus_USART_RxBusy", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a38225397e3744ef1b4995041ce80d839", null ],
      [ "kStatus_USART_TxIdle", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a539a0ebaa0137fdf6d7cc9b984acb13f", null ],
      [ "kStatus_USART_RxIdle", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a3cb7112c821c89d7fbb1c048a86756aa", null ],
      [ "kStatus_USART_TxError", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57abf010640733ea1ae8c97d7a0b83ea11b", null ],
      [ "kStatus_USART_RxError", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a888280fbe6cf9b4b0647db78f0578ff5", null ],
      [ "kStatus_USART_RxRingBufferOverrun", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a256720c63c378f57791e946c0473290e", null ],
      [ "kStatus_USART_NoiseError", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a3f21596ffe8300152d7f9f5587402e19", null ],
      [ "kStatus_USART_FramingError", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a9e2f548075400b4c0a719ee1b0a534a9", null ],
      [ "kStatus_USART_ParityError", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57a579872694419f8708e38935593105868", null ],
      [ "kStatus_USART_BaudrateNotSupport", "a00062.html#ggab04a0655cd1e3bcac5e8f48c18df1a57ae427c8491ce7294dfeaa3b87e506bd81", null ]
    ] ],
    [ "usart_sync_mode_t", "a00062.html#ga7ecd603d2579abbe714d58eb582821b8", [
      [ "kUSART_SyncModeDisabled", "a00062.html#gga7ecd603d2579abbe714d58eb582821b8af7a378247f82677090007d0c38be78f2", null ],
      [ "kUSART_SyncModeSlave", "a00062.html#gga7ecd603d2579abbe714d58eb582821b8a71df5907ecd16c5f08fe1e6673064958", null ],
      [ "kUSART_SyncModeMaster", "a00062.html#gga7ecd603d2579abbe714d58eb582821b8af279132afb0bee88bae3065c20c79ab2", null ]
    ] ],
    [ "usart_parity_mode_t", "a00062.html#ga9b5ca9521874092ccb637a02d7b26ba2", [
      [ "kUSART_ParityDisabled", "a00062.html#gga9b5ca9521874092ccb637a02d7b26ba2a46309b174047a84a78c77b7648bdf21b", null ],
      [ "kUSART_ParityEven", "a00062.html#gga9b5ca9521874092ccb637a02d7b26ba2aad8d786301da1bb92e2b911c386d39eb", null ],
      [ "kUSART_ParityOdd", "a00062.html#gga9b5ca9521874092ccb637a02d7b26ba2a64df3b823d2ab5f3f56ffb3f520eafb1", null ]
    ] ],
    [ "usart_stop_bit_count_t", "a00062.html#ga58ab07609b094f719f903475de6e57b4", [
      [ "kUSART_OneStopBit", "a00062.html#gga58ab07609b094f719f903475de6e57b4aa637b8f5f0edd262181a20c1fbed7c12", null ],
      [ "kUSART_TwoStopBit", "a00062.html#gga58ab07609b094f719f903475de6e57b4a3aee3195dc850778e33f2ebacf8847d3", null ]
    ] ],
    [ "usart_data_len_t", "a00062.html#ga28e46a3538cf5f5140523132a963283c", [
      [ "kUSART_7BitsPerChar", "a00062.html#gga28e46a3538cf5f5140523132a963283ca9562d6cdd240e9808a9a1d1946f400b7", null ],
      [ "kUSART_8BitsPerChar", "a00062.html#gga28e46a3538cf5f5140523132a963283cad71aee19329a4127614132d30e50ad8a", null ]
    ] ],
    [ "usart_clock_polarity_t", "a00062.html#ga786ba5b98195c3df810a061b6c0cca91", [
      [ "kUSART_RxSampleOnFallingEdge", "a00062.html#gga786ba5b98195c3df810a061b6c0cca91a40fdc56c60a07652a4283491087eed6b", null ],
      [ "kUSART_RxSampleOnRisingEdge", "a00062.html#gga786ba5b98195c3df810a061b6c0cca91aea6908db956ab3f686c7ee54d68d401c", null ]
    ] ],
    [ "usart_txfifo_watermark_t", "a00062.html#ga51645e760853f2899afe701fd8bafad6", [
      [ "kUSART_TxFifo0", "a00062.html#gga51645e760853f2899afe701fd8bafad6a4c06b40c032f50878b21a2d50ada6dce", null ],
      [ "kUSART_TxFifo1", "a00062.html#gga51645e760853f2899afe701fd8bafad6a1d9f1d7d066818718be802be7d375672", null ],
      [ "kUSART_TxFifo2", "a00062.html#gga51645e760853f2899afe701fd8bafad6a4e893f1b0acc108a4438e5e0246cfd98", null ],
      [ "kUSART_TxFifo3", "a00062.html#gga51645e760853f2899afe701fd8bafad6a956a6639ce7a77854b2b8818600056d1", null ],
      [ "kUSART_TxFifo4", "a00062.html#gga51645e760853f2899afe701fd8bafad6a125165264a20e9a99ebe8809c537519d", null ],
      [ "kUSART_TxFifo5", "a00062.html#gga51645e760853f2899afe701fd8bafad6aba62dcf8bffa3fea75659cd15d85f5f5", null ],
      [ "kUSART_TxFifo6", "a00062.html#gga51645e760853f2899afe701fd8bafad6ad5789e845dda660016f5ad20d44543c5", null ],
      [ "kUSART_TxFifo7", "a00062.html#gga51645e760853f2899afe701fd8bafad6a2d5f3f3e606d0234022ba79c51006c79", null ]
    ] ],
    [ "usart_rxfifo_watermark_t", "a00062.html#gadc4d91bd718e2b3748ec626875703f15", [
      [ "kUSART_RxFifo1", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15ae4c7d10afbc0733440891ebe03b16b3f", null ],
      [ "kUSART_RxFifo2", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15a0234bec87111ef277cf08bf1cdacf081", null ],
      [ "kUSART_RxFifo3", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15a1b8335b872c2a509fd2a623b161975cc", null ],
      [ "kUSART_RxFifo4", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15a6ea188f34c9f1c2ff50b71526e42a687", null ],
      [ "kUSART_RxFifo5", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15aa1868626ee2cca1ca8cc225b71c7a7bf", null ],
      [ "kUSART_RxFifo6", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15a4819c5fd1c6c274e2e6f08b8aa2bad19", null ],
      [ "kUSART_RxFifo7", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15a9bb99b104055213d54116ea79f0a2586", null ],
      [ "kUSART_RxFifo8", "a00062.html#ggadc4d91bd718e2b3748ec626875703f15a6e09aeee58582acc9b9baeb10315f024", null ]
    ] ],
    [ "_usart_interrupt_enable", "a00062.html#ga78b340bf3b25bfae957d0c5532b9b853", [
      [ "kUSART_TxIdleInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853ac5611ab6b973e6777a7d7db8321f62f2", null ],
      [ "kUSART_CtsChangeInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853a3d02580815ae2cc516b2eceaa5af7cb1", null ],
      [ "kUSART_RxBreakChangeInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853a8025155917fdba2167f615ac1f6fd0e8", null ],
      [ "kUSART_RxStartInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853ae642012aae33bd8954699572e4adb475", null ],
      [ "kUSART_FramingErrorInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853a5cd5bf36e32a46d994643a712fd08ea9", null ],
      [ "kUSART_ParityErrorInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853a3dc275bab22f77a1182bbab5fd5cea3b", null ],
      [ "kUSART_NoiseErrorInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853ac8d3050ed3df9d9278f74c9ab27b2ddc", null ],
      [ "kUSART_AutoBaudErrorInterruptEnable", "a00062.html#gga78b340bf3b25bfae957d0c5532b9b853a7154c6ede60243bf5f9e727a5cb18ce5", null ]
    ] ],
    [ "_usart_flags", "a00062.html#gaa6a81e7e82e236bb1c3c5ba306466610", [
      [ "kUSART_TxError", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a88f35fb71e2d5a7bea4f8eed7b6a050c", null ],
      [ "kUSART_RxError", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a8aa9eb127699721457f3ec9e1025246b", null ],
      [ "kUSART_TxFifoEmptyFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610af11f0420c46f24642414bb66d2e389c7", null ],
      [ "kUSART_TxFifoNotFullFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a584ded6df831827495ab206619c81174", null ],
      [ "kUSART_RxFifoNotEmptyFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a39952b4f24141f895bab7822dc330330", null ],
      [ "kUSART_RxFifoFullFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a519cf0825b102a001c88bfba6506824c", null ],
      [ "kUSART_RxIdleFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610abc766d479d00f480e78d05f21f8e1d59", null ],
      [ "kUSART_TxIdleFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a8106fa188ea138f48c7740dcf5c2b294", null ],
      [ "kUSART_CtsAssertFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a43d6b4aef70a978f4794efabd5457f51", null ],
      [ "kUSART_CtsChangeFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a7119fd7ce16b8aa8a98b1d82271698dd", null ],
      [ "kUSART_BreakDetectFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610aca92bd8dc74c45b62e3f9b4c707b8638", null ],
      [ "kUSART_BreakDetectChangeFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610aa73bdb740d60558f33c28c7bc4cffab3", null ],
      [ "kUSART_RxStartFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a2fab8368a6a3246714c83405202626b3", null ],
      [ "kUSART_FramingErrorFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a44b58335a118179dd7b3f5d760a314fa", null ],
      [ "kUSART_ParityErrorFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a46d487d50d8392d25c5c4e76cdfb6674", null ],
      [ "kUSART_NoiseErrorFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610a989650ce4c9a8f57ad771a33a2cb0335", null ],
      [ "kUSART_AutobaudErrorFlag", "a00062.html#ggaa6a81e7e82e236bb1c3c5ba306466610ae1cf980779fba1c617584d5ebc8ac3f8", null ]
    ] ],
    [ "USART_GetInstance", "a00062.html#ga0433386bbc6c2ac7e2ec7925fea263db", null ],
    [ "USART_Init", "a00062.html#ga2aeb4e11fdf0eb515d090865ffcf2ba2", null ],
    [ "USART_Deinit", "a00062.html#ga80892980b702b0b7614691014a5aaaca", null ],
    [ "USART_GetDefaultConfig", "a00062.html#ga74464f9357e585e42e5f3c909eb6e9b3", null ],
    [ "USART_SetBaudRate", "a00062.html#gabdcaf539e6c95903c779f4538b1f422b", null ],
    [ "USART_Enable32kMode", "a00062.html#gaa0b9131928d9bb14ef0af71c2d7b5ece", null ],
    [ "USART_Enable9bitMode", "a00062.html#gae81c380620e158c78a4693de48bc0d05", null ],
    [ "USART_SetMatchAddress", "a00062.html#gae2fcb06824d8966b682297342a0cbd25", null ],
    [ "USART_EnableMatchAddress", "a00062.html#gafe0eb431d7deccc370a4ecc5224c1aed", null ],
    [ "USART_GetStatusFlags", "a00062.html#ga33f5cdc2918edf02b9a4ef8a12e27442", null ],
    [ "USART_ClearStatusFlags", "a00062.html#ga235a301d8f22d6ce3a6f77dbdb76cfe3", null ],
    [ "USART_EnableInterrupts", "a00062.html#ga9132cf15ff5fd2ac007a88f9400bcf30", null ],
    [ "USART_DisableInterrupts", "a00062.html#ga091d6509100a6e6206483b2f41f16d6c", null ],
    [ "USART_GetEnabledInterrupts", "a00062.html#ga49c6966b2af5f0de0fbf49240acc1856", null ],
    [ "USART_EnableTxDMA", "a00062.html#ga1bdec9d5c2cfeb180579b9bb730ff3a3", null ],
    [ "USART_EnableRxDMA", "a00062.html#ga5a227e096aa1f7e50129f2adf35bf5a4", null ],
    [ "USART_EnableCTS", "a00062.html#gaaab4d6b7d7701fbdd022694330207614", null ],
    [ "USART_EnableContinuousSCLK", "a00062.html#gad0a37042ec21be840247a05ca9e8e805", null ],
    [ "USART_EnableAutoClearSCLK", "a00062.html#ga4d96acc82e0bbe947ececc200083c68a", null ],
    [ "USART_SetRxFifoWatermark", "a00062.html#gad1b92754b85be70151d0d1f9377a2794", null ],
    [ "USART_SetTxFifoWatermark", "a00062.html#gadfe67091db3b60f25bbfa04708dae354", null ],
    [ "USART_WriteByte", "a00062.html#gae7a92a20789cf111acadb543916aac91", null ],
    [ "USART_ReadByte", "a00062.html#gaaa9dc0edc5edacd55bb88ff7b9b55f98", null ],
    [ "USART_GetRxFifoCount", "a00062.html#gaea6b5fa1c42eee5ab2819af23afe197a", null ],
    [ "USART_GetTxFifoCount", "a00062.html#ga03ef6205dca5ba44c0b29ff2f2175fae", null ],
    [ "USART_SendAddress", "a00062.html#ga6ed8281b59aa8a1e181ae2692475adef", null ],
    [ "USART_WriteBlocking", "a00062.html#ga95abde78d5cbf569f2b6e4d7942b64df", null ],
    [ "USART_ReadBlocking", "a00062.html#ga09c3b588e2099ffa6b999c5ca7fb8d58", null ],
    [ "USART_TransferCreateHandle", "a00062.html#ga9aed876794d5c2ab2e37196242602b89", null ],
    [ "USART_TransferSendNonBlocking", "a00062.html#ga7f5a4e7e831ab0dc869637494a57362e", null ],
    [ "USART_TransferStartRingBuffer", "a00062.html#ga8500107ace3179c85e66d73fa1f41d16", null ],
    [ "USART_TransferStopRingBuffer", "a00062.html#gaedb2f678849734999e373a8b00a5248c", null ],
    [ "USART_TransferGetRxRingBufferLength", "a00062.html#gac0c8e42457c41efd02a4310423804e77", null ],
    [ "USART_TransferAbortSend", "a00062.html#ga095596e064fa8aa6bac0927e71b0329c", null ],
    [ "USART_TransferGetSendCount", "a00062.html#ga56acdf4c0046516f59c04d89e8e988e6", null ],
    [ "USART_TransferReceiveNonBlocking", "a00062.html#ga34c6c819c9e2d8608a6d7e63103fa542", null ],
    [ "USART_TransferAbortReceive", "a00062.html#ga2de9bf9557d2f7bc6fe0236608cb958e", null ],
    [ "USART_TransferGetReceiveCount", "a00062.html#ga43cc02b4521f48ae0b5626a1e8f87a0e", null ],
    [ "USART_TransferHandleIRQ", "a00062.html#gae955a5e336b3cb3276d5cdd6f1f6feda", null ]
];