PROJECT = app_mt

MAJOR_VERSION = 1
MINOR_VERSION = 0
PATCH_VERSION = 0

BOARD = II-MT-CONTROLLER

PROJECT_INCDIR = util wifi

PROJECT_AUTOGEN_CSRC = \
       image_resources.c \
       font_resources.c

PROJECT_CSRC = \
       app_cfg.c \
       app_hdr.c \
       datastream.c \
       debug_client.c \
       fault.c \
       font.c \
       gfx.c \
       gui.c \
       gui_calib.c \
       gui_history.c \
       gui_home.c \
       gui_output.c \
       gui_output_function.c \
       gui_output_trigger.c \
       gui_quantity_select.c \
       gui_settings.c \
       gui_update.c \
       gui_wifi.c \
       image.c \
       lcd.c \
       main.c \
       message.c \
       net.c \
       onewire.c \
       pid.c \
       quantity_widget.c \
       sensor.c \
       temp_control.c \
       touch.c \
       touch_calib.c \
       txn.c \
       web_api.c \
       web_api_parser.c \
       wifi/core/cc3000_common.c \
       wifi/core/cc3000_spi.c \
       wifi/core/evnt_handler.c \
       wifi/core/hci.c \
       wifi/core/c_netapp.c \
       wifi/core/c_nvmem.c \
       wifi/core/c_security.c \
       wifi/core/c_socket.c \
       wifi/core/c_wlan.c \
       wifi/netapp.c \
       wifi/nvmem.c \
       wifi/patch.c \
       wifi/security.c \
       wifi/socket.c \
       wifi/wlan.c \
       gui/button.c \
       gui/icon.c \
       gui/label.c \
       gui/progressbar.c \
       gui/scatter_plot.c \
       gui/widget.c \
       util/linked_list.c \
       ../common/crc/crc8.c \
       ../common/crc/crc16.c \
       ../common/crc/crc32.c \
       ../common/iflash.c \
       ../common/xflash.c

include make-bin.mk