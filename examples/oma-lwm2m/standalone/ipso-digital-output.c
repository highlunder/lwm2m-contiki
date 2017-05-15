#include <stdint.h>
#include <string.h>
#include "lwm2m-object.h"
#include "lwm2m-engine.h"
#include "er-coap-engine.h"
//#include "ipso-defines.h"
#include "ipso-generic-control.h"

#define MODBUS 0

#if MODBUS
#include "modbus-protocol.h"
#include "rs485-handler.h"
#define INSTANCE_NUMBER 8
#else
#define INSTANCE_NUMBER 2
#endif

#define STANDALONE 0
#if !STANDALONE
void my_print(int a, int b) {
  printf("set %d %d\n", a, b);
}
#define gpio_relay_set my_print

#endif

#include "ipso-digital-output.h"


#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif /* DEBUG */



#define IO_STATE 5550
#define ALL_IO_STATES 8888
//#define
//static lwm2m_instance_t digital_output_instances[INSTANCE_NUMBER];

static lwm2m_status_t set_value(uint8_t value);
static lwm2m_status_t read_custom(uint16_t resource, uint8_t instance, lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize);

static lwm2m_status_t (*set_custom)(int32_t value, uint16_t resource, uint8_t instance);
static lwm2m_status_t (*read_IO)(uint8_t instance, lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize);
int (*write_to_register)(int, int);

#if MODBUS
static lwm2m_status_t read_modbus_IO(uint8_t instance, lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize);
static lwm2m_status_t read_all_modbus_states(lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize);
static lwm2m_status_t write_modbus_custom(uint16_t resource, uint8_t instance, lwm2m_context_t *ctx, const uint8_t *inbuf, size_t insize);
int write_to_modbus_register(int, int);
#else
static lwm2m_status_t read_sg_IO(uint8_t instance, lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize);
static lwm2m_status_t set_sg_custom(int32_t value, uint16_t resource, uint8_t instance);
int write_to_sg_register(int, int);
#endif

/*
 * Helper functions
 */
int read_application_type(void);

/*
 * Implementation
 */

static ipso_gen_control_t digital_IO_control[INSTANCE_NUMBER];

// = {
//  .reg_object.object_id = 3201,
//  .reg_object.instance_id = 0,
//  .set_value = set_value,
//  .write_custom = write_custom,
//  .read_custom = read_custom,
//};
//static ipso_control_t digital_IO_control1 = {
//  .reg_object.object_id = 3201,
//  .reg_object.instance_id = 1,
//  .set_value = set_value,
//  .write_custom = write_custom,
//  .read_custom = read_custom,
//};
//static ipso_control_t digital_IO_control2 = {
//  .reg_object.object_id = 3201,
//  .reg_object.instance_id = 2,
//  .set_value = set_value,
//  .write_custom = write_custom,
//  .read_custom = read_custom,
//};



/*---------------------------------------------------------------------------*/
static lwm2m_status_t set_value(uint8_t value)
{
  return LWM2M_STATUS_OK;
}
/*---------------------------------------------------------------------------*/

static lwm2m_status_t read_custom(uint16_t resource, uint8_t instance, lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize)
{
  if(resource == IO_STATE) {

    return read_IO(instance, ctx, outbuf, outsize);
  }
#if MODBUS
  if(resource == ALL_IO_STATES) {
    return read_all_states(ctx, outbuf, outsize);
  }
#endif

  return LWM2M_STATUS_ERROR;
}
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
#if MODBUS
static lwm2m_status_t
read_modbus_IO(uint8_t instance, lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize)
{
  int byteCount;
  if (instance >= INSTANCE_NUMBER) {
    return LWM2M_STATUS_ERROR;
  }
  PRINTF("--> REQ: read state\n");

  byteCount = read_from_modbus_register(instance);

  if (byteCount > 0) {
    char str[(byteCount * 2) + 1];
    modbus_get_data(str);
    if (strcmp(str, "01") != 0) {
      return ctx->writer->write_boolean(ctx, outbuf, outsize, 0);
    } else if (strcmp(str, "00") != 0) {
      return ctx->writer->write_boolean(ctx, outbuf, outsize, 1);
    }
  }
  return LWM2M_STATUS_ERROR;
}
#else
static lwm2m_status_t
read_sg_IO(uint8_t instance, lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize) {
	return LWM2M_STATUS_ERROR; //TODO
}
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#if MODBUS
/**
 * Write state function, return a positive number
 */
