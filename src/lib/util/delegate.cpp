// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Vas Crabb
/***************************************************************************

    delegate.cpp

    Templates and classes to enable delegates for callbacks.

***************************************************************************/

#include "delegate.h"

#include <cstdio>
#include <sstream>


//**************************************************************************
//  MACROS
//**************************************************************************

#if defined(MAME_DELEGATE_LOG_ADJ)
	#define LOG(...) printf(__VA_ARGS__)
#else
	#define LOG(...) do { if (false) printf(__VA_ARGS__); } while (false)
#endif

// on some architectures, function pointers point to descriptors
// usually this is a global pointer value along with the branch target
// other platforms using this convention include:
// * AIX, Classic MacOS and WinNT on 32-bit POWER/PowerPC
// * pretty much anything on Itanium
#if (defined(__ppc64__) || (defined(__PPC64__))) && !defined(__APPLE__)
	#define MAME_DELEGATE_VT_DESCRIPTOR 1
#endif
#ifndef MAME_DELEGATE_VT_DESCRIPTOR
	#define MAME_DELEGATE_VT_DESCRIPTOR 0
#endif



//**************************************************************************
//  LATE BINDING EXCEPTION
//**************************************************************************

binding_type_exception::binding_type_exception(std::type_info const &target_type, std::type_info const &actual_type)
	: m_target_type(&target_type)
	, m_actual_type(&actual_type)
{
	std::ostringstream os;
	os << "Error performing late bind of function expecting type " << target_type.name() << " to instance of type " << actual_type.name();
	m_what = os.str();
}


char const *binding_type_exception::what() const noexcept
{
	return m_what.c_str();
}



namespace util::detail {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const delegate_mfp_compatible::raw_mfp_data delegate_mfp_compatible::s_null_mfp = { { 0 } };



//**************************************************************************
//  INTERNAL DELEGATE HELPERS
//**************************************************************************

//-------------------------------------------------
//  delegate_mfp_itanium::convert_to_generic -
//  given an object pointer and member function
//  pointer, apply the displacement and get the
//  actual function pointer
//-------------------------------------------------

delegate_generic_function delegate_mfp_itanium::convert_to_generic(delegate_generic_class *&object) const
{
	// apply the "this" delta to the object first - the value is shifted to the left one bit position for the ARM-like variant
	LOG("Input this=%p ptr=%p adj=%ld ", reinterpret_cast<void const *>(object), reinterpret_cast<void const *>(m_function), long(m_this_delta));
	object = reinterpret_cast<delegate_generic_class *>(
			reinterpret_cast<std::uint8_t *>(object) + (m_this_delta >> (MAME_DELEGATE_ITANIUM_ARM ? 1 : 0)));
	LOG("Calculated this=%p ", reinterpret_cast<void const *>(object));

	// test the virtual member function flag - it's the low bit of either the ptr or adj field, depending on the variant
	if (MAME_DELEGATE_ITANIUM_ARM ? !(m_this_delta & 1) : !(m_function & 1))
	{
		// conventional function pointer
		LOG("ptr=%p\n", reinterpret_cast<void const *>(m_function));
		return reinterpret_cast<delegate_generic_function>(m_function);
	}
	else
	{
		// byte index into the vtable to the function
		std::uint8_t const *const vtable_ptr = *reinterpret_cast<std::uint8_t const *const *>(object) + m_function - (MAME_DELEGATE_ITANIUM_ARM ? 0 : 1);
		delegate_generic_function result;
		if (MAME_DELEGATE_VT_DESCRIPTOR)
			result = reinterpret_cast<delegate_generic_function>(uintptr_t(vtable_ptr));
		else
			result = *reinterpret_cast<delegate_generic_function const *>(vtable_ptr);
		LOG("ptr=%p (vtable)\n", reinterpret_cast<void const *>(result));
		return result;
	}
}


//-------------------------------------------------
//  delegate_mfp_msvc::adjust_this_pointer - given
//  an object pointer and member function pointer,
//  apply the displacement
//-------------------------------------------------

void delegate_mfp_msvc::adjust_this_pointer(delegate_generic_class *&object) const
{
	LOG("Input this=%p ", reinterpret_cast<void const *>(object));
	if (sizeof(single_base_equiv) < m_size)
		LOG("thisdelta=%d ", m_this_delta);
	if (sizeof(unknown_base_equiv) == m_size)
		LOG("vptrdelta=%d vindex=%d ", m_vptr_offs, m_vt_index);
	std::uint8_t *byteptr = reinterpret_cast<std::uint8_t *>(object);
	if ((sizeof(unknown_base_equiv) == m_size) && m_vt_index)
	{
		std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(byteptr + m_vptr_offs);
		byteptr += *reinterpret_cast<int const *>(vptr + m_vt_index);
	}
	if (sizeof(single_base_equiv) < m_size)
		byteptr += m_this_delta;
	LOG("Calculated this=%p\n", reinterpret_cast<void const *>(byteptr));
	object = reinterpret_cast<delegate_generic_class *>(byteptr);
}

} // namespace util::detail
