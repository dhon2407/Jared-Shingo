#ifndef __BLE_COMMUNICATOR_H__
#define __BLE_COMMUNICATOR_H__

#define LED_INDICATOR_ERROR D5
#define LED_INDICATOR_CONNECTED D7

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*data_callback)(int);

void ble_comm_init(data_callback onChangeValue);
void ble_comm_deinit(void);


#ifdef __cplusplus
}
#endif

#endif /* __BLE_COMMUNICATOR_H__ */