static lwm2m_status_t write_modbus_custom(uint16_t resource, uint8_t instance, lwm2m_context_t *ctx, const uint8_t *inbuf, size_t insize)
{

  size_t len;
  int byteCount = 0;
  int value;

  if (instance >= INSTANCE_NUMBER) {
    return LWM2M_STATUS_ERROR;
  }
  PRINTF("--> REQ: write state.\n");
  len = ctx->reader->read_int(ctx, inbuf, insize, &value);
  //PRINTF("Len: %d\n", len);
  if (len > 0) {
  	write_to_modbus_register(instance, value);
  } else {
    PRINTF("--> IPSO digital output - illegal write to on/off\n");
  }

  if (byteCount <= 0) {
    return 0;
  } else {
    char str[(byteCount * 2) + 1];
    modbus_get_echo(str);
  }
  return len;
}
#endif

/*************************************************************/
static lwm2m_status_t set_sg_custom(int32_t value, uint16_t resource, uint8_t instance)
{

  size_t len;
  int byteCount = 0;

  if (instance >= INSTANCE_NUMBER) {
    return LWM2M_STATUS_ERROR;
  }
  PRINTF("--> D-IO: set sg state\n");
  PRINTF("--> D-IO: value: %"PRId32"\n", value);
  return write_to_sg_register(instance, value);


}

#if MODBUS
/**
 * Read states of 7 outputs, return string value
 */
static lwm2m_status_t read_all_modbus_states(lwm2m_context_t *ctx, const uint8_t *outbuf, size_t outsize) {

  int byteCount;
  PRINTF("--> REQ: application type read all\n");

  byteCount = modbus_read_function(0x0010, 0x01, 0x0007);
  if (byteCount > 0) {
    char str[(byteCount * 2) + 1];
    modbus_get_data(str);
    return ctx->writer->write_string(ctx, outbuf, outsize, str, strlen(str));
  }
  PRINTF("INTERNAL ERROR\n");
  return 0;
}
#endif

/*---------------------------------------------------------------------------*/
void ipso_digital_output_init(void) {

  printf("Init digital IO!\n");
#if MODBUS
  read_IO = read_modbus_IO;
  write_custom = write_modbus_custom;
  write_to_register = write_to_modbus_register;

#else
  read_IO = read_sg_IO;
  set_custom = set_sg_custom;
  write_to_register = write_to_sg_register;
#endif

  //digital_IO_control[INSTANCE_NUMBER];

  int i ;
  for(i = 0; i < INSTANCE_NUMBER; i++) {

  digital_IO_control[i].reg_object.object_id = 3299,
  digital_IO_control[i].reg_object.instance_id = i,
  //digital_IO_control[i].set_value = set_value,
  digital_IO_control[i].set_custom = set_custom,
  //digital_IO_control[i].read_custom = read_custom,

  ipso_gen_control_add(&digital_IO_control[i]);
  } //end for

// = {
//  .reg_object.object_id = 3201,
//  .reg_object.instance_id = 0,
//  .set_value = set_value,
//  .write_custom = write_custom,
//  .read_custom = read_custom,
//};
//digital_IO_control[INSTANCE_NUMBER];

// = {
//  .reg_object.object_id = 3201,
//  .reg_object.instance_id = 0,
//  .set_value = set_value,
//  .write_custom = write_custom,
//  .read_custom = read_custom,
//};
  //ipso_control_add(&digital_IO_control);
  //ipso_control_add(&digital_IO_control1);
  //ipso_control_add(&digital_IO_control2);
  //ipso_control_add(&digital_IO_control);
  PRINTF("--> IPSO digital output initialized\n");
}
/*---------------------------------------------------------------------------*/

/*
 *  From NIBE manual:
 *  http://www.nibe.co.uk/nibedocuments/16486/231468-3.pdf
 *
Closed or open switch means one of the following:
■ Blocking (A: Closed, B: Open)
"SG Ready" is active. The compressor in the heat
pump and additional heat is blocked like the day's
tariff blocking.
■ Normal mode (A: Open, B: Open)
"SG Ready" is not active. No effect on the system.
■ Low price mode (A: Open, B: Closed)
"SG Ready" is active. The system focuses on costs
savings and can for example exploit a low tariff from
the electricity supplier or over-capacity from any own
power source (effect on the system can be adjusted
in the menu 4.1.5).
■ Overcapacity mode (A: Closed, B: Closed)

*/

#define BLOCKING 0
#define NORMAL 1
#define LOW_PRICE 2
#define OVERCAPACITY 3

