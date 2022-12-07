# Copyright (C) 2022 Analog Devices, Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#     - Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     - Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     - Neither the name of Analog Devices, Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#     - The use of this software may or may not infringe the patent rights
#       of one or more patent holders.  This license does not release you
#       from the requirement that you obtain separate licenses from these
#       patent holders to use this software.
#     - Use of the software either in source or binary form, must be run
#       on or directly connected to an Analog Devices Inc. component.
#
# THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED.
#
# IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
# RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from eeprom_24xx32a import *
import tkinter as tk
from tkinter import ttk
from tkinter import scrolledtext
from ttkthemes import ThemedTk

class eeprom_config_ui():
    _device = None

    def __init__(self, device):
        self._device = device

        # Create a main/root window
        self._root = ThemedTk()
        self._root.get_themes()
        self._root.set_theme("radiance")
        self._root.title("EEPROM Config")

        # Create a notebook (window tabs)
        self._tabs = ttk.Notebook(self._root)

        # Create eeprom register debug tab
        self._debug_tab = ttk.Frame(self._tabs, width=530, height=410)
        self._tabs.add(self._debug_tab, text='EEPROM Debug  ')
        self._tabs.pack()
        self._tabs.propagate(False)

        # eeprom registers label frame and widgets (Read/Write of eeprom registers)

        # **** eeprom registers label frame ****
        eeprom_regs_frame = ttk.LabelFrame(self._debug_tab, text="EEPROM Registers", width=530, height=120)
        eeprom_regs_frame.pack(pady=8, padx=5)
        eeprom_regs_frame.grid_propagate(False)

        # **** Read Button ****
        eeprom_reg_rd_btn = ttk.Button(eeprom_regs_frame, text="Read", width=10, command=self.eeprom_reg_rd_btn_event)
        eeprom_reg_rd_btn.grid(row=1, column=0, padx=15, pady=5)

        # **** Address label ****
        eeprom_reg_addr_label = ttk.Label(eeprom_regs_frame, text="Address ")
        eeprom_reg_addr_label.grid(row=1, column=1)

        # **** Address entry text ****
        self._eeprom_reg_addr_text = ttk.Entry(eeprom_regs_frame, width=12)
        self._eeprom_reg_addr_text.insert(0,hex(0))
        self._eeprom_reg_addr_text.grid(row=1, column=2, padx=2, pady=5)

        # **** Address increment/decrement buttons ****
        eeprom_reg_addr_dec_btn = ttk.Button(eeprom_regs_frame, text="<", width=1, command=self.eeprom_reg_addr_dec_btn_event)
        eeprom_reg_addr_dec_btn.grid(row=1, column=3, padx=2, pady=2)
        eeprom_reg_addr_inc_btn = ttk.Button(eeprom_regs_frame, text=">", width=1, command=self.eeprom_reg_addr_inc_btn_event)
        eeprom_reg_addr_inc_btn.grid(row=1, column=4, padx=2, pady=2)

        # **** Write Button ****
        eeprom_reg_wr_btn = ttk.Button(eeprom_regs_frame, text="Write", width=10, command=self.eeprom_reg_wr_btn_event)
        eeprom_reg_wr_btn.grid(row=2, column=0, padx=15, pady=5)

        # **** Data label ****
        eeprom_reg_data_label = ttk.Label(eeprom_regs_frame, text="Data ")
        eeprom_reg_data_label.grid(row=2, column=1)

        # **** Data entry text ****
        self._eeprom_reg_data_text = ttk.Entry(eeprom_regs_frame, width=12)
        self._eeprom_reg_data_text.insert(0,hex(0))
        self._eeprom_reg_data_text.grid(row=2, column=2, padx=2, pady=5)


        # eeprom data read label frame and widgets

        # **** Read EEPROM data label frame ****
        data_frame = ttk.LabelFrame(self._debug_tab, text="EEPROM data ASCII (first 256 char)", width=530, height=60)
        data_frame.pack(pady=8, padx=5)
        data_frame.grid_propagate(False)

        # **** Read Button ****
        eeprom_reg_data_all_btn = ttk.Button(data_frame, text="Read", width=10, command=self.dev_reg_data_all_btn_event)
        eeprom_reg_data_all_btn.grid(row=1, column=0, padx=15, pady=5)

        # # **** EEPROM data Text box label frame (needed for text widget) ****
        eeprom_data_text_frame = ttk.LabelFrame(self._debug_tab, width=530, height=200)
        eeprom_data_text_frame.pack(pady=8, padx=5)
        eeprom_data_text_frame.grid_propagate(False)

        # **** Scrolling text ****
        self._eeprom_reg_data_all_text = scrolledtext.Text(eeprom_data_text_frame, width=300, height=200)
        self._eeprom_reg_data_all_text.grid(row=1, column=0, padx=2, pady=5)

        # Run the tkinter UI mainloop
        self._root.mainloop()

    def get_eeprom_cfg_ui_instance(self):
        """ Method to get the instance of root/main window """
        return self._root
        
    def eeprom_reg_rd_btn_event(self):
        """ Handle the eeprom register read button select event """
        address = int(self._eeprom_reg_addr_text.get(), 16)
        data = hex(self._device._ctrl.reg_read(address))
        self._eeprom_reg_data_text.delete(0,"end")
        self._eeprom_reg_data_text.insert(0,data)

    def eeprom_reg_wr_btn_event(self):
        """ Handle the eeprom register write button select event """
        address = int(self._eeprom_reg_addr_text.get(), 16)
        data = int(self._eeprom_reg_data_text.get(), 16)
        ret = self._device._ctrl.reg_write(address, data)
        if (ret):
            print("Register Write Failed \r\n")

    def eeprom_reg_addr_inc_btn_event(self):
        """ Handle the eeprom register address increment button select event """
        addr_hex = int(self._eeprom_reg_addr_text.get(), 16) + 1
        addr_int = hex(addr_hex)
        self._eeprom_reg_addr_text.delete(0,"end")
        self._eeprom_reg_addr_text.insert(0, str(addr_int))
        self.eeprom_reg_rd_btn_event()

    def eeprom_reg_addr_dec_btn_event(self):
        """ Handle the eeprom register address decrement button select event """
        addr_hex = int(self._eeprom_reg_addr_text.get(), 16)
        if (addr_hex > 0x0):
            addr_hex = addr_hex - 1
        addr_int = hex(addr_hex)
        self._eeprom_reg_addr_text.delete(0,"end")
        self._eeprom_reg_addr_text.insert(0, str(addr_int))
        self.eeprom_reg_rd_btn_event()

    def dev_reg_data_all_btn_event(self):
        data = ""
        """ Handle the all eeprom data read button select event """
        for address in range(0,256):
            recv = self._device._ctrl.reg_read(address)
            data = data + (chr(int(recv)))
            if ((address != 0) and (address % 50) == 0):
                data = data + '\n'
        
        self._eeprom_reg_data_all_text.delete("1.0","end")
        self._eeprom_reg_data_all_text.insert(tk.INSERT,data)
