/**
 * \file   
 * \author max <mpano91@gmail.com>
 */

#include "teocli.hpp"

using namespace teo;

Teocli::UniquePtr Teocli::create_connection(EventsCb event_cb, struct app_parameters* ap)
{
  return UniquePtr(new Teocli(event_cb, ap));
}

teo::Teocli::Teocli(EventsCb event_cb, struct app_parameters* ap)
{
  teoLNullInit();
//  m_connection = (connection_interface_t *)malloc(sizeof(connection_interface_t));
  ci_init(&m_connection, event_cb, ap);
}