int write_to_sg_register(int instance, int value) {
  switch(value) {
  case BLOCKING:
    gpio_relay_set(0, 1);
    gpio_relay_set(1, 0);
    PRINTF("BLOCKING\n");
    break;
  case NORMAL:
    gpio_relay_set(0, 0);
    gpio_relay_set(1, 0);
    PRINTF("NORMAL\n");
    break;
  case LOW_PRICE:
    gpio_relay_set(0, 0);
    gpio_relay_set(1, 1);
    PRINTF("LOW_PRICE\n");
    break;
  case OVERCAPACITY:
    gpio_relay_set(0, 1);
    gpio_relay_set(1, 1);
    PRINTF("OVERCAPACITY\n");
    break;
  default:
    return LWM2M_STATUS_ERROR;
  }
  return LWM2M_STATUS_OK;
}

#if MODBUS
int write_to_modbus_register(int instance, int value)
{
  PRINTF("Write %u to %d\n", value, instance);
    int byteCount;

    if (value) {
      switch (instance) {
      case 0:
        byteCount = modbus_write_function(0x0010, 0x05, 0xff00);
        break;
      case 1:
        byteCount = modbus_write_function(0x0011, 0x05, 0xff00);
        break;
      case 2:
        byteCount = modbus_write_function(0x0012, 0x05, 0xff00);
        break;
      case 3:
        byteCount = modbus_write_function(0x0013, 0x05, 0xff00);
        break;
      case 4:
        byteCount = modbus_write_function(0x0014, 0x05, 0xff00);
        break;
      case 5:
        byteCount = modbus_write_function(0x0015, 0x05, 0xff00);
        break;
      case 6:
        byteCount = modbus_write_function(0x0016, 0x05, 0xff00);
        break;
      case 7:
        byteCount = modbus_write_function(0x0017, 0x05, 0xff00);
        break;
      default:
        return 0;
      }
    } else {
      switch (instance) {
      case 0:
        byteCount = modbus_write_function(0x0010, 0x05, 0x0000);
        break;
      case 1:
        byteCount = modbus_write_function(0x0011, 0x05, 0x0000);
        break;
      case 2:
        byteCount = modbus_write_function(0x0012, 0x05, 0x0000);
        break;
      case 3:
        byteCount = modbus_write_function(0x0013, 0x05, 0x0000);
        break;
      case 4:
        byteCount = modbus_write_function(0x0014, 0x05, 0x0000);
        break;
      case 5:
        byteCount = modbus_write_function(0x0015, 0x05, 0x0000);
        break;
      case 6:
        byteCount = modbus_write_function(0x0016, 0x05, 0x0000);
        break;
      case 7:
        byteCount = modbus_write_function(0x0017, 0x05, 0x0000);
        break;
      default:
        return 0;
      }
    }
    return byteCount;
}


int read_from_modbus_register(int instance)
{
  PRINTF("Read from %d\n", instance);
    int byteCount;

  switch (instance) {
  case 0:
    byteCount = modbus_read_function(0x0010, 0x01, 0x0001);
    break;
  case 1:
    byteCount = modbus_read_function(0x0011, 0x01, 0x0001);
    break;
  case 2:
    byteCount = modbus_read_function(0x0012, 0x01, 0x0001);
    break;
  case 3:
    byteCount = modbus_read_function(0x0013, 0x01, 0x0001);
    break;
  case 4:
    byteCount = modbus_read_function(0x0014, 0x01, 0x0001);
    break;
  case 5:
    byteCount = modbus_read_function(0x0015, 0x01, 0x0001);
    break;
  case 6:
    byteCount = modbus_read_function(0x0016, 0x01, 0x0001);
    break;
  case 7:
    byteCount = modbus_read_function(0x0017, 0x01, 0x0001);
    break;
  default:
    return LWM2M_STATUS_ERROR;
  }
  return byteCount;
}
#endif

int read_application_type()
{

  int byteCount;
  PRINTF("--> REQ: application type read\n");

#if MODBUS
  byteCount = modbus_read_function(0x00d2, 0x03, 0x0001);
  char str[(byteCount * 2) + 1];
  if (byteCount > 0) {
    char str[(byteCount * 2) + 1];
    modbus_get_data(str);
  } else {
    PRINTF("--> READ app type failed\n");
    return -1;
  }
#else
  char str[14] = "SG-ready ctrl";
#endif

  PRINTF("--> READ app type returned with %s\n", str);
  return 0;
}

/*
 * Below is old stuff
 * */

/*---------------------------------------------------------------------------*/
