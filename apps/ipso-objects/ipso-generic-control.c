/*
 * Copyright (c) 2016, SICS Swedish ICT AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup ipso-objects
 * @{
 *
 */

/**
 * \file
 *         Implementation of OMA LWM2M / IPSO control template.
 *         Useful for implementing _generic_ controllable objects
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joel Hoglund <joel@sics.se>
 */
#include "ipso-generic-control.h"
#include "lwm2m-engine.h"
#include "sys/ntimer.h"
#include <string.h>
#include <stdio.h>

#define IPSO_DIGITAL_IO   5550

static const lwm2m_resource_id_t resources[] =
{
    RW(IPSO_DIGITAL_IO)
};

/*---------------------------------------------------------------------------*/
static lwm2m_status_t
lwm2m_callback(lwm2m_object_instance_t *object,
    lwm2m_context_t *ctx)
{
  /* Here we cast to our sensor-template struct */
  ipso_gen_control_t *control;
  size_t len;
  int32_t v;

  control = (ipso_gen_control_t *) object;

  /* Do the stuff */
  if(ctx->level < 3) {
    return LWM2M_STATUS_ERROR;
  }
  if(ctx->level == 3) {
    /* This is a get request on 3303/0/3700 */
    /* NOW we assume a get.... which might be wrong... */
    if(ctx->operation == LWM2M_OP_READ) {
      switch(ctx->resource_id) {
      default:
        return LWM2M_STATUS_ERROR;
      }
      lwm2m_object_write_int(ctx, v);
    } else if(ctx->operation == LWM2M_OP_WRITE) {
      switch(ctx->resource_id) {

      default:
        printf("No default handler - ");
          len = lwm2m_object_read_int(ctx, ctx->inbuf->buffer, ctx->inbuf->size, &v);
          int32_t new_state = v;
        /* Call the set custom callback  */
          //len = lwm2m_object_read_int(ctx, ctx->inbuf->buffer, ctx->inbuf->size, &v);
        if(control->set_custom) {
          if(control->set_custom(v, ctx->resource_id, ctx->resource_instance_id)) {
          return LWM2M_STATUS_ERROR;
        }
        return LWM2M_STATUS_OK;
        }

        return LWM2M_STATUS_ERROR;

      }//end switch
  }//end else if write
}//end if(ctx->level == 3)
return LWM2M_STATUS_OK;
}//end of function
/*---------------------------------------------------------------------------*/
int
ipso_gen_control_add(ipso_gen_control_t *control)
{
  if(control->reg_object.instance_id == 0) {
    control->reg_object.instance_id =
        lwm2m_engine_recommend_instance_id(control->reg_object.object_id);
  }
  control->reg_object.resource_ids = resources;
  control->reg_object.resource_count =
      sizeof(resources) / sizeof(lwm2m_resource_id_t);

  control->reg_object.callback = lwm2m_callback;
  lwm2m_engine_add_object(&control->reg_object);
  return 1;
}
/*---------------------------------------------------------------------------*/
int
ipso_gen_control_remove(ipso_gen_control_t *control)
{
  lwm2m_engine_remove_object(&control->reg_object);
  return 1;
}
/*---------------------------------------------------------------------------*/
