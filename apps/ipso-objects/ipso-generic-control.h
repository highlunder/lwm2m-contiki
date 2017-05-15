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
 *         Header file of OMA LWM2M / IPSO _generic_ control template.
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joel Hoglund <joel@sics.se>
 */

#ifndef IPSO_GEN_CONTROL_TEMPLATE_H_
#define IPSO_GEN_CONTROL_TEMPLATE_H_

#include "lwm2m-engine.h"

typedef lwm2m_status_t (*ipso_control_set_custom_t)(uint32_t v, uint16_t resource, uint8_t instance);
typedef lwm2m_status_t (*ipso_control_write_t)(uint16_t resource, uint8_t instance, lwm2m_context_t *ctx, const uint8_t *inbuf, size_t insize);

typedef struct ipso_gen_control ipso_gen_control_t;

/* Values of the IPSO control object */
struct ipso_gen_control {
  lwm2m_object_instance_t reg_object;
  uint8_t flags;
  uint8_t value;  /* used to emulate on/off and dim-value */
  uint32_t on_time; /* on-time in millis - value > 0 is counted */
  uint64_t last_on_time;
  ipso_control_set_custom_t set_custom;
  ipso_control_write_t write_custom;

};



int ipso_gen_control_add(ipso_gen_control_t *control);
int ipso_gen_control_remove(ipso_gen_control_t *control);

#endif /* IPSO_CONTROL_TEMPLATE_H_ */
