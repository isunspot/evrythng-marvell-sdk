#
# Copyright (C) 2008-2015, Marvell International Ltd.
# All Rights Reserved.

exec-y += rheem_demo

rheem_demo-objs-y := src/main.c src/websocket.c

rheem_demo-cflags-y := -DAPPCONFIG_DEBUG_ENABLE=1 

