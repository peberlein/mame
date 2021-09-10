// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MMC3_CLONES_H
#define MAME_BUS_NES_MMC3_CLONES_H

#pragma once

#include "mmc3.h"


// ======================> nes_nitra_device

class nes_nitra_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_nitra_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_fs6_device

class nes_fs6_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_fs6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_sbros11_device

class nes_sbros11_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sbros11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_malisb_device

class nes_malisb_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_malisb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_family4646_device

class nes_family4646_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_family4646_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_pikay2k_device

class nes_pikay2k_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_pikay2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[2];
};


// ======================> nes_8237_device

class nes_8237_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_8237_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_8237_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();
	u8 m_reg[3];
	int m_board;
};


// ======================> nes_8237a_device

class nes_8237a_device : public nes_8237_device
{
public:
	// construction/destruction
	nes_8237a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_kasing_device

class nes_kasing_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kasing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_kasing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

private:
	bool m_mmc3_mode;
};


// ======================> nes_sglionk_device

class nes_sglionk_device : public nes_kasing_device
{
public:
	// construction/destruction
	nes_sglionk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// construction/destruction
	nes_sglionk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	int m_board;
};


// ======================> nes_sgboog_device

class nes_sgboog_device : public nes_sglionk_device
{
public:
	// construction/destruction
	nes_sgboog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_kay_device

class nes_kay_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_regs();
	uint8_t m_reg[8];
	uint8_t m_low_reg;
};


// ======================> nes_h2288_device

class nes_h2288_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_h2288_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[2]; // reg 1 is unused?
};


// ======================> nes_6035052_device

class nes_6035052_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_6035052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_ex(offs_t offset) override;
	virtual void write_ex(offs_t offset, uint8_t data) override;
	virtual uint8_t read_l(offs_t offset) override { return read_ex(offset); }
	virtual uint8_t read_m(offs_t offset) override { return read_ex(offset); }
	virtual void write_l(offs_t offset, uint8_t data) override { write_ex(offset, data); }
	virtual void write_m(offs_t offset, uint8_t data) override { write_ex(offset, data); }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_prot;
};


// ======================> nes_txc_tw_device

class nes_txc_tw_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_txc_tw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override { write_l(offset & 0xff, data); }   // offset does not really count for this mapper }
	virtual void prg_cb(int start, int bank) override;
};


// ======================> nes_kof97_device

class nes_kof97_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kof97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_kof96_device

class nes_kof96_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kof96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;   // offset does not really count for this mapper
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[4];
};


// ======================> nes_sf3_device

class nes_sf3_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;

protected:
	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask) override;
};


// ======================> nes_gouder_device

class nes_gouder_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_gouder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[5];
};


// ======================> nes_sa9602b_device

class nes_sa9602b_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sa9602b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg;
	int m_prg_chip;
};


// ======================> nes_sachen_shero_device

class nes_sachen_shero_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sachen_shero_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg;
};

#ifdef UNUSED_FUNCTION
// ======================> nes_a9746_device

class nes_a9746_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_a9746_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset();

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks(uint8_t value);
	uint8_t m_reg[3];
};
#endif


// ======================> nes_a88s1_device

class nes_a88s1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_a88s1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();
	u8 m_reg[2];
};


// ======================> nes_fcgj8in1_device

class nes_fcgj8in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_fcgj8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_fk23c_device

class nes_fk23c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_fk23c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	nes_fk23c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	void fk23c_set_prg();
	void fk23c_set_chr();
	uint8_t m_reg[8];
	uint8_t m_mmc_cmd1;
};


// ======================> nes_fk23ca_device

class nes_fk23ca_device : public nes_fk23c_device
{
public:
	// construction/destruction
	nes_fk23ca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pcb_reset() override;
};


// ======================> nes_nt639_device

class nes_nt639_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_nt639_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_resettxrom0_device

class nes_resettxrom0_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_resettxrom0_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_resettxrom0_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prg_shift, int chr_shift);

	// device-level overrides
	virtual void device_start() override;

private:
	int m_count;
	int m_prg_shift, m_chr_shift;
};


// ======================> nes_resettxrom1_device

class nes_resettxrom1_device : public nes_resettxrom0_device
{
public:
	// construction/destruction
	nes_resettxrom1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_resettxrom2_device

class nes_resettxrom2_device : public nes_resettxrom0_device
{
public:
	// construction/destruction
	nes_resettxrom2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_s24in1sc03_device

class nes_s24in1sc03_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_s24in1sc03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[3];
};


// ======================> nes_tech9in1_device

class nes_tech9in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_tech9in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();
	u8 m_reg[3];
};


// ======================> nes_bmc_8in1_device

class nes_bmc_8in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_15in1_device

class nes_bmc_15in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_15in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_sbig7_device

class nes_bmc_sbig7_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_sbig7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_hik8_device

class nes_bmc_hik8_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_hik8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[4];
	uint8_t m_count;
};


// ======================> nes_bmc_hik4_device

class nes_bmc_hik4_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_hik4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_mario7in1_device

class nes_bmc_mario7in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_mario7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg_written;
};


// ======================> nes_bmc_f15_device

class nes_bmc_f15_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_f15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_gn45_device

class nes_bmc_gn45_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_gn45_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	bool m_lock;
};


