// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    RC2014 Clock Module

**********************************************************************/

#ifndef MAME_BUS_RC2014_CLOCK_H
#define MAME_BUS_RC2014_CLOCK_H

#pragma once

#include "bus/rc2014/rc2014.h"

DECLARE_DEVICE_TYPE(RC2014_SINGLE_CLOCK, device_rc2014_card_interface)
DECLARE_DEVICE_TYPE(RC2014_DUAL_CLOCK, device_rc2014_ext_card_interface)
DECLARE_DEVICE_TYPE(RC2014_DUAL_CLOCK_40P, device_rc2014_card_interface)

#endif // MAME_BUS_RC2014_CLOCK_H
