# Makefile for the Linux charge logos

obj-$(CONFIG_CHARGE_LOGO) += charge.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per0_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per10_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per20_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per30_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per40_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per50_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per60_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per70_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per80_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per90_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_per100_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_full_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_battery_fail_clut224.o
obj-$(CONFIG_CHARGE_LOGO) += logo_low_battery_clut224.o

# How to generate logo's

# Use logo-cfiles to retrieve list of .c files to be built
logo-cfiles = $(notdir $(patsubst %.$(2), %.c, \
              $(wildcard $(srctree)/$(src)/*$(1).$(2))))

# 224 Logos
extra-y += $(call logo-cfiles,_clut224,ppm)

pnmtologo := scripts/pnmtologo

# Create commands like "pnmtologo -t mono -n logo_mac_mono -o ..."
quiet_cmd_logo = LOGO    $@
	cmd_logo = $(pnmtologo) \
			-t $(patsubst $*_%,%,$(notdir $(basename $<))) \
			-n $(notdir $(basename $<)) -o $@ $<

$(obj)/%_clut224.c: $(src)/%_clut224.ppm $(pnmtologo) FORCE
	$(call if_changed,logo)

# Files generated that shall be removed upon make clean
clean-files := *.o *_clut224.c