// ======================> nes_bmc_gold7in1_device

class nes_bmc_gold7in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_gold7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg_written;
};


// ======================> nes_bmc_k3006_device

class nes_bmc_k3006_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_k3006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_411120c_device

class nes_bmc_411120c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_411120c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	u8 m_reg;
};


// ======================> nes_bmc_830118c_device

class nes_bmc_830118c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_830118c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg;
};


// ======================> nes_bmc_830832c_device

class nes_bmc_830832c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_830832c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_841101c_device

class nes_bmc_841101c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_841101c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_pjoy84_device

class nes_pjoy84_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_pjoy84_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	inline void set_base_mask();
	uint8_t m_reg[4];
};

// ======================> nes_coolboy_device

class nes_coolboy_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_coolboy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

private:
	uint8_t m_reg[4];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_NITRA,         nes_nitra_device)
DECLARE_DEVICE_TYPE(NES_FS6,           nes_fs6_device)
DECLARE_DEVICE_TYPE(NES_SBROS11,       nes_sbros11_device)
DECLARE_DEVICE_TYPE(NES_MALISB,        nes_malisb_device)
DECLARE_DEVICE_TYPE(NES_FAMILY4646,    nes_family4646_device)
DECLARE_DEVICE_TYPE(NES_PIKAY2K,       nes_pikay2k_device)
DECLARE_DEVICE_TYPE(NES_8237,          nes_8237_device)
DECLARE_DEVICE_TYPE(NES_8237A,         nes_8237a_device)
DECLARE_DEVICE_TYPE(NES_SG_LIONK,      nes_sglionk_device)
DECLARE_DEVICE_TYPE(NES_SG_BOOG,       nes_sgboog_device)
DECLARE_DEVICE_TYPE(NES_KASING,        nes_kasing_device)
DECLARE_DEVICE_TYPE(NES_KAY,           nes_kay_device)
DECLARE_DEVICE_TYPE(NES_H2288,         nes_h2288_device)
DECLARE_DEVICE_TYPE(NES_6035052,       nes_6035052_device)
DECLARE_DEVICE_TYPE(NES_TXC_TW,        nes_txc_tw_device)
DECLARE_DEVICE_TYPE(NES_KOF97,         nes_kof97_device)
DECLARE_DEVICE_TYPE(NES_KOF96,         nes_kof96_device)
DECLARE_DEVICE_TYPE(NES_SF3,           nes_sf3_device)
DECLARE_DEVICE_TYPE(NES_GOUDER,        nes_gouder_device)
DECLARE_DEVICE_TYPE(NES_SA9602B,       nes_sa9602b_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_SHERO,  nes_sachen_shero_device)
#ifdef UNUSED_FUNCTION
DECLARE_DEVICE_TYPE(NES_A9746,         nes_a9746_device)
#endif

DECLARE_DEVICE_TYPE(NES_A88S1,         nes_a88s1_device)
DECLARE_DEVICE_TYPE(NES_FCGJ8IN1,      nes_fcgj8in1_device)
DECLARE_DEVICE_TYPE(NES_FK23C,         nes_fk23c_device)
DECLARE_DEVICE_TYPE(NES_FK23CA,        nes_fk23ca_device)
DECLARE_DEVICE_TYPE(NES_NT639,         nes_nt639_device)
DECLARE_DEVICE_TYPE(NES_RESETTXROM0,   nes_resettxrom0_device)
DECLARE_DEVICE_TYPE(NES_RESETTXROM1,   nes_resettxrom1_device)
DECLARE_DEVICE_TYPE(NES_RESETTXROM2,   nes_resettxrom2_device)
DECLARE_DEVICE_TYPE(NES_S24IN1SC03,    nes_s24in1sc03_device)
DECLARE_DEVICE_TYPE(NES_TECHLINE9IN1,  nes_tech9in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_8IN1,      nes_bmc_8in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_15IN1,     nes_bmc_15in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_SBIG7,     nes_bmc_sbig7_device)
DECLARE_DEVICE_TYPE(NES_BMC_HIK8,      nes_bmc_hik8_device)
DECLARE_DEVICE_TYPE(NES_BMC_HIK4,      nes_bmc_hik4_device)
DECLARE_DEVICE_TYPE(NES_BMC_MARIO7IN1, nes_bmc_mario7in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_F15,       nes_bmc_f15_device)
DECLARE_DEVICE_TYPE(NES_BMC_GN45,      nes_bmc_gn45_device)
DECLARE_DEVICE_TYPE(NES_BMC_GOLD7IN1,  nes_bmc_gold7in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_K3006,     nes_bmc_k3006_device)
DECLARE_DEVICE_TYPE(NES_BMC_411120C,   nes_bmc_411120c_device)
DECLARE_DEVICE_TYPE(NES_BMC_830118C,   nes_bmc_830118c_device)
DECLARE_DEVICE_TYPE(NES_BMC_830832C,   nes_bmc_830832c_device)
DECLARE_DEVICE_TYPE(NES_BMC_841101C,   nes_bmc_841101c_device)
DECLARE_DEVICE_TYPE(NES_PJOY84,        nes_pjoy84_device)
DECLARE_DEVICE_TYPE(NES_COOLBOY,       nes_coolboy_device)

#endif // MAME_BUS_NES_MMC3_CLONES_H
